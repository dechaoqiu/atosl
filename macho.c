/*
 * =====================================================================================
 *
 *       Filename:  macho.c
 *
 *    Description:  Mach-O Reader
 *
 *        Version:  1.0
 *        Created:  02/17/2013 22:51:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reno Qiu
 *   Organization:
 *
 * =====================================================================================
 */

#include "macho.h"

char *project_name;

static struct die_info * read_die_and_children (char *info_ptr, struct dwarf2_cu *cu, char **new_info_ptr, struct die_info *parent);
static struct die_info * read_die_and_siblings (char *info_ptr, struct dwarf2_cu *cu, char **new_info_ptr, struct die_info *parent);

/* Free the line_header structure *LH, and any arrays and strings it
   refers to.  */
static void free_line_header (struct line_header *lh)
{
    if (lh->standard_opcode_lengths)
        free (lh->standard_opcode_lengths);

    /* Remember that all the lh->file_names[i].name pointers are
       pointers into debug_line_buffer, and don't need to be freed.  */
    if (lh->file_names)
        free (lh->file_names);

    /* Similarly for the include directory names.  */
    if (lh->include_dirs)
        free (lh->include_dirs);

    free (lh);
}


static CORE_ADDR read_signed_16(char *info_ptr){
    signed int ret = 0;
    ret = info_ptr[1];
    ret = (ret << 8) + info_ptr[0];
    return ret;
}

static signed int read_signed_32(char *info_ptr){
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    signed int ret = 0;
    ret = temp_ptr[3];
    ret = (ret << 8) + temp_ptr[2];
    ret = (ret << 8) + temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static int64_t read_signed_64(char *info_ptr){
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    int64_t ret = 0;
    ret = temp_ptr[7];
    ret = (ret << 8) + temp_ptr[6];
    ret = (ret << 8) + temp_ptr[5];
    ret = (ret << 8) + temp_ptr[4];
    ret = (ret << 8) + temp_ptr[3];
    ret = (ret << 8) + temp_ptr[2];
    ret = (ret << 8) + temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static unsigned int read_1_byte (char *info_ptr)
{
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    return *temp_ptr;
}

static int read_1_signed_byte (char *buf)
{
    int ret = 0;
    ret = (int)*buf;
    return ret;
}

static unsigned int read_2_bytes (char *info_ptr)
{
    //read bytes little endian?
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    unsigned short ret = 0;
    ret = temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static unsigned int read_4_bytes(char *info_ptr)
{
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    unsigned int ret = 0;
    ret = temp_ptr[3];
    ret = (ret << 8) + temp_ptr[2];
    ret = (ret << 8) + temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static unsigned long read_8_bytes (char *info_ptr)
{
    //read bytes little endian?
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    unsigned long ret = 0;
    ret = temp_ptr[7];
    ret = (ret << 8) + temp_ptr[6];
    ret = (ret << 8) + temp_ptr[5];
    ret = (ret << 8) + temp_ptr[4];
    ret = (ret << 8) + temp_ptr[3];
    ret = (ret << 8) + temp_ptr[2];
    ret = (ret << 8) + temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static unsigned int read_unsigned_int(char *info_ptr){
    //read bytes little endian?
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    unsigned int ret = 0;
    ret = temp_ptr[3];
    ret = (ret << 8) + temp_ptr[2];
    ret = (ret << 8) + temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static unsigned short read_unsigned_short(char *info_ptr){
    //read bytes little endian?
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    unsigned short ret = 0;
    ret = temp_ptr[1];
    ret = (ret << 8) + temp_ptr[0];
    return ret;
}

static unsigned char read_unsigned_char(char *info_ptr){
    unsigned char * temp_ptr = (unsigned char *)info_ptr;
    unsigned char ret = 0;
    ret = temp_ptr[0];
    return ret;
}

static long long read_signed_leb128(char* leb128_str, unsigned int* leb128_length)
{
    unsigned char * leb128 = (unsigned char * )leb128_str;
    signed long long number = 0;
    int sign = 0;
    signed long shift = 0;
    unsigned char byte = *leb128;
    signed long byte_length = 1;

    /*  byte_length being the number of bytes of data absorbed so far in
     *         turning the leb into a Dwarf_Signed. */

    for (;;) {
        sign = byte & 0x40;
        number |= ((signed long long) ((byte & 0x7f))) << shift;
        shift += 7;

        if ((byte & 0x80) == 0) {
            break;
        }
        ++leb128;
        byte = *leb128;
        byte_length++;
    }

    if ((shift < sizeof(signed long long) * 8) && sign) {
        number |= -((signed long long) 1 << shift);
    }

    if (leb128_length != NULL)
        *leb128_length = byte_length;
    return (number);
}


static unsigned long long read_unsigned_leb128(char* leb128_str, unsigned int* leb128_length)
{
    unsigned char byte;
    unsigned long word_number;
    unsigned long long number;
    signed long shift;
    signed long byte_length;
    unsigned char * leb128 = (unsigned char * )leb128_str;

    /*  The following unrolls-the-loop for the first few bytes and
     *  unpacks into 32 bits to make this as fast as possible.
     *  word_number is assumed big enough that the shift has a defined
     *  result. */
    if ((*leb128 & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 1;
        return (*leb128);
    } else if ((*(leb128 + 1) & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 2;

        word_number = *leb128 & 0x7f;
        word_number |= (*(leb128 + 1) & 0x7f) << 7;
        return (word_number);
    } else if ((*(leb128 + 2) & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 3;

        word_number = *leb128 & 0x7f;
        word_number |= (*(leb128 + 1) & 0x7f) << 7;
        word_number |= (*(leb128 + 2) & 0x7f) << 14;
        return (word_number);
    } else if ((*(leb128 + 3) & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 4;

        word_number = *leb128 & 0x7f;
        word_number |= (*(leb128 + 1) & 0x7f) << 7;
        word_number |= (*(leb128 + 2) & 0x7f) << 14;
        word_number |= (*(leb128 + 3) & 0x7f) << 21;
        return (word_number);
    }

    /*  The rest handles long numbers Because the 'number' may be larger
     *  than the default int/unsigned, we must cast the 'byte' before
     *  the shift for the shift to have a defined result. */
    number = 0;
    shift = 0;
    byte_length = 1;
    byte = *(leb128);
    for (;;) {
        number |= ((unsigned int) (byte & 0x7f)) << shift;

        if ((byte & 0x80) == 0) {
            if (leb128_length != NULL)
                *leb128_length = byte_length;
            return (number);
        }
        shift += 7;

        byte_length++;
        ++leb128;
        byte = *leb128;
    }
}


static char * read_string (char *buf, unsigned int *bytes_read_ptr)
{
    if (*buf == '\0')
    {
        *bytes_read_ptr = 1;
        return NULL;
    }
    *bytes_read_ptr = strlen (buf) + 1;
    return buf;
}

/* Read an offset from the data stream.  The size of the offset is
   given by cu_header->offset_size.  */

static long read_offset (char *buf, const struct comp_unit_head *cu_header, int *bytes_read)
{
    long retval = 0;

    switch (cu_header->offset_size)
    {
        case 4:
            retval = read_4_bytes(buf);
            *bytes_read = 4;
            break;
        case 8:
            retval = read_8_bytes(buf);
            *bytes_read = 8;
            break;
        default:
            printf("read_offset: bad switch\n");
    }

    return retval;
}


static CORE_ADDR read_address_of_cu (char *buf, struct dwarf2_cu *cu, int *bytes_read)
{
    struct comp_unit_head *cu_header = &cu->header;
    CORE_ADDR retval = 0;

    switch (cu_header->addr_size)
    {
        case 2:
            //unsigned
            retval = read_signed_16(buf);
            break;
        case 4:
            retval = read_signed_32(buf);
            break;
        case 8:
            retval = read_signed_64(buf);
            break;
        default:
            printf("read address: bad switch, signed\n");
    }

    *bytes_read = cu_header->addr_size;
    return retval;
}

/* Read the initial length from a section.  The (draft) DWARF 3
   specification allows the initial length to take up either 4 bytes
   or 12 bytes.  If the first 4 bytes are 0xffffffff, then the next 8
   bytes describe the length and all offsets will be 8 bytes in length
   instead of 4.

   An older, non-standard 64-bit format is also handled by this
   function.  The older format in question stores the initial length
   as an 8-byte quantity without an escape value.  Lengths greater
   than 2^32 aren't very common which means that the initial 4 bytes
   is almost always zero.  Since a length value of zero doesn't make
   sense for the 32-bit format, this initial zero can be considered to
   be an escape value which indicates the presence of the older 64-bit
   format.  As written, the code can't detect (old format) lengths
   greater than 4GB.  If it becomes necessary to handle lengths
   somewhat larger than 4GB, we could allow other small values (such
   as the non-sensical values of 1, 2, and 3) to also be used as
   escape values indicating the presence of the old format.

   The value returned via bytes_read should be used to increment the
   relevant pointer after calling read_initial_length_of_comp_unit().

   As a side effect, this function sets the fields initial_length_size
   and offset_size in cu_header to the values appropriate for the
   length field.  (The format of the initial length field determines
   the width of file offsets to be fetched later with read_offset().)

   [ Note:  read_initial_length_of_comp_unit() and read_offset() are based on the
   document entitled "DWARF Debugging Information Format", revision
   3, draft 8, dated November 19, 2001.  This document was obtained
from:

http://reality.sgiweb.org/davea/dwarf3-draft8-011125.pdf

This document is only a draft and is subject to change.  (So beware.)

Details regarding the older, non-standard 64-bit format were
determined empirically by examining 64-bit ELF files produced by
the SGI toolchain on an IRIX 6.5 machine.

- Kevin, July 16, 2002
] */
static long read_initial_length_of_aranges(char *buf, struct aranges_header *aranges_header, int *bytes_read){
    long length = read_4_bytes(buf);
    return length;
}

static long read_initial_length_of_comp_unit (char *buf, struct comp_unit_head *cu_header,
        int *bytes_read)
{
    long length = read_4_bytes(buf);

    if (length == 0xffffffff)
    {
        length = read_8_bytes(buf + 4);
        *bytes_read = 12;
    }
    else if (length == 0)
    {
        /* Handle the (non-standard) 63-bit DWARF2 format used by IRIX.  */
        length = read_8_bytes(buf + 4);
        *bytes_read = 8;
    }
    else
    {
        *bytes_read = 4;
    }

    if (cu_header)
    {
        assert (cu_header->initial_length_size == 0
                || cu_header->initial_length_size == 4
                || cu_header->initial_length_size == 8
                || cu_header->initial_length_size == 12);

        if(cu_header->initial_length_size != 0
                && cu_header->initial_length_size != 4
                && cu_header->initial_length_size != 8
                && cu_header->initial_length_size != 12){
            PyErr_Format(ATOSLError, "cu_header->initial_length_size invalid");
            return -1;
        }

        if (cu_header->initial_length_size != 0 && cu_header->initial_length_size != *bytes_read){
            PyErr_Format(ATOSLError, "cu_header->initial_length_size is not equal to bytes_read");
            fprintf(stderr, "cu_header->initial_length_size is not equal to bytes_read\n");
            return -1;
        }

        cu_header->initial_length_size = *bytes_read;
        cu_header->offset_size = (*bytes_read == 4) ? 4 : 8;
    }

    return length;
}

/* Add an entry to LH's include directory table.  */
static void add_include_dir (struct line_header *lh, char *include_dir)
{
    /* Grow the array if necessary.  */
    if (lh->include_dirs_size == 0)
    {
        lh->include_dirs_size = 1; /* for testing */
        lh->include_dirs = malloc (lh->include_dirs_size * sizeof (*lh->include_dirs));
    }
    else if (lh->num_include_dirs >= lh->include_dirs_size)
    {
        lh->include_dirs_size *= 2;
        lh->include_dirs = realloc (lh->include_dirs, (lh->include_dirs_size * sizeof (*lh->include_dirs)));
    }

    lh->include_dirs[lh->num_include_dirs++] = include_dir;
}

/* Add an entry to LH's file name table.  */
static void add_file_name (struct line_header *lh,
        char *name,
        unsigned int dir_index,
        unsigned int mod_time,
        unsigned int length)
{
    struct file_entry *fe;

    /* Grow the array if necessary.  */
    if (lh->file_names_size == 0)
    {
        lh->file_names_size = 1; /* for testing */
        lh->file_names = malloc (lh->file_names_size
                * sizeof (*lh->file_names));
    }
    else if (lh->num_file_names >= lh->file_names_size)
    {
        lh->file_names_size *= 2;
        lh->file_names = realloc (lh->file_names,
                (lh->file_names_size
                 * sizeof (*lh->file_names)));
    }

    fe = &lh->file_names[lh->num_file_names++];
    fe->name = name;
    fe->dir_index = dir_index;
    fe->mod_time = mod_time;
    fe->length = length;
    fe->included_p = 0;
}

/* Read the statement program header starting at OFFSET in
   .debug_line, according to the endianness of ABFD.  Return a pointer
   to a struct line_header, allocated using xmalloc.

NOTE: the strings in the include directory and file name tables of
the returned object point into debug_line_buffer, and must not be
freed.  */
static struct line_header * dwarf_decode_line_header (unsigned int offset, struct dwarf2_cu *cu)
{
    struct dwarf2_per_objfile *dwarf2_per_objfile = cu->dwarf2_per_objfile;
    //  struct cleanup *back_to;
    struct line_header *lh;
    char *line_ptr;
    /* APPLE LOCAL avoid type warnings by making BYTES_READ unsigned.  */
    unsigned bytes_read;
    int i;
    char *cur_dir, *cur_file;

    if (dwarf2_per_objfile->line_buffer == NULL)
    {
        printf("missing .debug_line section\n");
        PyErr_Format(ATOSLError, "missing .debug_line section");
        return NULL;
    }

    /* Make sure that at least there's room for the total_length field.
       That could be 12 bytes long, but we're just going to fudge that.  */
    if (offset + 4 >= dwarf2_per_objfile->line_size)
    {
        //dwarf2_statement_list_fits_in_line_number_section_complaint ();
        printf(".debug_line incomplete.\n");
        PyErr_Format(ATOSLError, ".debug_line incomplete");
        return NULL;
    }

    lh = malloc (sizeof (*lh));
    memset (lh, 0, sizeof (*lh));
    //back_to = make_cleanup ((make_cleanup_ftype *) free_line_header, (void *) lh);

    line_ptr = dwarf2_per_objfile->line_buffer + offset;

    /* Read in the header.  */
    /* APPLE LOCAL Add cast to avoid type mismatch in arg4 warning.  */
    lh->total_length = read_initial_length_of_comp_unit (line_ptr, &cu->header, (int *) &bytes_read);
    if(lh->total_length == -1){
        return NULL;
    }
    line_ptr += bytes_read;
    if (line_ptr + lh->total_length > (dwarf2_per_objfile->line_buffer
                + dwarf2_per_objfile->line_size))
    {
        printf(".debug_line incomplete.\n");
        PyErr_Format(ATOSLError, ".debug_line incomplete");
        return NULL;
    }
    lh->statement_program_end = line_ptr + lh->total_length;
    lh->version = read_2_bytes (line_ptr);
    line_ptr += 2;
    /* APPLE LOCAL Add cast to avoid type mismatch in arg4 warning.  */
    lh->header_length = read_offset (line_ptr, &cu->header, (int *) &bytes_read);
    line_ptr += bytes_read;
    lh->minimum_instruction_length = read_1_byte (line_ptr);
    line_ptr += 1;
    lh->default_is_stmt = read_1_byte (line_ptr);
    line_ptr += 1;
    lh->line_base = read_1_signed_byte (line_ptr);
    line_ptr += 1;
    lh->line_range = read_1_byte (line_ptr);
    line_ptr += 1;
    lh->opcode_base = read_1_byte (line_ptr);
    line_ptr += 1;
    lh->standard_opcode_lengths = (unsigned char *) malloc (lh->opcode_base * sizeof (unsigned char));

    lh->standard_opcode_lengths[0] = 1;  /* This should never be used anyway.  */
    for (i = 1; i < lh->opcode_base; ++i)
    {
        lh->standard_opcode_lengths[i] = read_1_byte (line_ptr);
        line_ptr += 1;
    }

    /* Read directory table.  */
    while ((cur_dir = read_string (line_ptr, &bytes_read)) != NULL)
    {
        line_ptr += bytes_read;
        add_include_dir (lh, cur_dir);
    }
    line_ptr += bytes_read;

    /* Read file name table.  */
    while ((cur_file = read_string (line_ptr, &bytes_read)) != NULL)
    {
        unsigned int dir_index, mod_time, length;

        line_ptr += bytes_read;
        dir_index = read_unsigned_leb128 (line_ptr, &bytes_read);
        line_ptr += bytes_read;
        mod_time = read_unsigned_leb128 (line_ptr, &bytes_read);
        line_ptr += bytes_read;
        length = read_unsigned_leb128 (line_ptr, &bytes_read);
        line_ptr += bytes_read;

        add_file_name (lh, cur_file, dir_index, mod_time, length);
    }
    line_ptr += bytes_read;
    lh->statement_program_start = line_ptr;

    if (line_ptr > (dwarf2_per_objfile->line_buffer + dwarf2_per_objfile->line_size)){
        printf("line number info header doesn't fit in `.debug_line' section\n");
        PyErr_Format(ATOSLError, "line number info header doesn't fit in `.debug_line' section");
        return NULL;
    }

    //    discard_cleanups (back_to);
    return lh;
}

/* Add a linetable entry for line number LINE and address PC to the
   line vector for SUBFILE.  */

static void record_line (struct subfile *subfile, int line, CORE_ADDR pc)
{
    struct linetable_entry *e;
    /* Ignore the dummy line number in libg.o */

    if (line == 0xffff)
    {
        return;
    }

    /* Make sure line vector exists and is big enough.  */
    if (!subfile->line_vector)
    {
        subfile->line_vector_length = INITIAL_LINE_VECTOR_LENGTH;
        subfile->line_vector = (struct linetable *) malloc (sizeof (struct linetable) + subfile->line_vector_length * sizeof (struct linetable_entry));
        subfile->line_vector->nitems = 0;
        /* APPLE LOCAL codewarrior support */
        subfile->line_vector->lines_are_chars = 0;
        //have_line_numbers = 1;
    }

    if (subfile->line_vector->nitems + 1 >= subfile->line_vector_length)
    {
        subfile->line_vector_length *= 2;
        subfile->line_vector = (struct linetable *) realloc ((char *) subfile->line_vector,
                (sizeof (struct linetable)
                 + (subfile->line_vector_length
                     * sizeof (struct linetable_entry))));
    }

    e = subfile->line_vector->item + subfile->line_vector->nitems++;
    e->line = line;
    e->pc = pc;
    //  e->pc = ADDR_BITS_REMOVE(pc);
}

/* Needed in order to sort line tables from IBM xcoff files.  Sigh!  */

/* APPLE LOCAL make compare_line_numbers extern */
//static int compare_line_numbers (const void *ln1p, const void *ln2p)
//{
//    struct linetable_entry *ln1 = (struct linetable_entry *) ln1p;
//    struct linetable_entry *ln2 = (struct linetable_entry *) ln2p;
//
//    /* Note: this code does not assume that CORE_ADDRs can fit in ints.
//       Please keep it that way.  */
//    if (ln1->pc < ln2->pc)
//        return -1;
//
//    if (ln1->pc > ln2->pc)
//        return 1;
//
//    /* If pc equal, sort by line.  I'm not sure whether this is optimum
//       behavior (see comment at struct linetable in symtab.h).  */
//    return ln1->line - ln2->line;
//}


/* Decode the Line Number Program (LNP) for the given line_header
   structure and CU.  The actual information extracted and the type
   of structures created from the LNP depends on the value of PST.

   1. If PST is NULL, then this procedure uses the data from the program
   to create all necessary symbol tables, and their linetables.
   The compilation directory of the file is passed in COMP_DIR,
   and must not be NULL.

   2. If PST is not NULL, this procedure reads the program to determine
   the list of files included by the unit represented by PST, and
   builds all the associated partial symbol tables.  In this case,
   the value of COMP_DIR is ignored, and can thus be NULL (the COMP_DIR
   is not used to compute the full name of the symtab, and therefore
   omitting it when building the partial symtab does not introduce
   the potential for inconsistency - a partial symtab and its associated
   symbtab having a different fullname -).  */

static struct subfile * dwarf_decode_lines (struct line_header *lh, char *comp_dir, struct dwarf2_cu *cu)
{
    char *line_ptr;
    char *line_end;
    unsigned int bytes_read;
    unsigned char op_code, extended_op, adj_opcode;
    CORE_ADDR baseaddr;
    //struct objfile *objfile = cu->objfile;
    //    const int decode_for_pst_p = (pst != NULL);
    const int decode_for_pst_p = 0;

    /* APPLE LOCAL: We'll need to skip linetable entries in functions that
       were coalesced out.  */
    int record_linetable_entry = 1;
    struct subfile *current_subfile = malloc (sizeof (struct subfile));
    memset(current_subfile, 0, sizeof(struct subfile));
    /* APPLE LOCAL */
    //if (debug_debugmap)
    //    fprintf_unfiltered (gdb_stdlog,
    //            "debugmap: reading line program for %s\n",
    //            cu->per_cu->psymtab->filename);

    //baseaddr = ANOFFSET (objfile->section_offsets, SECT_OFF_TEXT (objfile));
    baseaddr = 0;

    line_ptr = lh->statement_program_start;
    line_end = lh->statement_program_end;

    /* Read the statement sequences until there's nothing left.  */
    while (line_ptr < line_end)
    {
        /* state machine registers  */
        CORE_ADDR address = 0;
        unsigned int file = 1;
        unsigned int line = 1;
        //unsigned int column = 0;
        int is_stmt = lh->default_is_stmt;
        //int basic_block = 0;
        int end_sequence = 0;

        //if (!decode_for_pst_p && lh->num_file_names >= file)
        //{
        //    /* Start a subfile for the current file of the state machine.  */
        //    /* lh->include_dirs and lh->file_names are 0-based, but the
        //       directory and file name numbers in the statement program
        //       are 1-based.  */
        //    struct file_entry *fe = &lh->file_names[file - 1];
        //    char *dir;

        //    if (fe->dir_index)
        //        dir = lh->include_dirs[fe->dir_index - 1];
        //    else
        //        dir = comp_dir;
        //    /* APPLE LOCAL: Pass in the compilation directory of this CU.  */
        //    dwarf2_start_subfile (fe->name, dir, cu->comp_dir);
        //}

        /* Decode the table.  */
        while (!end_sequence)
        {
            op_code = read_1_byte (line_ptr);
            line_ptr += 1;

            if (op_code >= lh->opcode_base)
            {
                /* Special operand.  */
                adj_opcode = op_code - lh->opcode_base;
                address += (adj_opcode / lh->line_range)
                    * lh->minimum_instruction_length;
                line += lh->line_base + (adj_opcode % lh->line_range);
                lh->file_names[file - 1].included_p = 1;
                /* APPLE LOCAL: Skip linetable entries coalesced out */
                if (!decode_for_pst_p && record_linetable_entry)
                {
                    /* Append row to matrix using current values.  */
                    //FIXME check_cu_functions
                    //record_line (current_subfile, line, check_cu_functions (address, cu));
                    record_line (current_subfile, line, address);
                }
                //basic_block = 1;
            }
            else switch (op_code)
            {
                case DW_LNS_extended_op:
                    read_unsigned_leb128 (line_ptr, &bytes_read);
                    line_ptr += bytes_read;
                    extended_op = read_1_byte (line_ptr);
                    line_ptr += 1;
                    switch (extended_op)
                    {
                        case DW_LNE_end_sequence:
                            end_sequence = 1;
                            lh->file_names[file - 1].included_p = 1;
                            /* APPLE LOCAL: Skip linetable entries coalesced out */
                            if (!decode_for_pst_p && record_linetable_entry){
                                //record_line (current_subfile, 0, address);
                                record_line (current_subfile, line, address);
                            }
                            break;
                        case DW_LNE_set_address:
                            /* APPLE LOCAL Add cast to avoid type mismatch in arg4 warn.*/
                            address = read_address_of_cu (line_ptr, cu, (int *) &bytes_read);
                            /* APPLE LOCAL: debug map */
                            {
                                //CORE_ADDR addr;
                                //FIXME
                                //if (translate_debug_map_address (cu, address, &addr, 0))
                                //{
                                //    address = addr;
                                //    record_linetable_entry = 1;
                                //}
                                //else
                                record_linetable_entry = 1;
                            }
                            line_ptr += bytes_read;
                            address += baseaddr;
                            break;
                        case DW_LNE_define_file:
                            {
                                char *cur_file;
                                unsigned int dir_index, mod_time, length;

                                cur_file = read_string (line_ptr, &bytes_read);
                                line_ptr += bytes_read;
                                dir_index =
                                    read_unsigned_leb128 (line_ptr, &bytes_read);
                                line_ptr += bytes_read;
                                mod_time =
                                    read_unsigned_leb128 (line_ptr, &bytes_read);
                                line_ptr += bytes_read;
                                length =
                                    read_unsigned_leb128 (line_ptr, &bytes_read);
                                line_ptr += bytes_read;
                                add_file_name (lh, cur_file, dir_index, mod_time, length);
                            }
                            break;
                        default:
                            printf("mangled .debug_line section\n");
                            return NULL;
                    }
                    break;
                case DW_LNS_copy:
                    lh->file_names[file - 1].included_p = 1;
                    /* APPLE LOCAL: Skip linetable entries coalesced out */
                    if (!decode_for_pst_p && record_linetable_entry)
                        //                        record_line (current_subfile, line, check_cu_functions (address, cu));
                        record_line (current_subfile, line, address);
                    //basic_block = 0;
                    break;
                case DW_LNS_advance_pc:
                    address += lh->minimum_instruction_length
                        * read_unsigned_leb128 (line_ptr, &bytes_read);
                    line_ptr += bytes_read;
                    break;
                case DW_LNS_advance_line:
                    line += read_signed_leb128 (line_ptr, &bytes_read);
                    line_ptr += bytes_read;
                    break;
                case DW_LNS_set_file:
                    {
                        /* The arrays lh->include_dirs and lh->file_names are
                           0-based, but the directory and file name numbers in
                           the statement program are 1-based.  */

                        file = read_unsigned_leb128 (line_ptr, &bytes_read);
                        line_ptr += bytes_read;

                        //struct file_entry *fe;
                        //fe = &lh->file_names[file - 1];
                        //char *dir = NULL;
                        //if (fe->dir_index)
                        //    dir = lh->include_dirs[fe->dir_index - 1];
                        //else
                        //    dir = comp_dir;
                        /* APPLE LOCAL: Pass in the compilation dir of this CU.  */
                        //FIXME
                        //if (!decode_for_pst_p)
                        //    dwarf2_start_subfile (fe->name, dir, cu->comp_dir);
                    }
                    break;
                case DW_LNS_set_column:
                    //column = read_unsigned_leb128 (line_ptr, &bytes_read);
                    read_unsigned_leb128 (line_ptr, &bytes_read);
                    line_ptr += bytes_read;
                    break;
                case DW_LNS_negate_stmt:
                    is_stmt = (!is_stmt);
                    break;
                case DW_LNS_set_basic_block:
                    //basic_block = 1;
                    break;
                    /* Add to the address register of the state machine the
                       address increment value corresponding to special opcode
                       255.  I.e., this value is scaled by the minimum
                       instruction length since special opcode 255 would have
                       scaled the the increment.  */
                case DW_LNS_const_add_pc:
                    address += (lh->minimum_instruction_length
                            * ((255 - lh->opcode_base) / lh->line_range));
                    break;
                case DW_LNS_fixed_advance_pc:
                    address += read_2_bytes (line_ptr);
                    line_ptr += 2;
                    break;
                default:
                    {
                        /* Unknown standard opcode, ignore it.  */
                        int i;

                        for (i = 0; i < lh->standard_opcode_lengths[op_code]; i++)
                        {
                            (void) read_unsigned_leb128 (line_ptr, &bytes_read);
                            line_ptr += bytes_read;
                        }
                    }
            }
        }
    }

    return current_subfile;
}

//static void set_cu_language (unsigned int lang, struct dwarf2_cu *cu)
//{
//    switch (lang)
//    {
//        case DW_LANG_C89:
//        case DW_LANG_C:
//            cu->language = language_c;
//            break;
//        case DW_LANG_C_plus_plus:
//            cu->language = language_cplus;
//            break;
//        case DW_LANG_Fortran77:
//        case DW_LANG_Fortran90:
//        case DW_LANG_Fortran95:
//            cu->language = language_fortran;
//            break;
//        case DW_LANG_Mips_Assembler:
//            cu->language = language_asm;
//            break;
//        case DW_LANG_Java:
//            cu->language = language_java;
//            break;
//        case DW_LANG_Ada83:
//        case DW_LANG_Ada95:
//            cu->language = language_ada;
//            break;
//            /* APPLE LOCAL:  No need to be Apple local but not merged in to FSF..  */
//        case DW_LANG_ObjC:
//            cu->language = language_objc;
//            break;
//            /* APPLE LOCAL:  No need to be Apple local but not merged in to FSF..  */
//        case DW_LANG_ObjC_plus_plus:
//            cu->language = language_objcplus;
//            break;
//        case DW_LANG_Cobol74:
//        case DW_LANG_Cobol85:
//        case DW_LANG_Pascal83:
//        case DW_LANG_Modula2:
//        default:
//            cu->language = language_minimal;
//            break;
//    }
//    //cu->language_defn = language_def (cu->language);
//}
//
void free_dwarf2_per_objfile(struct dwarf2_per_objfile *dwarf2_per_objfile){
    if(dwarf2_per_objfile->info_buffer){
        free(dwarf2_per_objfile->info_buffer);
    }
    if(dwarf2_per_objfile->abbrev_buffer){
        free(dwarf2_per_objfile->abbrev_buffer);
    }
    if(dwarf2_per_objfile->line_buffer){
        free(dwarf2_per_objfile->line_buffer);
    }
    if(dwarf2_per_objfile->pubnames_buffer){
        free(dwarf2_per_objfile->pubnames_buffer);
    }
    if(dwarf2_per_objfile->aranges_buffer){
        free(dwarf2_per_objfile->aranges_buffer);
    }
    if(dwarf2_per_objfile->loc_buffer){
        free(dwarf2_per_objfile->loc_buffer);
    }
    if(dwarf2_per_objfile->macinfo_buffer){
        free(dwarf2_per_objfile->macinfo_buffer);
    }
    if(dwarf2_per_objfile->str_buffer){
        free(dwarf2_per_objfile->str_buffer);
    }
    if(dwarf2_per_objfile->ranges_buffer){
        free(dwarf2_per_objfile->ranges_buffer);
    }
    if(dwarf2_per_objfile->inlined_buffer){
        free(dwarf2_per_objfile->inlined_buffer);
    }
    if(dwarf2_per_objfile->pubtypes_buffer){
        free(dwarf2_per_objfile->pubtypes_buffer);
    }
    if(dwarf2_per_objfile->frame_buffer){
        free(dwarf2_per_objfile->frame_buffer);
    }
    if(dwarf2_per_objfile->eh_frame_buffer){
        free(dwarf2_per_objfile->eh_frame_buffer);
    }

    free(dwarf2_per_objfile);
}

void get_uuid_of_thin(struct thin_macho*thin_macho, char*uuid){
    int i = 0;
    while(i < 16){
        sprintf(uuid + (i * 2), "%02x", thin_macho->uuid[i]);
        i++;
    }
}

static struct dwarf2_per_objfile* parse_dwarf_segment(char *macho_str, long offset,struct segment_command *command){
    uint32_t numofdwarfsections = command->nsects;

    struct dwarf2_per_objfile *dwarf2_per_objfile = malloc(sizeof(struct dwarf2_per_objfile));
    if (dwarf2_per_objfile == NULL){
        printf("Malloc Error!\n");
        PyErr_NoMemory();
        return NULL;
    }
    memset(dwarf2_per_objfile, '\0', sizeof(struct dwarf2_per_objfile));

    struct section * dwarf_section_headers = malloc(numofdwarfsections * sizeof(struct section));
    if (dwarf_section_headers == NULL){
        printf("Malloc Error!\n");
        PyErr_NoMemory();
        return NULL;
    }
    memset(dwarf_section_headers, '\0', numofdwarfsections * sizeof (struct section));
    memcpy(dwarf_section_headers, macho_str + offset, numofdwarfsections * sizeof(struct section));

    uint32_t i = 0;
    while(i < numofdwarfsections){
        char *temp = malloc(dwarf_section_headers[i].size);
        if (temp == NULL){
            printf("Malloc Error!\n");
            PyErr_NoMemory();
            return NULL;
        }
        memset(temp, '\0', dwarf_section_headers[i].size);
        memcpy(temp, macho_str + dwarf_section_headers[i].offset, dwarf_section_headers[i].size);

        if(strcmp(dwarf_section_headers[i].sectname, "__debug_abbrev") == 0){
            dwarf2_per_objfile->abbrev_buffer = temp;
            dwarf2_per_objfile->abbrev_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_aranges") == 0){
            dwarf2_per_objfile->aranges_buffer = temp;
            dwarf2_per_objfile->aranges_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_info") == 0){
            dwarf2_per_objfile->info_buffer = temp;
            dwarf2_per_objfile->info_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_inlined") == 0){
            dwarf2_per_objfile->inlined_buffer = temp;
            dwarf2_per_objfile->inlined_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_line") == 0){
            dwarf2_per_objfile->line_buffer = temp;
            dwarf2_per_objfile->line_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_loc") == 0){
            dwarf2_per_objfile->loc_buffer = temp;
            dwarf2_per_objfile->loc_size = dwarf_section_headers[i].size;
        }else if((strcmp(dwarf_section_headers[i].sectname, "__debug_pubnames") == 0) ||
                (strcmp(dwarf_section_headers[i].sectname, "__debug_pubnames__DWARF") == 0)
                ){
            dwarf2_per_objfile->pubnames_buffer = temp;
            dwarf2_per_objfile->pubnames_size = dwarf_section_headers[i].size;
        }else if((strcmp(dwarf_section_headers[i].sectname, "__debug_pubtypes") == 0) ||
                (strcmp(dwarf_section_headers[i].sectname, "__debug_pubtypes__DWARF") == 0)
                ){
            dwarf2_per_objfile->pubtypes_buffer = temp;
            dwarf2_per_objfile->pubtypes_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_ranges") == 0){
            dwarf2_per_objfile->ranges_buffer = temp;
            dwarf2_per_objfile->ranges_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_str") == 0){
            dwarf2_per_objfile->str_buffer = temp;
            dwarf2_per_objfile->str_size = dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_frame") == 0){
            //do nothing for now
            free(temp);
        }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_names") == 0){
            //do nothing for now
            free(temp);
        }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_types") == 0){
            //do nothing for now
            free(temp);
        }else if((strcmp(dwarf_section_headers[i].sectname, "__apple_namespa") == 0) ||
                (strcmp(dwarf_section_headers[i].sectname, "__apple_namespac__DWARF") == 0)
                ){
            //do nothing for now
            free(temp);
        }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_objc") == 0){
            //do nothing for now
            free(temp);
        }else{
            printf("╮(╯▽╰)╭ Unknown Section, %s \n", dwarf_section_headers[i].sectname);
            free(temp);
        }
        i++;
    }
    free(dwarf_section_headers);
    return dwarf2_per_objfile;
}


static struct dwarf2_per_objfile* parse_dwarf_segment_64(char *macho_str, long offset,struct segment_command_64 *command){
    uint32_t numofdwarfsections = command->nsects;

    struct dwarf2_per_objfile *dwarf2_per_objfile = malloc(sizeof(struct dwarf2_per_objfile));
    if (dwarf2_per_objfile == NULL){
        printf("Malloc Error!\n");
        PyErr_NoMemory();
        return NULL;
    }
    memset(dwarf2_per_objfile, '\0', sizeof(struct dwarf2_per_objfile));

    struct section_64 * dwarf_section_headers = malloc(numofdwarfsections * sizeof(struct section_64));
    if (dwarf_section_headers == NULL){
        printf("Malloc Error!\n");
        PyErr_NoMemory();
        return NULL;
    }
    memset(dwarf_section_headers, '\0', numofdwarfsections * sizeof (struct section));
    memcpy(dwarf_section_headers, macho_str + offset, numofdwarfsections * sizeof(struct section_64));

    uint32_t i = 0;
    while(i < numofdwarfsections){
        //FIXME more the size more 4G
        char *temp = malloc((uint32_t)dwarf_section_headers[i].size);
        if (temp == NULL){
            printf("Malloc Error!\n");
            PyErr_NoMemory();
            return NULL;
        }
        memset(temp, '\0', (uint32_t)dwarf_section_headers[i].size);
        memcpy(temp, macho_str + dwarf_section_headers[i].offset, (uint32_t)dwarf_section_headers[i].size);

        if(strcmp(dwarf_section_headers[i].sectname, "__debug_abbrev") == 0){
            dwarf2_per_objfile->abbrev_buffer = temp;
            dwarf2_per_objfile->abbrev_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_aranges") == 0){
            dwarf2_per_objfile->aranges_buffer = temp;
            dwarf2_per_objfile->aranges_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_info") == 0){
            dwarf2_per_objfile->info_buffer = temp;
            dwarf2_per_objfile->info_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_inlined") == 0){
            dwarf2_per_objfile->inlined_buffer = temp;
            dwarf2_per_objfile->inlined_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_line") == 0){
            dwarf2_per_objfile->line_buffer = temp;
            dwarf2_per_objfile->line_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_loc") == 0){
            dwarf2_per_objfile->loc_buffer = temp;
            dwarf2_per_objfile->loc_size = (uint32_t)dwarf_section_headers[i].size;
        }else if((strcmp(dwarf_section_headers[i].sectname, "__debug_pubnames") == 0) ||
                (strcmp(dwarf_section_headers[i].sectname, "__debug_pubnames__DWARF") == 0)
                ){
            dwarf2_per_objfile->pubnames_buffer = temp;
            dwarf2_per_objfile->pubnames_size = (uint32_t)dwarf_section_headers[i].size;
        }else if((strcmp(dwarf_section_headers[i].sectname, "__debug_pubtypes") == 0) ||
                (strcmp(dwarf_section_headers[i].sectname, "__debug_pubtypes__DWARF") == 0)
                ){
            dwarf2_per_objfile->pubtypes_buffer = temp;
            dwarf2_per_objfile->pubtypes_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_ranges") == 0){
            dwarf2_per_objfile->ranges_buffer = temp;
            dwarf2_per_objfile->ranges_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_str") == 0){
            dwarf2_per_objfile->str_buffer = temp;
            dwarf2_per_objfile->str_size = (uint32_t)dwarf_section_headers[i].size;
        }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_frame") == 0){
            //do nothing for now
            free(temp);
        }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_names") == 0){
            //do nothing for now
            free(temp);
        }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_types") == 0){
            //do nothing for now
            free(temp);
        }else if((strcmp(dwarf_section_headers[i].sectname, "__apple_namespa") == 0) ||
                (strcmp(dwarf_section_headers[i].sectname, "__apple_namespac__DWARF") == 0)
                ){
            //do nothing for now
            free(temp);
        }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_objc") == 0){
            //do nothing for now
            free(temp);
        }else{
            printf("╮(╯▽╰)╭ Unknown Section, %s \n", dwarf_section_headers[i].sectname);
            free(temp);
        }
        i++;
    }
    free(dwarf_section_headers);
    return dwarf2_per_objfile;
}

void print_all_dwarf2_per_objfile(struct dwarf2_per_objfile *dwarf2_per_objfile){
    int i = 0;
    printf("abbrev_buffer:\n");
    for (i =0; i< dwarf2_per_objfile->abbrev_size; i++){
        printf("%02x", dwarf2_per_objfile->abbrev_buffer[i]);
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("aranges_buffer:\n");

    for (i =0; i< dwarf2_per_objfile->aranges_size; i++){
        printf("%02x", dwarf2_per_objfile->aranges_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");

    printf("info_buffer:\n");
    for (i =0; i< dwarf2_per_objfile->info_size; i++){
        printf("%02x", dwarf2_per_objfile->info_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");

    printf("inlined_buffer:\n");
    for (i =0; i< dwarf2_per_objfile->inlined_size; i++){
        printf("%02x", dwarf2_per_objfile->inlined_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");

    printf("line_buffer:\n");
    for (i =0; i< dwarf2_per_objfile->line_size; i++){
        printf("%02x", dwarf2_per_objfile->line_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("loc_buffer:\n");

    for (i =0; i< dwarf2_per_objfile->loc_size; i++){
        printf("%02x", dwarf2_per_objfile->loc_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");

    printf("pubnames_buffer:\n");
    for (i =0; i< dwarf2_per_objfile->pubnames_size; i++){
        printf("%02x", dwarf2_per_objfile->pubnames_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("pubtypes_buffer:\n");

    for (i =0; i< dwarf2_per_objfile->pubtypes_size; i++){
        printf("%02x", dwarf2_per_objfile->pubtypes_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");

    printf("ranges_buffer:\n");
    for (i =0; i< dwarf2_per_objfile->ranges_size; i++){
        printf("%02x", dwarf2_per_objfile->ranges_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("str_size:\n");

    for (i =0; i< dwarf2_per_objfile->str_size; i++){
        printf("%02x", dwarf2_per_objfile->str_buffer[i]);

    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
}


int parse_normal(FILE *fp, uint32_t magic_number, struct target_file *tf){
    tf->numofarchs = 1;
    tf->thin_machos = malloc(1 *sizeof(struct thin_macho*));
    if (tf->thin_machos == NULL){
        PyErr_NoMemory();
        return -1;
    }
    memset(tf->thin_machos, '\0', 1 * sizeof(struct thin_macho*));

    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    int numofbytes = 0;
    tf->thin_machos[0] = malloc(sizeof(struct thin_macho));
    if(tf->thin_machos[0] == NULL){
        PyErr_NoMemory();
        return -1;
    }
    tf->thin_machos[0]->data = malloc(size);
    if(tf->thin_machos[0]->data == NULL){
        PyErr_NoMemory();
        return -1;
    }
    memset(tf->thin_machos[0]->data, '\0', size);

    numofbytes = fread(tf->thin_machos[0]->data, sizeof(char), size, fp);
    assert(numofbytes == size);
    if(numofbytes == size){
        parse_macho(tf->thin_machos[0]);
        return 0;
    }else{
        return -1;
    }
}

void free_buffers(struct dwarf2_per_objfile *dwarf2_per_objfile){
    free(dwarf2_per_objfile->info_buffer);
    free(dwarf2_per_objfile->abbrev_buffer);
    free(dwarf2_per_objfile->line_buffer);
    free(dwarf2_per_objfile->pubnames_buffer);
    free(dwarf2_per_objfile->aranges_buffer);
    free(dwarf2_per_objfile->loc_buffer);
    free(dwarf2_per_objfile->macinfo_buffer);
    free(dwarf2_per_objfile->str_buffer);
    free(dwarf2_per_objfile->ranges_buffer);
    free(dwarf2_per_objfile->inlined_buffer);
    free(dwarf2_per_objfile->pubtypes_buffer);
    free(dwarf2_per_objfile->frame_buffer);
    free(dwarf2_per_objfile->eh_frame_buffer);
}

void free_dwarf_abbrev_hash(struct dwarf2_per_objfile *dwarf2_per_objfile){
    if(dwarf2_per_objfile->dwarf2_abbrevs == NULL){
        return;
    }
    int i = 0;
    while(i < ABBREV_HASH_SIZE){
        struct abbrev_info *dwarf2_abbrevs= dwarf2_per_objfile->dwarf2_abbrevs[i];
        struct abbrev_info *current = dwarf2_abbrevs;
        while(current != NULL){
            current = current->next;
            free(current);
        }
        i++;
    }
    free(dwarf2_per_objfile->dwarf2_abbrevs);
}

void free_dwarf_aranges(struct dwarf2_per_objfile *dwarf2_per_objfile){
    struct arange **all_aranges = dwarf2_per_objfile->all_aranges;
    int i = 0;
    while(i < dwarf2_per_objfile->n_aranges){
        free(all_aranges[i]->address_range_descriptors);
        free(all_aranges[i]);
        i++;
    }
}

/* Free a linked list of dies.  */
static void free_die_list (struct die_info *dies)
{
    struct die_info *die, *next;
    int i = 0;
    die = dies;
    while (die)
    {
        if (die->child != NULL)
            free_die_list (die->child);
        next = die->sibling;

        while(i < die->num_attrs){
            if ((die->attrs[i].form == DW_FORM_block2)
             || (die->attrs[i].form == DW_FORM_block4)
             || (die->attrs[i].form == DW_FORM_block)
             || (die->attrs[i].form == DW_FORM_block1)){
                 free((die->attrs[i]).u.blk);
             }
            i++;
        }
        free (die->attrs);
        free (die);
        die = next;
    }
}

static void free_compilation_units(struct dwarf2_per_objfile *dwarf2_per_objfile){
    struct dwarf2_per_cu_data **all_comp_units = dwarf2_per_objfile->all_comp_units;
    int i = 0;
    while(i < dwarf2_per_objfile->n_comp_units){
        struct dwarf2_cu* cu= all_comp_units[i]->cu;
        if (cu->dies){
            free_die_list (cu->dies);
        }
        free(cu);
        i++;
    }
    free(dwarf2_per_objfile->all_comp_units);
}

void free_target_file(struct target_file *tf){
    uint32_t i = 0;
    while(i < tf->numofarchs){
        free(tf->thin_machos[i]->data);
        if(tf->thin_machos[i]->dwarf2_per_objfile){
            struct dwarf2_per_objfile *dwarf2_per_objfile = tf->thin_machos[i]->dwarf2_per_objfile;
            if (dwarf2_per_objfile != NULL){
                free_buffers(dwarf2_per_objfile);
                //free all compilation units
                free_compilation_units(dwarf2_per_objfile);
                //free aranges
                free_dwarf_aranges(dwarf2_per_objfile);
                //free dwarf abbrev hash
                free_dwarf_abbrev_hash(dwarf2_per_objfile);
                //free dwarf file
                free(dwarf2_per_objfile);
            }
        }
        free(tf->thin_machos[i]->all_symbols);
        free(tf->thin_machos[i]);
        i++;
    }
    free(tf->thin_machos);
    free(tf);
}

int select_thin_macho_by_arch(struct target_file *tf, const char *target_arch){
    int i = 0;
    char *arch = NULL;
    while(i < tf->numofarchs){
        struct thin_macho *thin_macho = tf->thin_machos[i];
        switch(thin_macho->cputype){
           case CPU_TYPE_ARM:
               {
                   switch (thin_macho->cpusubtype){
                        case CPU_SUBTYPE_ARM_V4T:
                            //armv4t
                            arch = "armv4t";
                            break;
                        case CPU_SUBTYPE_ARM_V5TEJ:
                            //armv5
                            arch = "armv5";
                            break;
                        case CPU_SUBTYPE_ARM_V6:
                            //armv6
                            arch = "armv6";
                            break;
                        case CPU_SUBTYPE_ARM_V7:
                            //armv7
                            arch = "armv7";
                            break;
                        case CPU_SUBTYPE_ARM_V7S:
                            //armv7s
                            arch = "armv7s";
                            break;
                        case CPU_SUBTYPE_ARM_V8:
                            //armv8
                            arch = "armv8";
                            break;
                   }
                   break;
               }
           case CPU_TYPE_I386:
               //i386
               arch = "i386";
               break;
            case CPU_TYPE_X86_64:
               //x86_64
               arch = "x86_64";
               break;
            case CPU_TYPE_POWERPC:
               //ppc
               arch = "ppc";
               break;
            case CPU_TYPE_POWERPC64:
               //ppc64
               arch = "ppc64";
               break;
        }
        if (arch != NULL && strcmp(arch, target_arch) == 0){
            return i;
        }
        i++;
    }
    if (arch == NULL){
        printf("unknow arch: %s\n", target_arch);
    }
    return -1;
}

struct target_file *parse_file(const char *filename){
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        fprintf(stderr, "Can not open file %s for read.\n", filename);
        PyErr_Format(ATOSLError, "Can not open file %s for read\n", filename);
        return NULL;
    }

    //tell file type by their magic number
    //Values for integer types in all Mach-O data structures are written
    //using the host CPU’s byte ordering scheme, except for fat_header and
    //fat_arch, which are written in big-endian byte order.

    struct target_file *tf = malloc(sizeof(struct target_file));
    if (tf == NULL){
        PyErr_NoMemory();
        return NULL;
    }
    memset(tf, '\0', sizeof(struct target_file));

    int rc = 0;
    int seekreturn = 0;
    uint32_t magic_number = 0;
    int parse_result = -1;
    if( (rc = fread(&magic_number, sizeof(uint32_t), 1, fp)) != 0 )
    {
        seekreturn = fseek (fp, 0 - sizeof(uint32_t), SEEK_CUR);
        assert(seekreturn == 0);
        if (seekreturn != 0){
            debug("seekreturn != 0");
            PyErr_Format(ATOSLError, "seek error");
            fclose(fp);
            return NULL;
        }
        debug("magic_number: %x\n", magic_number);
        switch(magic_number){
            case MH_MAGIC:
                //current machine endian is same with host machine
                parse_result = parse_normal(fp, MH_MAGIC, tf);
                break;
            case MH_MAGIC_64:
                //current machine endian is same with host machine
                parse_result = parse_normal(fp, MH_MAGIC_64, tf);
                break;
            case MH_CIGAM:
                printf("MH_CIGAM: %x\n", MH_CIGAM);
                PyErr_Format(ATOSLError, "MH_CIGAM: %x\n", MH_CIGAM);
                break;
            case MH_CIGAM_64:
                //current machine endian is not same with host machine
                printf("MH_CIGAM_64: %x\n", MH_CIGAM_64);
                PyErr_Format(ATOSLError, "MH_CIGAM_64: %x\n", MH_CIGAM_64);
                break;
            case FAT_MAGIC:
                //current machine is big endian
                parse_result = parse_universal(fp, FAT_MAGIC, tf);
                break;
            case FAT_CIGAM:
                //current machie is small endian
                parse_result = parse_universal(fp, FAT_CIGAM, tf);
                break;
            default:
                fprintf(stderr, "magic_number invalid.");
                PyErr_Format(ATOSLError, "magic_number invalid");
        }
    }
    fclose(fp);
    if(parse_result == -1){
        return NULL;
    }
    return tf;
}

//static void int32_endian_convert(int32_t *num)
//{
//    int32_t original_num = *num;
//    *num = 0;
//    int i;
//    for (i = 0; i < 4; i++)
//    {
//        *num <<= 8;
//        *num |= (original_num & 0xFF);
//        original_num >>= 8;
//    }
//}

static void uint32_endian_convert(uint32_t *num)
{
    uint32_t original_num = *num;
    *num = 0;
    int i;
    for (i = 0; i < 4; i++)
    {
        *num <<= 8;
        *num |= (original_num & 0xFF);
        original_num >>= 8;
    }
}

static void integer_t_endian_convert(integer_t *num)
{
    integer_t original_num = *num;
    *num = 0;
    int i;
    for (i = 0; i < 4; i++)
    {
        *num <<= 8;
        *num |= (original_num & 0xFF);
        original_num >>= 8;
    }
}

int parse_fat_arch(FILE *fp, struct fat_arch *fa, struct thin_macho**thin_macho, uint32_t magic_number){
    if (magic_number == FAT_CIGAM){
        integer_t_endian_convert(&fa->cputype);
        integer_t_endian_convert(&fa->cpusubtype);
        uint32_endian_convert(&fa->offset);
        uint32_endian_convert(&fa->size);
        uint32_endian_convert(&fa->align);
    }

    //printf("offset: 0x%x\n", fa->offset);
    //printf("size: 0x%x\n", fa->size);
    //printf("align: 0x%x\n", fa->align);

    (*thin_macho)->data = malloc(fa->size);
    memset((*thin_macho)->data, '\0', fa->size);

    //record current pos
    long cur_position = ftell(fp);
    int seekreturn = 0;
    seekreturn = fseek(fp, fa->offset, SEEK_SET);
    assert(seekreturn == 0);
    if(seekreturn != 0){
        PyErr_Format(ATOSLError, "seek error");
        fprintf(stderr, "seek error.\n");
        return -1;
    }

    int numofbytes = 0;
    numofbytes = fread((*thin_macho)->data, sizeof(char), fa->size, fp);
    assert(numofbytes == fa->size);
    if(numofbytes != fa->size){
        PyErr_Format(ATOSLError, "read macho data error");
        fprintf(stderr, "read macho data error.\n");
        return -1;
    }
    seekreturn = fseek(fp, cur_position, SEEK_SET);
    assert(seekreturn == 0);
    if(seekreturn != 0){
        PyErr_Format(ATOSLError, "seek error.");
        fprintf(stderr, "seek error.\n");
        return -1;
    }

    return 0;
}

int parse_universal(FILE *fp, uint32_t magic_number, struct target_file *tf){
    int rc = 0;
    struct fat_header fh = {0};

    uint32_t nfat_arch = 0;
    if( (rc = fread(&fh ,sizeof(struct fat_header), 1, fp)) != 0 )
    {
        if (magic_number == FAT_CIGAM){
            uint32_endian_convert(&fh.nfat_arch);
        }
        nfat_arch = fh.nfat_arch;
        //printf("nfat_arch: %u\n", nfat_arch);
    }
    //free maloc failed?
    tf->numofarchs = nfat_arch;
    tf->thin_machos = malloc(nfat_arch *sizeof(struct thin_macho*));
    if (tf->thin_machos == NULL){
        PyErr_NoMemory();
        return -1;
    }
    memset(tf->thin_machos, '\0', nfat_arch * sizeof(struct thin_macho*));

    uint32_t i = 0;
    struct fat_arch fa = {0};
    while (i < nfat_arch){
        tf->thin_machos[i] = malloc(sizeof(struct thin_macho));
        if (tf->thin_machos[i] == NULL){
            PyErr_NoMemory();
            return -1;
        }
        memset(tf->thin_machos[i], '\0', sizeof(struct thin_macho));
        if( (rc = fread(&fa ,sizeof(struct fat_arch), 1, fp)) == 1 )
        {
            int parse_fat_arch_result = parse_fat_arch(fp, &fa, &tf->thin_machos[i], magic_number);
            if(parse_fat_arch_result == -1){
                return -1;
            }
        }else{
            PyErr_Format(ATOSLError, "fread fat arch error");
            fprintf(stderr, "read fat arch error\n");
            return -1;
        }
        //FIXME
        int parse_macho_result = parse_macho(tf->thin_machos[i]);
        if (parse_macho_result == -1){
            return -1;
        }
        i++;
    }
    return 0;
}

int parse_macho(struct thin_macho*tm){
    char *macho_str = tm->data;

    int num_load_cmds = 0;
    long offset = 0;
    size_t header_size = 0;

    uint32_t magic_number = 0;
    memcpy(&magic_number, macho_str, sizeof(uint32_t));
    //printf("magic_number: %x\n", magic_number);
    switch(magic_number){
        case MH_MAGIC:
            {
                //current machine endian is same with host machine
                //printf("MH_MAGIC: %x\n", MH_MAGIC);
                //parse_normal(fp, MH_MAGIC, tf);
                struct mach_header mh = {0};
                header_size = sizeof(struct mach_header);
                memcpy(&mh, macho_str + offset, header_size);
                num_load_cmds = mh.ncmds;
                tm->cputype = mh.cputype;
                tm->cpusubtype = mh.cpusubtype;
                break;
            }
        case MH_MAGIC_64:
            {
                //current machine endian is same with host machine
                //printf("MH_MAGIC_64: %x\n", MH_MAGIC_64);
                //parse_normal(fp, MH_MAGIC_64, tf);
                struct mach_header_64 mh64 = {0};
                header_size = sizeof(struct mach_header_64);
                memcpy(&mh64, macho_str + offset, header_size);
                num_load_cmds = mh64.ncmds;
                tm->cputype = mh64.cputype;
                tm->cpusubtype = mh64.cpusubtype;
                break;
            }
        case MH_CIGAM:
            //current machine endian is not same with host machine
            //printf("MH_CIGAM: %x\n", MH_CIGAM);
            printf("TODO: MH_CIGAM\n");
            PyErr_Format(ATOSLError, "TODO: MH_CIGAM");
            break;
        case MH_CIGAM_64:
            printf("TODO: MH_CIGAM_64\n");
            PyErr_Format(ATOSLError, "TODO: MH_CIGAM_64");
            //current machine endian is not same with host machine
            //printf("MH_CIGAM_64: %x\n", MH_CIGAM_64);
            break;
        case FAT_MAGIC:
        case FAT_CIGAM:
            fprintf(stderr, "fat in fat?\n");
            PyErr_Format(ATOSLError, "fat in fat?");
            return -1;
            break;
        default:
            fprintf(stderr, "magic_number invalid");
            PyErr_Format(ATOSLError, "magic_number invalid");
            return -1;
    }

    offset += header_size;

    struct load_command lc = {0};
    int i = 0;
    while (i < num_load_cmds){
        memcpy(&lc, macho_str + offset, sizeof(struct load_command));
        //because we will try to read the actual load_command depend on
        //load_command type, so we do not need to add the offset.
        //offset += sizeof(struct load_command);
        //printf("%d\n",i);
        int parse_load_command_result = parse_load_command(macho_str, &offset, &lc, tm);
        if (parse_load_command_result == -1){
            return -1;
        }
        i++;
    }
    //    printf("finished\n");
    return 0;
}

/* Return a pointer to just past the end of an LEB128 number in BUF.  */

static unsigned int get_num_attr_spec_pair(char* info_ptr){
    unsigned int bytes_read = 0;
    unsigned int num_attr_spec_pair = 0;
    unsigned int attr_name_code = (unsigned int)read_unsigned_leb128(info_ptr, &bytes_read);
    info_ptr += bytes_read;
    unsigned int attr_form_code = (unsigned int)read_unsigned_leb128(info_ptr, &bytes_read);
    info_ptr += bytes_read;
    while(attr_name_code != 0 || attr_form_code != 0){
        attr_name_code = (unsigned int)read_unsigned_leb128(info_ptr ,&bytes_read);
        info_ptr += bytes_read;
        attr_form_code = (unsigned int)read_unsigned_leb128(info_ptr ,&bytes_read);
        info_ptr += bytes_read;
        num_attr_spec_pair ++;
    }
    return num_attr_spec_pair;
}

/* Lookup an abbrev_info structure in the abbrev hash table.  */

static struct abbrev_info * dwarf2_lookup_abbrev (unsigned int number, struct dwarf2_cu *cu)
{
    unsigned int hash_number;
    struct abbrev_info *abbrev;

    hash_number = number % ABBREV_HASH_SIZE;
    abbrev = cu->dwarf2_abbrevs[hash_number];

    while (abbrev)
    {
        if (abbrev->number == number)
            return abbrev;
        else
            abbrev = abbrev->next;
    }
    return NULL;
}

static char * read_comp_unit_head (struct comp_unit_head *header, char *info_ptr)
{
    header->addr_size = 4;
    header->offset_size = 4;
    unsigned int cu_head_length = read_unsigned_int(info_ptr);
    //printf("length: \t0X%08X\n", cu_head_length);
    info_ptr += 4;
    header->length = cu_head_length;

    unsigned short cu_version = read_unsigned_short(info_ptr);
    info_ptr += 2;
    //printf("version: \t0X%04X\n", cu_version);
    header->version = cu_version;

    unsigned int cu_abbrev_offset= read_unsigned_int(info_ptr);
    info_ptr += 4;
    //printf("abbr_offset: \t0X%08X\n", cu_abbrev_offset);
    header->abbrev_offset = cu_abbrev_offset;

    unsigned char cu_addr_size = read_unsigned_char(info_ptr);
    info_ptr += 1;
    //printf("abbr_size: \t0X%02X\n", cu_addr_size);
    header->addr_size = cu_addr_size;
    return info_ptr;
}

/* Read the initial uleb128 in the die at current position in compilation unit CU.
   Return the corresponding abbrev, or NULL if the number is zero (indicating
   an empty DIE).  In either case *BYTES_READ will be set to the length of
   the initial number.  */

//static struct abbrev_info * peek_die_abbrev (char *info_ptr, unsigned int *bytes_read, struct dwarf2_cu *cu)
//{
//    unsigned int abbrev_number;
//    struct abbrev_info *abbrev;
//
//    abbrev_number = (unsigned int)read_unsigned_leb128(info_ptr, bytes_read);
//    printf("%u\n", abbrev_number);
//
//    if (abbrev_number == 0)
//        return NULL;
//
//    abbrev = dwarf2_lookup_abbrev (abbrev_number, cu);
//    if (!abbrev)
//    {
//        printf("Dwarf Error: Could not find abbrev number %d\n", abbrev_number);
//    }
//
//    return abbrev;
//}

static char * read_n_bytes (char *buf, unsigned int size)
{
    /* If the size of a host char is 8 bits, we can return a pointer
       to the buffer, otherwise we have to copy the data to a buffer
       allocated on the temporary obstack.  */
    //gdb_assert (HOST_CHAR_BIT == 8);
    return buf;
}

static CORE_ADDR read_address_of_arange (char *buf, struct arange *arange, int *bytes_read){
    CORE_ADDR retval = 0;
    switch (arange->aranges_header.addr_size){
        case 2:
            //unsigned
            retval = read_signed_16(buf);
            break;
        case 4:
            retval = read_signed_32(buf);
            break;
        case 8:
            retval = read_signed_64(buf);
            break;
        default:
            fprintf(stderr, "read address: bad switch, signed\n");
    }
    *bytes_read = arange->aranges_header.addr_size;
    return retval;
}

/* memory allocation interface */

static struct dwarf_block * dwarf_alloc_block (struct dwarf2_cu *cu)
{
    struct dwarf_block *blk;
    //FIX ME FREEME
    blk = (struct dwarf_block *) malloc(sizeof (struct dwarf_block));
    memset(blk, '\0', sizeof(struct dwarf_block));
    return (blk);
}



static char * read_indirect_string (char *buf, const struct comp_unit_head *cu_header, unsigned int *bytes_read_ptr, char *debug_str_buffer)
{
    long str_offset = read_offset (buf, cu_header, (int *) bytes_read_ptr);

    //if (dwarf2_per_objfile->str_buffer == NULL)
    //{
    //    error (_("DW_FORM_strp used without .debug_str section [in module %s]"),
    //            bfd_get_filename (abfd));
    //    return NULL;
    //}
    //if (str_offset >= dwarf2_per_objfile->str_size)
    //{
    //    error (_("DW_FORM_strp pointing outside of .debug_str section [in module %s]"),
    //            bfd_get_filename (abfd));
    //    return NULL;
    //}
    //gdb_assert (HOST_CHAR_BIT == 8);
    //if (dwarf2_per_objfile->str_buffer[str_offset] == '\0')
    //    return NULL;
    return debug_str_buffer + str_offset;
}

/* Read an attribute value described by an attribute form.  */

static char * read_attribute_value (struct attribute *attr, unsigned int form, char *info_ptr, struct dwarf2_cu *cu)
{
    struct comp_unit_head *cu_header = &cu->header;
    struct dwarf2_per_objfile *dwarf2_per_objfile = cu->dwarf2_per_objfile;
    unsigned int bytes_read;
    struct dwarf_block *blk;

    attr->form = form;
    switch (form)
    {
        case DW_FORM_addr:
        case DW_FORM_ref_addr:
            /* APPLE LOCAL Add cast to avoid type mismatch in arg4 warning.  */
            attr->u.addr = read_address_of_cu (info_ptr, cu, (int *) &bytes_read);
            info_ptr += bytes_read;
            break;
        case DW_FORM_block2:
            blk = dwarf_alloc_block (cu);
            blk->size = read_2_bytes (info_ptr);
            info_ptr += 2;
            blk->data = read_n_bytes (info_ptr, blk->size);
            info_ptr += blk->size;
            attr->u.blk = blk;
            break;
        case DW_FORM_block4:
            blk = dwarf_alloc_block (cu);
            blk->size = read_4_bytes (info_ptr);
            info_ptr += 4;
            blk->data = read_n_bytes (info_ptr, blk->size);
            info_ptr += blk->size;
            attr->u.blk = blk;
            break;
        case DW_FORM_data2:
            //FIXME
            attr->u.unsnd = read_2_bytes (info_ptr);
            info_ptr += 2;
            break;
        case DW_FORM_data4:
            //FIXME
            attr->u.unsnd = read_4_bytes (info_ptr);
            info_ptr += 4;
            break;
        case DW_FORM_data8:
            //FIXME
            attr->u.unsnd = read_8_bytes (info_ptr);
            info_ptr += 8;
            break;
        case DW_FORM_string:
            attr->u.str = read_string (info_ptr, &bytes_read);
            info_ptr += bytes_read;
            break;
        case DW_FORM_strp:
            attr->u.str = read_indirect_string (info_ptr, cu_header, &bytes_read, dwarf2_per_objfile->str_buffer);
            //printf("attr->u.addr %s\n", attr->u.str);
            info_ptr += bytes_read;
            break;
        case DW_FORM_block:
            blk = dwarf_alloc_block (cu);
            blk->size = read_unsigned_leb128 (info_ptr, &bytes_read);
            info_ptr += bytes_read;
            blk->data = read_n_bytes (info_ptr, blk->size);
            info_ptr += blk->size;
            attr->u.blk = blk;
            break;
        case DW_FORM_block1:
            blk = dwarf_alloc_block (cu);
            blk->size = read_1_byte (info_ptr);
            info_ptr += 1;
            blk->data = read_n_bytes (info_ptr, blk->size);
            info_ptr += blk->size;
            attr->u.blk = blk;
            break;
        case DW_FORM_data1:
            attr->u.unsnd = read_1_byte (info_ptr);
            info_ptr += 1;
            break;
        case DW_FORM_flag:
            attr->u.unsnd = read_1_byte (info_ptr);
            info_ptr += 1;
            break;
        case DW_FORM_sdata:
            attr->u.snd = read_signed_leb128 (info_ptr, &bytes_read);
            info_ptr += bytes_read;
            break;
        case DW_FORM_APPLE_db_str:
        case DW_FORM_udata:
            attr->u.unsnd = read_unsigned_leb128 (info_ptr, &bytes_read);
            info_ptr += bytes_read;
            break;
        case DW_FORM_ref1:
            //FIXME
            attr->u.addr = cu->header.offset + read_1_byte (info_ptr);
            info_ptr += 1;
            break;
        case DW_FORM_ref2:
            attr->u.addr = cu->header.offset + read_2_bytes (info_ptr);
            info_ptr += 2;
            break;
        case DW_FORM_ref4:
            attr->u.addr = cu->header.offset + read_4_bytes (info_ptr);
            info_ptr += 4;
            break;
        case DW_FORM_ref8:
            attr->u.addr = cu->header.offset + read_8_bytes (info_ptr);
            info_ptr += 8;
            break;
        case DW_FORM_ref_udata:
            attr->u.addr = (cu->header.offset + read_unsigned_leb128 (info_ptr, &bytes_read));
            info_ptr += bytes_read;
            break;
        case DW_FORM_indirect:
            form = read_unsigned_leb128 (info_ptr, &bytes_read);
            info_ptr += bytes_read;
            info_ptr = read_attribute_value (attr, form, info_ptr, cu);
            break;
        default:
            fprintf(stderr, "Dwarf Error: Cannot handle  in DWARF reader [in module s]");
            //   dwarf_form_name (form),
            //   bfd_get_filename (abfd));
    }
    return info_ptr;
}


/* Read an attribute described by an abbreviated attribute.  */

static char * read_attribute (struct attribute *attr, struct attr_abbrev *abbrev, char *info_ptr, struct dwarf2_cu *cu)
{
    attr->name = abbrev->name;
    return read_attribute_value (attr, abbrev->form, info_ptr, cu);
}

static struct die_info * dwarf_alloc_die(void)
{
    struct die_info *die;

    die = (struct die_info *) malloc (sizeof (struct die_info));
    memset (die, 0, sizeof (struct die_info));
    return (die);
}


/* Read the die from the .debug_info section buffer.  Set DIEP to
   point to a newly allocated die with its information, except for its
   child, sibling, and parent fields.  Set HAS_CHILDREN to tell
   whether the die has children or not.  */

static char * read_full_die (struct die_info **diep, char *info_ptr,
        struct dwarf2_cu *cu, int *has_children)
{
    unsigned int abbrev_number, i;
    //unsigned int offset;
    unsigned int bytes_read;
    struct abbrev_info *abbrev;
    struct die_info *die;
    char *comp_dir = NULL;

    abbrev_number = read_unsigned_leb128 (info_ptr, &bytes_read);
    info_ptr += bytes_read;
    if (!abbrev_number)
    {
        die = dwarf_alloc_die ();
        die->tag = 0;
        die->abbrev = abbrev_number;
        //die->type = NULL;
        *diep = die;
        *has_children = 0;
        return info_ptr;
    }

    abbrev = dwarf2_lookup_abbrev (abbrev_number, cu);
    if (!abbrev)
    {
        fprintf(stderr, "Dwarf Error: could not find abbrev number %d\n", abbrev_number);
    }
    die = dwarf_alloc_die ();
    //die->offset = offset;
    die->tag = abbrev->tag;
    die->abbrev = abbrev_number;
    //die->type = NULL;

    die->num_attrs = abbrev->num_attrs;
    die->attrs = (struct attribute *)malloc (die->num_attrs * sizeof (struct attribute));
    //    printf("%s\n", dwarf_tag_name(die->tag));

    for (i = 0; i < abbrev->num_attrs; ++i){
        info_ptr = read_attribute (&die->attrs[i], &abbrev->attrs[i], info_ptr, cu);
        //        printf("%s\t %s\n", dwarf_attr_name(die->attrs[i].name), dwarf_form_name(die->attrs[i].form));

        //   /* APPLE LOCAL begin dwarf repository  */
        //   if (die->attrs[i].name == DW_AT_APPLE_repository_file)
        // repository_name = DW_STRING (&die->attrs[i]);
        if (die->attrs[i].name == DW_AT_comp_dir){
            comp_dir = die->attrs[i].u.str;
            //printf("%s\n", comp_dir);
            cu->comp_dir = comp_dir;
        }
    }

    *diep = die;
    *has_children = abbrev->has_children;
    return info_ptr;
}


/* Read a whole compilation unit into a linked list of dies.  */

static struct die_info * read_comp_unit (char *info_ptr, struct dwarf2_cu *cu)
{
    return read_die_and_children (info_ptr, cu, &info_ptr, NULL);
}

/* Read a single die and all its descendents.  Set the die's sibling
   field to NULL; set other fields in the die correctly, and set all
   of the descendents' fields correctly.  Set *NEW_INFO_PTR to the
   location of the info_ptr after reading all of those dies.  PARENT
   is the parent of the die in question.  */

static struct die_info * read_die_and_children (char *info_ptr, struct dwarf2_cu *cu, char **new_info_ptr, struct die_info *parent)
{
    struct die_info *die;
    char *cur_ptr;
    int has_children;

    cur_ptr = read_full_die (&die, info_ptr, cu, &has_children);
    //store_in_ref_table (die->offset, die, cu);

    if (has_children)
    {
        die->child = read_die_and_siblings (cur_ptr, cu, new_info_ptr, die);
    }
    else
    {
        die->child = NULL;
        *new_info_ptr = cur_ptr;
    }

    die->sibling = NULL;
    die->parent = parent;
    return die;
}

/* Read a die, all of its descendents, and all of its siblings; set
   all of the fields of all of the dies correctly.  Arguments are as
   in read_die_and_children.  */

static struct die_info * read_die_and_siblings (char *info_ptr, struct dwarf2_cu *cu, char **new_info_ptr, struct die_info *parent)
{
    struct die_info *first_die, *last_sibling;
    char *cur_ptr;

    cur_ptr = info_ptr;
    first_die = last_sibling = NULL;

    while (1)
    {
        struct die_info *die
            = read_die_and_children (cur_ptr, cu, &cur_ptr, parent);

        if (!first_die)
        {
            first_die = die;
        }
        else
        {
            last_sibling->sibling = die;
        }

        if (die->tag == 0)
        {
            *new_info_ptr = cur_ptr;
            return first_die;
        }
        else
        {
            last_sibling = die;
        }
    }
}

/* Load the DIEs associated with PST and PER_CU into memory.  */
/* APPLE LOCAL debug map: Accept an optional 2nd parameter ADDR_MAP */

//static struct dwarf2_cu * load_full_comp_unit (struct dwarf2_per_objfile *dwarf2_per_objfile, int i)
static void load_full_comp_unit (struct dwarf2_per_objfile *dwarf2_per_objfile, int i)
{
    struct dwarf2_per_cu_data *per_cu = dwarf2_per_objfile->all_comp_units[i];
    //struct partial_symtab *pst = per_cu->psymtab;
    //bfd *abfd = pst->objfile->obfd;
    struct dwarf2_cu *cu;
    unsigned long offset;
    char *info_ptr;
    //`struct cleanup *back_to, *free_cu_cleanup;
    //TODO ADD TARGET LANGUAGE
    //struct attribute *attr;
    /* APPLE LOCAL avoid unused var warning. */
    /* CORE_ADDR baseaddr; */

    /* Set local variables from the partial symbol table info.  */
    offset = per_cu->offset;
    //printf("offset: %08lx\n", offset);

    info_ptr = dwarf2_per_objfile->info_buffer + offset;

    cu = malloc (sizeof (struct dwarf2_cu));
    memset (cu, 0, sizeof (struct dwarf2_cu));
    cu->dwarf2_per_objfile = dwarf2_per_objfile;

    /* If an error occurs while loading, release our storage.  */
    //free_cu_cleanup = make_cleanup (free_one_comp_unit, cu);

    //cu->objfile = pst->objfile;

    /* read in the comp_unit header  */
    info_ptr = read_comp_unit_head (&cu->header, info_ptr);

    /* Read the abbrevs for this compilation unit  */
    //dwarf2_read_abbrevs (cu);
    cu->dwarf2_abbrevs = dwarf2_per_objfile->dwarf2_abbrevs;


    //  back_to = make_cleanup (dwarf2_free_abbrev_table, cu);

    cu->header.offset = offset;

    /* APPLE LOCAL debug map */
    //cu->addr_map = addr_map;

    cu->per_cu = per_cu;
    per_cu->cu = cu;

    /* We use this obstack for block values in dwarf_alloc_block.  */
    //obstack_init (&cu->comp_unit_obstack);

    cu->dies = read_comp_unit (info_ptr, cu);

    /* We try not to read any attributes in this function, because not
       all objfiles needed for references have been loaded yet, and symbol
       table processing isn't initialized.  But we have to set the CU language,
       or we won't be able to build types correctly.  */
    //attr = dwarf2_attr (cu->dies, DW_AT_language, cu);
    //if (attr)
    //  set_cu_language (attr->u.unsnd., cu);
    //else
    //  set_cu_language (language_minimal, cu);

    //do_cleanups (back_to);

    /* We've successfully allocated this compilation unit.  Let our caller
       clean it up when finished with it.  */
    //discard_cleanups (free_cu_cleanup);

    //return cu;
}

/* Generate full symbol information for PST and CU, whose DIEs have
   already been loaded into memory.  */


/* Create a list of all compilation units in OBJFILE.  We do this only
   if an inter-comp-unit reference is found; presumably if there is one,
   there will be many, and one will occur early in the .debug_info section.
   So there's no point in building this list incrementally.  */

static void create_all_comp_units(struct dwarf2_per_objfile *dwarf2_per_objfile)
{
    int n_allocated;
    int n_comp_units;
    struct dwarf2_per_cu_data **all_comp_units;
    char *info_ptr = dwarf2_per_objfile->info_buffer;

    n_comp_units = 0;
    n_allocated = 10;
    all_comp_units = malloc (n_allocated * sizeof (struct dwarf2_per_cu_data *));

    while (info_ptr < dwarf2_per_objfile->info_buffer + dwarf2_per_objfile->info_size)
    {
        struct comp_unit_head cu_header;
        struct dwarf2_per_cu_data *this_cu;
        unsigned long offset;
        int bytes_read;

        offset = info_ptr - dwarf2_per_objfile->info_buffer;

        /* Read just enough information to find out where the next
           compilation unit is.  */
        cu_header.initial_length_size = 0;
        cu_header.length = read_initial_length_of_comp_unit(info_ptr, &cu_header, &bytes_read);

        /* Save the compilation unit for later lookup.  */
        this_cu = malloc(sizeof (struct dwarf2_per_cu_data));
        memset (this_cu, 0, sizeof (*this_cu));
        this_cu->offset = offset;
        this_cu->length = cu_header.length + cu_header.initial_length_size;

        if (n_comp_units == n_allocated)
        {
            n_allocated *= 2;
            all_comp_units = realloc (all_comp_units, n_allocated * sizeof (struct dwarf2_per_cu_data *));
        }
        all_comp_units[n_comp_units++] = this_cu;

        info_ptr = info_ptr + this_cu->length;
        //printf("%p %p\n",info_ptr , dwarf2_per_objfile->info_buffer + dwarf2_per_objfile->info_size);
    }

    dwarf2_per_objfile->all_comp_units = malloc (n_comp_units * sizeof (struct dwarf2_per_cu_data *));
    memcpy (dwarf2_per_objfile->all_comp_units, all_comp_units,
            n_comp_units * sizeof (struct dwarf2_per_cu_data *));
    free (all_comp_units);
    dwarf2_per_objfile->n_comp_units = n_comp_units;
}

static int parse_dwarf_abbrev(struct dwarf2_per_objfile *dwarf2_per_objfile){
    //allocate space form the abbrev hash
    struct abbrev_info **dwarf2_abbrevs= malloc(sizeof(struct abbrev_info *) * ABBREV_HASH_SIZE);
    memset(dwarf2_abbrevs, 0, sizeof(struct abbrev_info *) * ABBREV_HASH_SIZE);

    dwarf2_per_objfile->dwarf2_abbrevs = dwarf2_abbrevs;
    char * info_ptr = dwarf2_per_objfile->abbrev_buffer;
    int size = dwarf2_per_objfile->abbrev_size;
    char* endof_abbrev_pos = info_ptr + size;
    int i = 0;
    while(info_ptr < endof_abbrev_pos && *info_ptr != '\0'){
        unsigned int bytes_read = 0;
        unsigned long long abbrev_code = read_unsigned_leb128(info_ptr, &bytes_read);
        //printf("%llu %u\n", abbrev_code, bytes_read);
        info_ptr += bytes_read;
        unsigned long long entry_code = read_unsigned_leb128(info_ptr, &bytes_read);
        //printf("%llu\n", entry_code);
        info_ptr += bytes_read;
        unsigned char has_children = (unsigned char)*info_ptr;
        //printf("%u\n", has_children);
        info_ptr ++;

        unsigned int num_attr_spec_pair = 0;
        num_attr_spec_pair = get_num_attr_spec_pair(info_ptr);

        struct abbrev_info *ai = malloc(sizeof(struct abbrev_info));
        memset(ai, '\0', sizeof(struct abbrev_info));
        ai->number = (unsigned int)abbrev_code;
        ai->tag = (unsigned int)entry_code;
        ai->has_children = (unsigned short)has_children;
        ai->num_attrs = (unsigned short)num_attr_spec_pair;
        ai->next = NULL;
        //printf("%s\t", dwarf_tag_name(ai->tag));
        //printf("num_attr_spec_pair: %d\n", num_attr_spec_pair);
        if (num_attr_spec_pair != 0){
            struct attr_abbrev *attrs = malloc(num_attr_spec_pair * sizeof(struct attr_abbrev));
            memset(attrs, '\0', num_attr_spec_pair * sizeof(struct attr_abbrev));
            unsigned int attr_name_code = (unsigned int)read_unsigned_leb128(info_ptr, &bytes_read);
            info_ptr += bytes_read;
            unsigned int attr_form_code = (unsigned int)read_unsigned_leb128(info_ptr, &bytes_read);
            info_ptr += bytes_read;
            int j = 0;
            while(attr_name_code != 0 || attr_form_code != 0){
                attrs[j].name = attr_name_code;
                attrs[j].form = attr_form_code;
                debug("%s %s\n", dwarf_attr_name(attrs[j].name), dwarf_form_name(attrs[j].form));
                attr_name_code = (unsigned int)read_unsigned_leb128(info_ptr ,&bytes_read);
                info_ptr += bytes_read;
                attr_form_code = (unsigned int)read_unsigned_leb128(info_ptr ,&bytes_read);
                info_ptr += bytes_read;
                j++;
            }
            ai->attrs = attrs;
        }else{
            info_ptr += 2;
            ai->attrs = NULL;
        }
        struct abbrev_info **temp_abbr = &dwarf2_abbrevs[ai->number % ABBREV_HASH_SIZE];
        while(*temp_abbr){
            temp_abbr = &(*temp_abbr)->next;
        }
        *temp_abbr = ai;
        i++;
    }
    debug("parse dwarf2_per_objfile finished.");
    return 0;
}

static int parse_dwarf_info(struct dwarf2_per_objfile *dwarf2_per_objfile){
    create_all_comp_units(dwarf2_per_objfile);
    int i = 0;
//    struct dwarf2_cu *temp = NULL;
    for (i = 0; i< dwarf2_per_objfile->n_comp_units; i++){
        load_full_comp_unit(dwarf2_per_objfile, i);
    }
    return 0;
}

static unsigned int get_num_arange_descriptor(char *aranges_ptr, struct arange *arange, int *flag){
    int bytes_read = 0;
    unsigned int num_of_ards = 0;
    CORE_ADDR beginning_addr = 0;
    unsigned int length = 0;
    while(1){
        beginning_addr = read_address_of_arange(aranges_ptr, arange, &bytes_read);
        aranges_ptr += bytes_read;

        switch (arange->aranges_header.addr_size){
            case 2:
                length = read_signed_16(aranges_ptr);
                aranges_ptr += 2;
                break;
            case 4:
                length = read_signed_32(aranges_ptr);
                aranges_ptr += 4;
                break;
            case 8:
                length = read_signed_64(aranges_ptr);
                aranges_ptr += 8;
                break;
            default:
                fprintf(stderr, "read address length offset: bad switch, signed\n");
                PyErr_Format(ATOSLError, "read address length offset: bad switch, signed");
                *flag = -1;
                return 0;
        }
        if(beginning_addr == 0 && length == 0){
            break;
        }
        num_of_ards ++;
    }
    return num_of_ards;
}

static int parse_dwarf_aranges(struct dwarf2_per_objfile *dwarf2_per_objfile)
{

    int n_allocated;
    int n_aranges;
    struct arange **all_aranges;
    //struct dwarf2_per_cu_data **all_comp_units;
    //char *info_ptr = dwarf2_per_objfile->info_buffer;
    char *aranges_ptr = dwarf2_per_objfile->aranges_buffer;

    n_aranges = 0;
    n_allocated = 10;
    //all_comp_units = malloc (n_allocated * sizeof (struct dwarf2_per_cu_data *));
    all_aranges = malloc(n_allocated * sizeof(struct arange *));

    while (aranges_ptr < dwarf2_per_objfile->aranges_buffer+ dwarf2_per_objfile->aranges_size)
    {
        //struct aranges_header aranges_header;
        //    struct dwarf2_per_cu_data *this_cu;
        //unsigned long offset;
        int bytes_read;

        //offset = aranges_ptr - dwarf2_per_objfile->aranges_buffer;

        //    /* Read just enough information to find out where the next
        //       compilation unit is.  */
        //    cu_header.initial_length_size = 0;
        //    cu_header.length = read_initial_length (info_ptr, &cu_header, &bytes_read);

        struct arange *arange = malloc(sizeof(struct arange));
        memset(arange, 0, sizeof(struct arange));

        arange->aranges_header.length = read_initial_length_of_aranges(aranges_ptr, &arange->aranges_header, &bytes_read);
        aranges_ptr += 4;

        arange->aranges_header.version = read_2_bytes(aranges_ptr);
        aranges_ptr += 2;

        arange->aranges_header.info_offset = read_4_bytes(aranges_ptr);
        aranges_ptr += 4;

        arange->aranges_header.addr_size = read_1_byte(aranges_ptr);
        aranges_ptr += 1;

        arange->aranges_header.seg_size = read_1_byte(aranges_ptr);
        aranges_ptr += 1;

        //FIXME 4 additional null bytes
        unsigned int zeros = read_4_bytes(aranges_ptr);
        assert(zeros == 0);
        if(zeros != 0){
            fprintf(stderr, "should be 4 additional null bytes.");
            PyErr_Format(ATOSLError, "should be 4 additional null bytes");
            return -1;
        }
        aranges_ptr += 4;
        int flag = 0;
        unsigned int num_of_ards = get_num_arange_descriptor(aranges_ptr, arange, &flag);
        if (flag == -1){
            return -1;
        }
        arange->num_of_ards = num_of_ards;
        //printf("num_of_ards: %d\n", num_of_ards);

        arange->address_range_descriptors = malloc(num_of_ards * sizeof(struct address_range_descriptor));
        assert(arange->address_range_descriptors != NULL);
        if(arange->address_range_descriptors == NULL){
            PyErr_NoMemory();
            return -1;
        }
        memset(arange->address_range_descriptors, 0, num_of_ards * sizeof(struct address_range_descriptor));
        //struct address_range_descriptor ard
        int i = 0;
        int aranges_header_addr_size = 0;
        for(i = 0; i < num_of_ards; i++){
            arange->address_range_descriptors[i].beginning_addr = read_address_of_arange(aranges_ptr, arange, &bytes_read);
            aranges_ptr += bytes_read;
            //save the address size for skipping the right number of bytes from the end of address_range_descriptors
            aranges_header_addr_size = bytes_read;
            switch (arange->aranges_header.addr_size){
                case 2:
                    arange->address_range_descriptors[i].length = read_signed_16(aranges_ptr);
                    aranges_ptr += 2;
                    break;
                case 4:
                    arange->address_range_descriptors[i].length = read_signed_32(aranges_ptr);
                    aranges_ptr += 4;
                    break;
                case 8:
                    arange->address_range_descriptors[i].length = read_signed_64(aranges_ptr);
                    aranges_ptr += 8;
                    break;
                default:
                    fprintf(stderr, "read address length offset: bad switch, signed\n");
                    PyErr_Format(ATOSLError, "read address length offset: bad switch, signed\n");
                    return -1;
            }

            //arange->address_range_descriptors[i].length = read_4_bytes(aranges_ptr);

            //    printf("beginning_addr: 0X%X\t", arange->address_range_descriptors[i].beginning_addr);
            //    printf("length: 0X%X\n", arange->address_range_descriptors[i].length);
        }
        //skip ending zeros
        aranges_ptr += (2 * aranges_header_addr_size);
        //    /* Save the compilation unit for later lookup.  */
        //    this_cu = malloc(sizeof (struct dwarf2_per_cu_data));
        //    memset (this_cu, 0, sizeof (*this_cu));
        //    this_cu->offset = offset;
        //    this_cu->length = cu_header.length + cu_header.initial_length_size;

        if (n_aranges == n_allocated)
        {
            n_allocated *= 2;
            all_aranges = realloc (all_aranges, n_allocated * sizeof (struct arange*));
        }
        all_aranges[n_aranges++] = arange;

        //    info_ptr = info_ptr + this_cu->length;
        //    //printf("%p %p\n",info_ptr , dwarf2_per_objfile->info_buffer + dwarf2_per_objfile->info_size);
    }

    //dwarf2_per_objfile->all_comp_units = malloc (n_comp_units * sizeof (struct dwarf2_per_cu_data *));
    dwarf2_per_objfile->all_aranges = malloc (n_aranges * sizeof (struct arange*));
    memcpy (dwarf2_per_objfile->all_aranges, all_aranges, n_aranges * sizeof (struct arange *));
    free (all_aranges);
    dwarf2_per_objfile->n_aranges = n_aranges;
    return 0;
}

static int is_target_subprogram(struct die_info *die, struct address_range_descriptor *target_ard, CORE_ADDR integer_address){
    //FIXME May not need target_ard
    int flag = 0;
    unsigned int i = 0;
    for(i = 0; i< die->num_attrs; i++){
        //if(die->attrs[i].name == DW_AT_low_pc && die->attrs[i].u.addr >= target_ard->beginning_addr && integer_address ){
        //    flag++;
        //}

        //if(die->attrs[i].name == DW_AT_high_pc && die->attrs[i].u.addr <= (target_ard->beginning_addr + target_ard->length)){
        //    flag++;
        //}
        if(die->attrs[i].name == DW_AT_low_pc && die->attrs[i].u.addr <= integer_address ){
            flag++;
        }

        if(die->attrs[i].name == DW_AT_high_pc && die->attrs[i].u.addr > integer_address){
            flag++;
        }


        if(flag == 2){
            return 1;
        }
    }

    return -1;

}

static struct die_info *find_target_subprogram(struct die_info *die, struct address_range_descriptor *target_ard, CORE_ADDR integer_address){
    if(die->tag == DW_TAG_subprogram){
        if(is_target_subprogram(die, target_ard, integer_address) == 1){
            return die;
        }
    }

    if(die->sibling != NULL){
        return find_target_subprogram(die->sibling, target_ard, integer_address);
    }

    if(die->child != NULL){
        return find_target_subprogram(die->child, target_ard, integer_address);
    }
    return NULL;

}
static char *get_name_attribute(struct die_info *die){
    unsigned int i = 0;
    for(i = 0; i < die->num_attrs; i++){
        if (die->attrs[i].name == DW_AT_name){
            return die->attrs[i].u.str;
        }
    }
    return NULL;
}

static unsigned int get_stmt_list_attribute(struct die_info *die, char *flag){
    unsigned int i = 0;
    for(i = 0; i < die->num_attrs; i++){
        if (die->attrs[i].name == DW_AT_stmt_list){
            *flag = 0;
            return die->attrs[i].u.addr;
        }
    }
    *flag = 1;
    return 0;
}

static int get_lineno_for_address(struct subfile *subfile, CORE_ADDR address){
    struct linetable_entry *current_entry;
    struct linetable_entry *next_entry;
    int i = 0;
    for(i = 0; i < subfile->line_vector->nitems; i ++){
        current_entry = subfile->line_vector->item + i;
        next_entry = subfile->line_vector->item + i + 1;
        if(address >= current_entry->pc && address < next_entry->pc ){
            return current_entry ->line;
        }
    }
    return 0;
}

void free_sub_file(struct subfile *subfile){
    free(subfile->line_vector);
    free(subfile);
}

void print_thin_macho_aranges(struct thin_macho *thin_macho){
    struct dwarf2_per_objfile* dwarf2_per_objfile = thin_macho->dwarf2_per_objfile;
    unsigned int num = dwarf2_per_objfile->n_aranges;
    struct arange **all_aranges = dwarf2_per_objfile->all_aranges;

    unsigned int i = 0, j = 0;
    for(i = 0; i< num; i++){
        struct arange *arange = all_aranges[i];
        printf("Address Range Header: length = 0x%08x  version = 0x%04x  cu_offset = 0x%08x  addr_size = 0x%02x  seg_size = 0x%02x\n", arange->aranges_header.length, arange->aranges_header.version, arange->aranges_header.info_offset, arange->aranges_header.addr_size, arange->aranges_header.seg_size);
        for (j = 0; j < arange->num_of_ards; j++){
            printf("0x%016llx + 0x%016llx = 0x%016llx\n", arange->address_range_descriptors[j].beginning_addr, arange->address_range_descriptors[j].length, arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length);
            //printf("0x%08x + 0x%08x = 0x%08x\n", arange->address_range_descriptors[j].beginning_addr, arange->address_range_descriptors[j].length, arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length);
        }
    }
}

static void select_symbol_by_address(struct nlist *symbols, uint32_t nsyms, CORE_ADDR target, struct nlist **found_symbol, int *offset){
    uint32_t i= 0;
    for (i = 0; i < nsyms; ++i) {
        /*  The symbol must be defined in a section, and must not be a debugging entry. */
        if ((symbols[i].n_type & N_TYPE) != N_SECT || ((symbols[i].n_type & N_STAB) != 0)) {
            continue;
        }
        /*  If we haven't already found a symbol and the address we want is
         *  greater than the symbol's value, save this symbol. */
        if (!*found_symbol && symbols[i].n_value <= target) {
            *found_symbol = &symbols[i];
            *offset = target - symbols[i].n_value;
            /*  If we have found a symbol already, but if the address we want is
             *  greater than the current symbol's value and the current symbol is later
             *  than the last one found, the current one is a closer match. */
        } else if (*found_symbol && symbols[i].n_value <= target && ((*found_symbol)->n_value < symbols[i].n_value)) {
            *found_symbol = &symbols[i];
            *offset = target - symbols[i].n_value;
        }
    }
}

int lookup_by_address_in_symtable(struct thin_macho *tm, CORE_ADDR integer_address){
    int offset = -1;
    struct nlist *found_symbol = NULL;
    //struct nlist *global_syms = (struct nlist *)(tm->all_symbols + tm->symbolInformation.firstGlobalSymbol * sizeof(struct nlist)),
    //             *local_syms = (struct nlist *)(tm->all_symbols + tm->symbolInformation.firstLocalSymbol * sizeof(struct nlist));
    //select_symbol_by_address(global_syms, tm->symbolInformation.numGlobalSymbols, integer_address, &found_symbol, &offset);
    //select_symbol_by_address(local_syms, tm->symbolInformation.numLocalSymbols, integer_address, &found_symbol, &offset);
    select_symbol_by_address(tm->all_symbols, tm->nsyms, integer_address, &found_symbol, &offset);
    if(found_symbol){
        printf("%s (in %s) + %d\n", tm->strings + found_symbol->n_un.n_strx, project_name, offset);
        return 0;
    }else{
        return -1;
    }
}

int lookup_by_address_in_dwarf(struct thin_macho *thin_macho, CORE_ADDR integer_address){
    CORE_ADDR address = (CORE_ADDR)integer_address;
    struct dwarf2_per_objfile* dwarf2_per_objfile = thin_macho->dwarf2_per_objfile;
    unsigned int num = dwarf2_per_objfile->n_aranges;
    struct arange **all_aranges = dwarf2_per_objfile->all_aranges;
    struct arange *target_arange = NULL;
    struct address_range_descriptor *target_ard = NULL;
    unsigned int i = 0, j = 0;
    for(i = 0; i< num; i++){
        struct arange *arange = all_aranges[i];
        for(j = 0; j < arange->num_of_ards; j++){
            //debug
            CORE_ADDR beginning_addr = arange->address_range_descriptors[j].beginning_addr;
            CORE_ADDR ending_addr = arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length;
            //printf("0x%016llx + 0x%016llx = 0x%016llx\n", arange->address_range_descriptors[j].beginning_addr, arange->address_range_descriptors[j].length, arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length);
            if (address >= beginning_addr && address < ending_addr){
                target_arange = arange;
                target_ard = &arange->address_range_descriptors[j];
                break;
            }
        }
    }

    if(target_arange == NULL){
        debug("target_arange is NULL\n\n");
        return -1;
    }

    if(target_ard == NULL){
        debug("target_ard is NULL\n\n");
        return -1;
    }

    //find the target compilation unit
    struct dwarf2_per_cu_data *target_dwarf2_per_cu_data= NULL;
    for (i = 0; i < dwarf2_per_objfile->n_comp_units; i++){
        if (dwarf2_per_objfile->all_comp_units[i]->offset == target_arange->aranges_header.info_offset){
            debug("offset :0x%08lx\tlength: 0x%08lx\n", dwarf2_per_objfile->all_comp_units[i]->offset, dwarf2_per_objfile->all_comp_units[i]->length);
            target_dwarf2_per_cu_data = dwarf2_per_objfile->all_comp_units[i];
            break;
        }
    }
    struct dwarf2_cu *target_cu = target_dwarf2_per_cu_data->cu;

    struct die_info *target_die = find_target_subprogram(target_cu->dies, target_ard, integer_address);
    if(target_die == NULL){
        debug("Can not find target subprogram.\n");
        return -1;
    }
    char *target_program_full_name = get_name_attribute(target_cu->dies);
    char *target_program_name = strrchr(target_program_full_name, '/');
    if(target_program_name == NULL){
        target_program_name = target_program_full_name;
    }else{
        target_program_name = target_program_name +1;
    }
    char *target_subprogram_name = get_name_attribute(target_die);

    //Lookup address infomation
    char flag = 0;
    unsigned int offset = get_stmt_list_attribute(target_cu->dies, &flag);
    if(flag == 1){
        fprintf(stderr, "do not have stmt_list attribute\n");
        PyErr_Format(ATOSLError, "do not have stmt_list attribute");
        return -1;
    }else{
        debug("offset: 0x%08x\n", offset);
    }
    struct line_header *lh = dwarf_decode_line_header (offset, target_cu);
    if(lh == NULL){
        return -1;
    }
    struct subfile * current_subfile = dwarf_decode_lines (lh, NULL, target_cu);
    //print_line_vector(current_subfile);
    int lineno = get_lineno_for_address(current_subfile, address);
    debug("lineno: %d\n",lineno);
    printf("%s (in %s) (%s:%d)\n", target_subprogram_name, project_name, target_program_name, lineno);
    free_sub_file(current_subfile);
    free_line_header(lh);

    return 0;
}

void print_symbols(struct nlist *all_symbols, uint32_t numofsyms, char *strings, uint32_t strsize){
    int i = 0;
    for (i = 0; i < numofsyms; i++){
        printf("%08x: %s\n", all_symbols[i].n_value, strings + all_symbols[i].n_un.n_strx);
    }
}

void parse_lc_symtab(char *macho_str, struct symtab_command *command, struct thin_macho*tm){
    //FIXME BIGENDIAN?
    uint32_t symoff = command->symoff;
    uint32_t nsyms = command->nsyms;
    uint32_t stroff = command->stroff;
    uint32_t strsize = command->strsize;

    //struct nlist_64 *all_symbols64;


    tm->all_symbols = malloc(command->nsyms * sizeof(struct nlist));
    memset(tm->all_symbols, '\0', command->nsyms * sizeof(struct nlist));
    memcpy(tm->all_symbols, macho_str + symoff, command->nsyms * sizeof(struct nlist));
    tm->nsyms = nsyms;

    tm->strings = macho_str + stroff;
    tm->strsize = strsize;
    //print_symbols(tm->all_symbols, tm->nsyms, tm->strings, tm->strsize);
    //FIXME for 64
    //all_symbols64 = (struct nlist_64 *)(ofile->object_addr +st->symoff);
}

int parse_dwarf2_per_objfile(struct dwarf2_per_objfile *dwarf2_per_objfile){
    int result = -1;
    //TODO
    debug("about to parse_dwarf_abbrev");
    result = parse_dwarf_abbrev(dwarf2_per_objfile);
    if (result == -1){
        return -1;
    }
    debug("about to parse_dwarf_info");
    result = parse_dwarf_info(dwarf2_per_objfile);
    if (result == -1){
        return -1;
    }
    debug("about to parse_dwarf_aranges");
    result = parse_dwarf_aranges(dwarf2_per_objfile);
    if (result == -1){
        return -1;
    }
    return 0;
}

int parse_load_command(char *macho_str, long *offset, struct load_command *lc, struct thin_macho*tm){
    int load_command_result = -1;
    switch (lc->cmd){
        case LC_UUID:
            load_command_result = process_lc_uuid(macho_str, offset, tm);
            break;
        case LC_SEGMENT:
            load_command_result = process_lc_segment(macho_str, offset, tm);
            break;
        case LC_SEGMENT_64:
            load_command_result = process_lc_segment_64(macho_str, offset, tm);
            break;
        case LC_SYMTAB:
            load_command_result = process_lc_symtab(macho_str, offset, tm);
            break;
        case LC_DYSYMTAB:
            load_command_result = process_lc_dysymtab(macho_str, offset, tm);
            break;
        case LC_THREAD:
            load_command_result = process_lc_thread(macho_str, offset);
            break;
        case LC_UNIXTHREAD:
            load_command_result = process_lc_unixthread(macho_str, offset);
            break;
        case LC_LOAD_DYLIB:
            load_command_result = process_lc_load_dylib(macho_str, offset);
            break;
        case LC_ID_DYLIB:
            load_command_result = process_lc_id_dylib(macho_str, offset);
            break;
        case LC_PREBOUND_DYLIB:
            load_command_result = process_lc_prebound_dylib(macho_str, offset);
            break;
        case LC_LOAD_DYLINKER:
            load_command_result = process_lc_load_dylinker(macho_str, offset);
            break;
        case LC_ID_DYLINKER:
            load_command_result = process_lc_id_dylinker(macho_str, offset);
            break;
        case LC_ROUTINES:
            load_command_result = process_lc_routines(macho_str, offset);
            break;
        case LC_ROUTINES_64:
            load_command_result = process_lc_routines_64(macho_str, offset);
            break;
        case LC_TWOLEVEL_HINTS:
            load_command_result = process_lc_twolevel_hints(macho_str, offset);
            break;
        case LC_SUB_FRAMEWORK:
            load_command_result = process_lc_sub_framework(macho_str, offset);
            break;
        case LC_SUB_UMBRELLA:
            load_command_result = process_lc_sub_umbrella(macho_str, offset);
            break;
        case LC_SUB_LIBRARY:
            load_command_result = process_lc_sub_library(macho_str, offset);
            break;
        case LC_SUB_CLIENT:
            load_command_result = process_lc_sub_client(macho_str, offset);
            break;
        case LC_DATA_IN_CODE:
            load_command_result = process_lc_data_in_code(macho_str, offset);
            break;
        case LC_FUNCTION_STARTS:
            load_command_result = process_lc_function_starts(macho_str, offset);
            break;
        case LC_DYLD_INFO_ONLY:
            load_command_result = process_lc_dyld_info_only(macho_str, offset);
            break;
        case LC_DYLD_INFO:
            load_command_result = process_lc_dyld_info_only(macho_str, offset);
            break;
        case LC_VERSION_MIN_IPHONEOS:
            load_command_result = process_lc_version_min_iphoneos(macho_str, offset);
            break;
        case LC_VERSION_MIN_MACOSX:
            load_command_result = process_lc_version_min_macosx(macho_str, offset);
            break;
        case LC_SOURCE_VERSION:
            load_command_result = process_lc_source_version(macho_str, offset);
            break;
        case LC_REEXPORT_DYLIB:
            load_command_result = process_lc_reexport_dylib(macho_str, offset);
            break;
        case LC_SYMSEG:
            load_command_result = process_lc_symseg(macho_str, offset);
            break;
        case LC_LOADFVMLIB:
            load_command_result = process_lc_loadfvmlib(macho_str, offset);
            break;
        case LC_IDFVMLIB:
            load_command_result = process_lc_idfvmlib(macho_str, offset);
            break;
        case LC_IDENT:
            load_command_result = process_lc_ident(macho_str, offset);
            break;
        case LC_FVMFILE:
            load_command_result = process_lc_fvmfile(macho_str, offset);
            break;
        case LC_PREBIND_CKSUM:
            load_command_result = process_lc_prebind_cksum(macho_str, offset);
            break;
        case LC_LOAD_WEAK_DYLIB:
            load_command_result = process_lc_load_weak_dylib(macho_str, offset);
            break;
        case LC_RPATH:
            load_command_result = process_lc_rpath(macho_str, offset);
            break;
        case LC_CODE_SIGNATURE:
            load_command_result = process_lc_code_signature(macho_str, offset);
            break;
        case LC_SEGMENT_SPLIT_INFO:
            load_command_result = process_lc_segment_split_info(macho_str, offset);
            break;
        case LC_ENCRYPTION_INFO:
            load_command_result = process_lc_encryption_info(macho_str, offset);
            break;
        case LC_DYLD_ENVIRONMENT:
            load_command_result = process_lc_dyld_environment(macho_str, offset);
            break;
        case LC_MAIN:
            load_command_result = process_lc_main(macho_str, offset);
            break;
        case LC_DYLIB_CODE_SIGN_DRS:
            load_command_result = process_lc_dylib_code_sign_drs(macho_str, offset);
            break;
        case LC_PREPAGE:
            //printf("known load commmand type LC_PREPAGE, but ignoring...\n");
            load_command_result = process_lc_command(macho_str, offset);
            break;
        case LC_LAZY_LOAD_DYLIB:
            //printf("known load commmand type LC_LAZY_LOAD_DYLIB, but ignoring...\n");
            load_command_result = process_lc_command(macho_str, offset);
            break;
        case LC_LOAD_UPWARD_DYLIB:
            //printf("known load commmand type LC_LOAD_UPWARD_DYLIB, but ignoring...\n");
            load_command_result = process_lc_command(macho_str, offset);
            break;
        default:
            fprintf(stderr, "unknown load commmand type, ignoring...\n");
    }
    return load_command_result;
}

void print_uuid(struct uuid_command *command){
    int numofbytes = sizeof(command->uuid)/sizeof(*command->uuid);
    printf("uuid: ");
    int i = 0;
    for (i = 0; i < numofbytes; i++){
        printf("%02X", command->uuid[i]);
    }
    printf("\n");
}

int process_lc_source_version(char *macho_str, long *offset){
    struct source_version_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct source_version_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_version_min_macosx(char *macho_str, long *offset){
    struct version_min_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct version_min_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_version_min_iphoneos(char *macho_str, long *offset){
    struct version_min_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct version_min_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_dyld_info_only(char *macho_str, long *offset){
    struct dyld_info_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dyld_info_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_dyld_info(char *macho_str, long *offset){
    struct dyld_info_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dyld_info_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_data_in_code(char *macho_str, long *offset){
    struct lc_data_in_code command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct lc_data_in_code));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_function_starts(char *macho_str, long *offset){
    struct lc_function_starts command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct lc_function_starts));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_uuid(char *macho_str, long *offset, struct thin_macho*tm){
    struct uuid_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct uuid_command));
    //print_uuid(&command);
    *offset += command.cmdsize;
    int i = 0;
    while(i < 16){
        tm->uuid[i] = command.uuid[i];
        i++;
    }
    return 0;
}

int process_lc_segment(char *macho_str, long *offset, struct thin_macho*tm){
    struct segment_command command = {0};

    memcpy(&command, macho_str + *offset, sizeof(struct segment_command));
    *offset += sizeof(struct segment_command);
    //if(strcmp(command.segname, "__TEXT") == 0){
    //    doi.text_vmaddr = command.vmaddr;
    //}
    if(strcmp(command.segname, "__DWARF") == 0){
        tm->dwarf2_per_objfile = parse_dwarf_segment(macho_str, *offset, &command);
        if (tm->dwarf2_per_objfile == NULL){
            return -1;
        }
    }
    //in case there are sections, we need to seek the file point to the next load command
    *offset += command.cmdsize - sizeof(struct segment_command);
    return 0;
}

int process_lc_segment_64(char *macho_str, long *offset, struct thin_macho*tm){
    struct segment_command_64 command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct segment_command_64));
    *offset += sizeof(struct segment_command_64);
    //if(strcmp(command.segname, "__TEXT") == 0){
    //    doi.text_vmaddr_64 = command.vmaddr;
    //}
    if(strcmp(command.segname, "__DWARF") == 0){
        tm->dwarf2_per_objfile = parse_dwarf_segment_64(macho_str, *offset, &command);
        if (tm->dwarf2_per_objfile == NULL){
            return -1;
        }
    }
    *offset += command.cmdsize - sizeof(struct segment_command_64);

    return 0;
}

int process_lc_sub_client(char *macho_str, long *offset){
    struct sub_client_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct sub_client_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_sub_library(char *macho_str, long *offset){
    struct sub_library_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct sub_library_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_sub_umbrella(char *macho_str, long *offset){
    struct sub_umbrella_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct sub_umbrella_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_sub_framework(char *macho_str, long *offset){
    struct sub_framework_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct sub_framework_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_twolevel_hints(char *macho_str, long *offset){
    struct twolevel_hints_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct twolevel_hints_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_routines_64(char *macho_str, long *offset){
    struct routines_command_64 command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct routines_command_64));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_routines(char *macho_str, long *offset){
    struct routines_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct routines_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_id_dylinker(char *macho_str, long *offset){
    struct dylinker_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dylinker_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_load_dylinker(char *macho_str, long *offset){
    struct dylinker_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dylinker_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_prebound_dylib(char *macho_str, long *offset){
    struct prebound_dylib_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct prebound_dylib_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_reexport_dylib(char *macho_str, long *offset){
    struct prebound_dylib_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct prebound_dylib_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_id_dylib(char *macho_str, long *offset){
    struct dylib_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dylib_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_load_dylib(char *macho_str, long *offset){
    struct dylib_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dylib_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_thread(char *macho_str, long *offset){
    struct thread_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct thread_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_unixthread(char *macho_str, long *offset){
    struct thread_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct thread_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_dysymtab(char *macho_str, long *offset, struct thin_macho*tm){
    struct dysymtab_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dysymtab_command));
    tm->symbolInformation.firstGlobalSymbol = command.iextdefsym;
    tm->symbolInformation.numGlobalSymbols = command.nextdefsym;
    tm->symbolInformation.firstLocalSymbol = command.ilocalsym;
    tm->symbolInformation.numLocalSymbols = command.nlocalsym;
    *offset += command.cmdsize;
    return 0;
}

int process_lc_symtab(char *macho_str, long *offset, struct thin_macho*tm){
    struct symtab_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct symtab_command));
    parse_lc_symtab(macho_str, &command, tm);
    *offset += command.cmdsize;
    return 0;
}

int process_lc_symseg(char *macho_str, long *offset){
    struct symseg_command  command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct symseg_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_loadfvmlib(char *macho_str, long *offset){
    struct fvmlib_command  command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct fvmlib_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_idfvmlib(char *macho_str, long *offset){
    struct fvmlib_command  command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct fvmlib_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_ident(char *macho_str, long *offset){
    struct ident_command  command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct ident_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_fvmfile(char *macho_str, long *offset){
    struct fvmfile_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct fvmfile_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_prepage(char *macho_str, long *offset){
    struct symtab_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct symtab_command));
    *offset += command.cmdsize;
    return 0;
}

int process_lc_prebind_cksum(char *macho_str, long *offset){
    struct prebind_cksum_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct prebind_cksum_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_load_weak_dylib(char *macho_str, long *offset){
    struct dylib_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dylib_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_rpath(char *macho_str, long *offset){
    struct rpath_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct rpath_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_code_signature(char *macho_str, long *offset){
    struct linkedit_data_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct linkedit_data_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_segment_split_info(char *macho_str, long *offset){
    struct linkedit_data_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct linkedit_data_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_lazy_load_dylib(char *macho_str, long *offset){
    struct symtab_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct symtab_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_encryption_info(char *macho_str, long *offset){
    struct encryption_info_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct encryption_info_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_command(char *macho_str, long *offset){
    struct load_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct load_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_dyld_environment(char *macho_str, long *offset){
    struct dylinker_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct dylinker_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_main(char *macho_str, long *offset){
    struct entry_point_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct entry_point_command));
    *offset += command.cmdsize;
    return 0;
}
int process_lc_dylib_code_sign_drs(char *macho_str, long *offset){
    struct linkedit_data_command command = {0};
    memcpy(&command, macho_str + *offset, sizeof(struct linkedit_data_command));
    *offset += command.cmdsize;
    return 0;
}
