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



/* We hold several abbreviation tables in memory at the same time. */
#ifndef ABBREV_HASH_SIZE
#define ABBREV_HASH_SIZE 121
#endif
typedef unsigned int CORE_ADDR;

struct lc_function_starts
{
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t offset;
    uint32_t size;
};

struct lc_data_in_code
{
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t offset;
    uint32_t size;
};



/* Persistent data held for a compilation unit, even when not
   processing it.  We put a pointer to this structure in the
   read_symtab_private field of the psymtab.  If we encounter
   inter-compilation-unit references, we also maintain a sorted
   list of all compilation units.  */

struct dwarf2_per_cu_data
{
    /* The start offset and length of this compilation unit.  2**31-1
       bytes should suffice to store the length of any compilation unit
       - if it doesn't, GDB will fall over anyway.  */
    unsigned long offset;
    unsigned long length : 31;

    /* Flag indicating this compilation unit will be read in before
       any of the current compilation units are processed.  */
    //unsigned long queued : 1;

    /* Set iff currently read in.  */
    struct dwarf2_cu *cu;

    /* If full symbols for this CU have been read in, then this field
       holds a map of DIE offsets to types.  It isn't always possible
       to reconstruct this information later, so we have to preserve
       it.  */
    //htab_t type_hash;

    ///* The partial symbol table associated with this compilation unit.  */
    //struct partial_symtab *psymtab;
};


/* .debug_pubnames header
   Because of alignment constraints, this structure has padding and cannot
   be mapped directly onto the beginning of the .debug_info section.  */
struct aranges_header
  {
    unsigned int length;	/* byte len of the .debug_aranges
				   contribution */
    unsigned short version;	/* version number -- 2 for dwarf
				   version 2 */
    unsigned int info_offset;	/* offset into .debug_info section */
    unsigned char addr_size;	/* byte size of an address */
    unsigned char seg_size;	/* byte size of segment descriptor */
  } ;


struct arange{
    struct aranges_header aranges_header;
    struct address_range_descriptor *address_range_descriptors;
    unsigned int num_of_ards;
};

struct dwarf2_per_objfile
{
    /* Sizes of debugging sections.  */
    unsigned int info_size;
    unsigned int abbrev_size;
    unsigned int line_size;
    unsigned int pubnames_size;
    unsigned int aranges_size;
    unsigned int loc_size;
    unsigned int macinfo_size;
    unsigned int str_size;
    unsigned int ranges_size;
    unsigned int inlined_size;
    unsigned int pubtypes_size;
    unsigned int frame_size;
    unsigned int eh_frame_size;

    /* Loaded data from the sections.  */
    char *info_buffer;
    char *abbrev_buffer;
    char *line_buffer;
    char *pubnames_buffer;
    char *aranges_buffer;
    char *loc_buffer;
    char *macinfo_buffer;
    char *str_buffer;
    char *ranges_buffer;
    char *inlined_buffer;
    char *pubtypes_buffer;
    char *frame_buffer;
    char *eh_frame_buffer;
    //char *

    /* A list of all the compilation units.  This is used to locate
       the target compilation unit of a particular reference.  */
    struct dwarf2_per_cu_data **all_comp_units;
    struct arange **all_aranges;

    /* The number of compilation units in ALL_COMP_UNITS.  */
    int n_comp_units;
    int n_aranges;

    /* A chain of compilation units that are currently read in, so that
       they can be freed later.  */
    struct dwarf2_per_cu_data *read_in_chain;
};


static struct dwarf2_per_objfile *dwarf2_per_objfile;



/*  This data structure holds the information of an abbrev. */
struct abbrev_info
{
    unsigned int number;    /*  number identifying abbrev */
    enum dwarf_tag tag;     /*  dwarf tag */
    unsigned short has_children;        /*  boolean */
    unsigned short num_attrs;   /*  number of attributes */
    struct attr_abbrev *attrs;  /*  an array of attribute descriptions */
    struct abbrev_info *next;   /*  next in chain */
};

struct attr_abbrev
{
    enum dwarf_attribute name; 
    enum dwarf_form form; 
};


/* Blocks are a bunch of untyped bytes. */
struct dwarf_block
{
    unsigned int size;
    char *data;
};



struct abbrev_info **dwarf2_abbrevs;

/* Attributes have a name and a value */
struct attribute
{
    enum dwarf_attribute name;
    enum dwarf_form form;
    union
    {
        char *str;
        struct dwarf_block *blk;
        unsigned long unsnd;
        long int snd;
        CORE_ADDR addr;
    }
    u;
};

/* This data structure holds a complete die structure. */
struct die_info
{
    enum dwarf_tag tag;		/* Tag indicating type of die */
    unsigned int abbrev;	/* Abbrev number */
    unsigned int offset;	/* Offset in .debug_info section */
    /* APPLE LOCAL - dwarf repository  */
    //unsigned int repository_id; /* Id number in debug repository */
    unsigned int num_attrs;	/* Number of attributes */
    struct attribute *attrs;	/* An array of attributes */
    //struct die_info *next_ref;	/* Next die in ref hash table */

    /* The dies in a compilation unit form an n-ary tree.  PARENT
       points to this die's parent; CHILD points to the first child of
       this node; and all the children of a given node are chained
       together via their SIBLING fields, terminated by a die whose
       tag is zero.  */
    struct die_info *child;	/* Its first child, if any.  */
    struct die_info *sibling;	/* Its next sibling, if any.  */
    struct die_info *parent;	/* Its parent, if any.  */

    //struct type *type;		/* Cached type information */
};



/* The data in a compilation unit header, after target2host
   translation, looks like this.  */
struct comp_unit_head
{
    unsigned long length;
    short version;
    unsigned int abbrev_offset;
    unsigned char addr_size;
    unsigned char signed_addr_p;

    /* Size of file offsets; either 4 or 8.  */
    unsigned int offset_size;

    /* Size of the length field; either 4 or 12.  */
    unsigned int initial_length_size;

    /* Offset to the first byte of this compilation unit header in the
       .debug_info section, for resolving relative reference dies.  */
    unsigned int offset;

    /* Pointer to this compilation unit header in the .debug_info
       section.  */
    char *cu_head_ptr;

    /* Pointer to the first die of this compilation unit.  This will be
       the first byte following the compilation unit header.  */
    char *first_die_ptr;

    /* Pointer to the next compilation unit header in the program.  */
    struct comp_unit_head *next;

    /* Base address of this compilation unit.  */
    CORE_ADDR base_address;

    /* Non-zero if base_address has been set.  */
    int base_known;
};


/* Internal state when decoding a particular compilation unit.  */
struct dwarf2_cu
{
    /* The objfile containing this compilation unit.  */
    //struct objfile *objfile;

    /* The header of the compilation unit.

       FIXME drow/2003-11-10: Some of the things from the comp_unit_head
       should logically be moved to the dwarf2_cu structure.  */
    struct comp_unit_head header;
    //TODO
    //struct function_range *first_fn, *last_fn, *cached_fn;

    /* The language we are debugging.  */
    enum language language;
    //const struct language_defn *language_defn;

    const char *producer;

    /* APPLE LOCAL: Retain the compilation directory pathname for header
       file relative pathnames (via gcc parameters like "-I../../../include").  */
    char *comp_dir;

    /* The generic symbol table building routines have separate lists for
       file scope symbols and all all other scopes (local scopes).  So
       we need to select the right one to pass to add_symbol_to_list().
       We do it by keeping a pointer to the correct list in list_in_scope.

FIXME: The original dwarf code just treated the file scope as the
first local scope, and all other local scopes as nested local
scopes, and worked fine.  Check to see if we really need to
distinguish these in buildsym.c.  */
    //struct pending **list_in_scope;

    /* Maintain an array of referenced fundamental types for the current
       compilation unit being read.  For DWARF version 1, we have to construct
       the fundamental types on the fly, since no information about the
       fundamental types is supplied.  Each such fundamental type is created by
       calling a language dependent routine to create the type, and then a
       pointer to that type is then placed in the array at the index specified
       by it's FT_<TYPENAME> value.  The array has a fixed size set by the
       FT_NUM_MEMBERS compile time constant, which is the number of predefined
       fundamental types gdb knows how to construct.  */
    //struct type *ftypes[FT_NUM_MEMBERS];	/* Fundamental types */

    /* DWARF abbreviation table associated with this compilation unit.  */
    struct abbrev_info **dwarf2_abbrevs;

    /* Storage for the abbrev table.  */
    //TODO
    //struct obstack abbrev_obstack;

    /* Hash table holding all the loaded partial DIEs.  */
    //htab_t partial_dies;

    /* Storage for things with the same lifetime as this read-in compilation
       unit, including partial DIEs.  */
    //struct obstack comp_unit_obstack;

    /* When multiple dwarf2_cu structures are living in memory, this field
       chains them all together, so that they can be released efficiently.
       We will probably also want a generation counter so that most-recently-used
       compilation units are cached...  */
    //struct dwarf2_per_cu_data *read_in_chain;

    /* Backchain to our per_cu entry if the tree has been built.  */
    struct dwarf2_per_cu_data *per_cu;

    /* How many compilation units ago was this CU last referenced?  */
    //int last_used;

    /* A hash table of die offsets for following references.  */
    //struct die_info *die_ref_table[REF_HASH_SIZE];

    /* Full DIEs if read in.  */
    struct die_info *dies;

    /* A set of pointers to dwarf2_per_cu_data objects for compilation
       units referenced by this one.  Only set during full symbol processing;
       partial symbol tables do not have dependencies.  */
    //htab_t dependencies;

    /* Mark used when releasing cached dies.  */
    //unsigned int mark : 1;

    /* This flag will be set if this compilation unit might include
       inter-compilation-unit references.  */
    //unsigned int has_form_ref_addr : 1;

    /* This flag will be set if this compilation unit includes any
       DW_TAG_namespace DIEs.  If we know that there are explicit
       DIEs for namespaces, we don't need to try to infer them
       from mangled names.  */
    //unsigned int has_namespace_info : 1;

    /* APPLE LOCAL begin dwarf repository  */
    //sqlite3 *repository;

    //char *repository_name;
    /* APPLE LOCAL end dwarf repository  */

    /* APPLE LOCAL debug map */
    //struct oso_to_final_addr_map *addr_map;
};

//struct dwarf2_cu *cu;


static void set_cu_language (unsigned int lang, struct dwarf2_cu *cu)
{
    switch (lang)
    {
        case DW_LANG_C89:
        case DW_LANG_C:
            cu->language = language_c;
            break;
        case DW_LANG_C_plus_plus:
            cu->language = language_cplus;
            break;
        case DW_LANG_Fortran77:
        case DW_LANG_Fortran90:
        case DW_LANG_Fortran95:
            cu->language = language_fortran;
            break;
        case DW_LANG_Mips_Assembler:
            cu->language = language_asm;
            break;
        case DW_LANG_Java:
            cu->language = language_java;
            break;
        case DW_LANG_Ada83:
        case DW_LANG_Ada95:
            cu->language = language_ada;
            break;
            /* APPLE LOCAL:  No need to be Apple local but not merged in to FSF..  */
        case DW_LANG_ObjC:
            cu->language = language_objc;
            break;
            /* APPLE LOCAL:  No need to be Apple local but not merged in to FSF..  */
        case DW_LANG_ObjC_plus_plus:
            cu->language = language_objcplus;
            break;
        case DW_LANG_Cobol74:
        case DW_LANG_Cobol85:
        case DW_LANG_Pascal83:
        case DW_LANG_Modula2:
        default:
            cu->language = language_minimal;
            break;
    }
    //cu->language_defn = language_def (cu->language);
}









struct data_of_interest doi = {0};

struct section *dwarf_section_headers = NULL;
struct abbrev_info *dwarf_abbrevs = NULL;

//struct dwarf_sections dwarf_sections = {0};
uint32_t numofdwarfsections = 0;
void free_dwarf2_per_objfile(){
    //uint32_t i = 0;
    //while(i < numofdwarfsections){
    //    free(dwarf2_per_objfile[i]);
    //    i++;
    //}
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

    free(dwarf_section_headers);
    //todo
    //free();
}
int parse_dwarf_segment(FILE* fp, struct segment_command *command){
    numofdwarfsections = command->nsects;
    dwarf2_per_objfile = malloc(sizeof(struct dwarf2_per_objfile));
    memset(dwarf2_per_objfile, '\0', sizeof(struct dwarf2_per_objfile));
    dwarf_section_headers = malloc(numofdwarfsections * sizeof(struct section));
    memset(dwarf_section_headers, '\0', numofdwarfsections * sizeof (struct section));
    int rc = 0;
    uint32_t i = 0;
    long current_pos = ftell (fp);
    while(i < numofdwarfsections){
        if( (rc = fread(&(dwarf_section_headers[i]) ,sizeof(struct section), 1, fp)) != 0 ){
            //printf("%s %s size: 0X%02X %d\n", dwarf_section_headers[i].segname, dwarf_section_headers[i].sectname, dwarf_section_headers[i].size, dwarf_section_headers[i].size); 
            //dwarf2_per_objfile->info_buffer
            unsigned char *temp = malloc(dwarf_section_headers[i].size);
            if (temp == NULL){
                printf("Malloc Error!\n");
                return -1;
            }
            //memset(dwarf2_per_objfile->info_buffer, '\0', dwarf_section_headers[i].size);
            memset(temp, '\0', dwarf_section_headers[i].size);

            long temp_position = ftell(fp);
            fseek(fp, dwarf_section_headers[i].offset, SEEK_SET);
            int numofbytes = fread(temp, sizeof(char), dwarf_section_headers[i].size, fp);
            assert(numofbytes == dwarf_section_headers[i].size);
            fseek(fp, temp_position, SEEK_SET);
            //todo asset

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
            }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_names") == 0){
                //dwarf2_per_objfile->abbrev_buffer = temp;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_types") == 0){
                //dwarf2_per_objfile->abbrev_buffer = temp;
            }else if((strcmp(dwarf_section_headers[i].sectname, "__apple_namespa") == 0) ||
                    (strcmp(dwarf_section_headers[i].sectname, "__apple_namespac__DWARF") == 0) 
                    ){
                //dwarf2_per_objfile->abbrev_buffer = temp;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_objc") == 0){
                //dwarf2_per_objfile->abbrev_buffer = temp;
            }else{
                printf("╮(╯▽╰)╭, %s \n", dwarf_section_headers[i].sectname);
                free(temp);
            }
        }
        rc = 0;
        i++;
    }
    int seekreturn = 0;
    seekreturn = fseek (fp, current_pos, SEEK_SET); 
    assert(seekreturn == 0);
}

void print_all_dwarf2_per_objfile(){
    int i = 0;
    for (i =0; i< dwarf2_per_objfile->abbrev_size; i++){
        printf("%02x", dwarf2_per_objfile->abbrev_buffer[i]);
    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->aranges_size; i++){
        printf("%02x", dwarf2_per_objfile->aranges_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->info_size; i++){
        printf("%02x", dwarf2_per_objfile->info_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->inlined_size; i++){
        printf("%02x", dwarf2_per_objfile->inlined_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->line_size; i++){
        printf("%02x", dwarf2_per_objfile->line_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->loc_size; i++){
        printf("%02x", dwarf2_per_objfile->loc_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->pubnames_size; i++){
        printf("%02x", dwarf2_per_objfile->pubnames_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->pubtypes_size; i++){
        printf("%02x", dwarf2_per_objfile->pubtypes_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->ranges_size; i++){
        printf("%02x", dwarf2_per_objfile->ranges_buffer[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf2_per_objfile->str_size; i++){
        printf("%02x", dwarf2_per_objfile->str_buffer[i]);

    }
    printf("\n");
    printf("\n");
}

int parse_macho(const char *filename){
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("Read File Error.\n");
        return 0;
    }

//    printf("Parsing Mach Header\n");

    struct mach_header mh = {0};
    int rc = 0;
    int num_load_cmds = 0;
    if( (rc = fread(&mh ,sizeof(struct mach_header), 1, fp)) != 0 )
    {
        num_load_cmds = mh.ncmds;
        //printf("sizeof struct mach_header: %u\n", sizeof(struct mach_header));
        //printf("magic: %u\n", mh.magic);
        //printf("cputype: %u\n", mh.cputype);
        //printf("cpusubtype: %u\n", mh.cpusubtype);
        //printf("filetype: %u\n", mh.filetype);
        //printf("MH_EXECUTE: %u\n", MH_EXECUTE);
        //printf("ncmds: %u\n", mh.ncmds);
        //printf("sizeofcmds: %u\n", mh.sizeofcmds);
        //printf("flags: %u\n", mh.flags);
        //printf("record count %d\n", rc);
    } 

    //printf("Parsing Load Commands\n");
    struct load_command lc = {0}; 
    int i = 0;
    //num_load_cmds = 3;
    while (i < num_load_cmds){
        //printf("Start Reading Load Command %d\n", i);
        //todo try continue when find the data that we are interested with.
        if( (rc = fread(&lc ,sizeof(struct load_command), 1, fp)) != 0 )
        {
            assert(rc == 1);
            //printf("record count: %d\n", rc);
            parse_load_command(fp, &lc);
        }else{
            printf("Read Load Command Error\n");
        }
        i++;
    }

    fclose(fp);
    //print_all_dwarf2_per_objfile();
    parse_dwarf2_per_objfile();
    return 0; 
}




//static unsigned long
//read_unsigned_leb128 (bfd *abfd, char *buf, unsigned int *bytes_read_ptr)
//{
//  unsigned long result;
//  unsigned int num_read;
//  int i, shift;
//  unsigned char byte;
//
//  result = 0;
//  shift = 0;
//  num_read = 0;
//  i = 0;
//  while (1)
//    {
//      byte = bfd_get_8 (abfd, (bfd_byte *) buf);
//      buf++;
//      num_read++;
//      result |= ((unsigned long)(byte & 127) << shift);
//      if ((byte & 128) == 0)
//	{
//	  break;
//	}
//      shift += 7;
//    }
//  *bytes_read_ptr = num_read;
//  return result;
//}
//
//static long
//read_signed_leb128 (bfd *abfd, char *buf, unsigned int *bytes_read_ptr)
//{
//  long result;
//  int i, shift, num_read;
//  unsigned char byte;
//
//  result = 0;
//  shift = 0;
//  num_read = 0;
//  i = 0;
//  while (1)
//    {
//      byte = bfd_get_8 (abfd, (bfd_byte *) buf);
//      buf++;
//      num_read++;
//      result |= ((long)(byte & 127) << shift);
//      shift += 7;
//      if ((byte & 128) == 0)
//	{
//	  break;
//	}
//    }
//  if ((shift < 8 * sizeof (result)) && (byte & 0x40))
//    result |= -(((long)1) << shift);
//  *bytes_read_ptr = num_read;
//  return result;
//}

/* Return a pointer to just past the end of an LEB128 number in BUF.  */

static char * skip_leb128 (unsigned char *buf)
{
    int byte;

    while (1)
    {
        byte = *buf;
        buf++;
        if ((byte & 128) == 0)
            return buf;
    }
}









long long read_signed_leb128(unsigned char* leb128, unsigned int* leb128_length)
{
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


unsigned long long read_unsigned_leb128(unsigned char* leb128, unsigned int* leb128_length)
{
    unsigned char byte;
    unsigned long word_number;
    unsigned long long number;
    signed long shift;
    signed long byte_length;

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
unsigned int get_num_attr_spec_pair(unsigned char* info_ptr){
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

void free_dwarf_abbrev(){

}

CORE_ADDR read_signed_16(char *buf){
    //FIXME
    return (*buf + (*buf << 8));
}

signed int read_signed_32(unsigned char *info_ptr){
    signed int ret = 0;
    ret = info_ptr[3];
    ret = (ret << 8) + info_ptr[2];
    ret = (ret << 8) + info_ptr[1];
    ret = (ret << 8) + info_ptr[0];
    return ret;
}

static unsigned int read_1_byte (unsigned char *info_ptr)
{
    return *info_ptr;
}

//static int
//read_1_signed_byte (bfd *abfd, char *buf)
//{
//  return bfd_get_signed_8 (abfd, (bfd_byte *) buf);
//}

static unsigned int read_2_bytes (unsigned char *info_ptr)
{
    //read bytes little endian?
    unsigned short ret = 0;
    ret = info_ptr[1];
    ret = (ret << 8) + info_ptr[0];
    return ret;
}

//static int
//read_2_signed_bytes (bfd *abfd, char *buf)
//{
//  return bfd_get_signed_16 (abfd, (bfd_byte *) buf);
//}

static unsigned int read_4_bytes (unsigned char *info_ptr)
{
    unsigned int ret = 0;
    ret = info_ptr[3];
    //printf("ret: %x\n", ret);
    //printf("info_ptr: %x\n", (unsigned char)info_ptr[3]);
    ret = (ret << 8) + info_ptr[2];
    //printf("ret: %x\n", ret);
    //printf("info_ptr: %x\n", (unsigned char)info_ptr[2]);
    ret = (ret << 8) + info_ptr[1];
    //printf("ret: %x\n", ret);
    //printf("info_ptr: %x\n", (unsigned char)info_ptr[1]);
    ret = (ret << 8) + info_ptr[0];
    //printf("ret: %x\n", ret);
    //printf("info_ptr: %x\n", (unsigned char)info_ptr[0]);
    //printf("\n");
    return ret;
}

//static int
//read_4_signed_bytes (bfd *abfd, char *buf)
//{
//  return bfd_get_signed_32 (abfd, (bfd_byte *) buf);
//}

static unsigned long read_8_bytes (unsigned char *info_ptr)
{
    //read bytes little endian?
    unsigned long ret = 0;
    ret = info_ptr[7];
    ret = (ret << 8) + info_ptr[6];
    ret = (ret << 8) + info_ptr[5];
    ret = (ret << 8) + info_ptr[4];
    ret = (ret << 8) + info_ptr[3];
    ret = (ret << 8) + info_ptr[2];
    ret = (ret << 8) + info_ptr[1];
    ret = (ret << 8) + info_ptr[0];
    return ret;
}

unsigned int read_unsigned_int(unsigned char *info_ptr){
    //read bytes little endian?
    unsigned int ret = 0;
    ret = info_ptr[3];
    ret = (ret << 8) + info_ptr[2];
    ret = (ret << 8) + info_ptr[1];
    ret = (ret << 8) + info_ptr[0];
    return ret;
}

unsigned short read_unsigned_short(unsigned char *info_ptr){
    //read bytes little endian?
    unsigned short ret = 0;
    ret = info_ptr[1];
    ret = (ret << 8) + info_ptr[0];
    return ret;
}

read_unsigned_char(unsigned char *info_ptr){
    unsigned char ret = 0;
    ret = info_ptr[0];
    return ret;
}


/* Release the memory used by the abbrev table for a compilation unit.  */

    static void
dwarf2_free_abbrev_table (void *ptr_to_cu)
{
    struct dwarf2_cu *cu = ptr_to_cu;

    //obstack_free (&cu->abbrev_obstack, NULL);
    cu->dwarf2_abbrevs = NULL;
}

/* Lookup an abbrev_info structure in the abbrev hash table.  */

    static struct abbrev_info *
dwarf2_lookup_abbrev (unsigned int number, struct dwarf2_cu *cu)
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


/* When we construct a partial symbol table entry we only
   need this much information. */
//struct partial_die_info{
//    /* Offset of this DIE.  */
//    unsigned int offset;
//
//    /* DWARF-2 tag for this DIE.  */
//    enum dwarf_tag tag;		/* Tag indicating type of die */
//
//    /* Language code associated with this DIE.  This is only used
//       for the compilation unit DIE.  */
//    unsigned int language : 8;
//
//    /* Assorted flags describing the data found in this DIE.  */
//    unsigned int has_children : 1;
//    unsigned int is_external : 1;
//    unsigned int is_declaration : 1;
//    unsigned int has_type : 1;
//    unsigned int has_specification : 1;
//    unsigned int has_stmt_list : 1;
//    unsigned int has_pc_info : 1;
//    /* APPLE LOCAL begin dwarf repository  */
//    unsigned int has_repo_specification : 1;
//    unsigned int has_repository : 1;
//    unsigned int has_repository_type : 1;
//    /* APPLE LOCAL end dwarf repository  */
//
//    /* Flag set if the SCOPE field of this structure has been
//       computed.  */
//    unsigned int scope_set : 1;
//
//    /* The name of this DIE.  Normally the value of DW_AT_name, but
//       sometimes DW_TAG_MIPS_linkage_name or a string computed in some
//       other fashion.  */
//    char *name;
//    char *dirname;
//
//    /* The scope to prepend to our children.  This is generally
//       allocated on the comp_unit_obstack, so will disappear
//       when this compilation unit leaves the cache.  */
//    //char *scope;
//
//    /* The location description associated with this DIE, if any.  */
//    //struct dwarf_block *locdesc;
//
//    /* If HAS_PC_INFO, the PC range associated with this DIE.  */
//    CORE_ADDR lowpc;
//    CORE_ADDR highpc;
//
//    /* Pointer into the info_buffer pointing at the target of
//       DW_AT_sibling, if any.  */
//    char *sibling;
//
//    /* If HAS_SPECIFICATION, the offset of the DIE referred to by
//       DW_AT_specification (or DW_AT_abstract_origin or
//       DW_AT_extension).  */
//    unsigned int spec_offset;
//
//    /* APPLE LOCAL begin dwarf repository  */
//    /* If HAS_REPO_SPECIFICATION, the id of the DIE in the sql
//       repository referred to by DW_AT_APPLE_repository_specification.  */
//
//    unsigned int repo_spec_id;
//    
//    /* The filename of the dwarf sql repository file, if one was used.  */
//
//    char *repo_name;
//    /* APPLE LOCAL end dwarf repository  */
//
//    /* If HAS_STMT_LIST, the offset of the Line Number Information data.  */
//    unsigned int line_offset;
//
//    /* Pointers to this DIE's parent, first child, and next sibling,
//       if any.  */
//    struct partial_die_info *die_parent, *die_child, *die_sibling;
//};
//


unsigned char * read_comp_unit_head (struct comp_unit_head *header, char *info_ptr)
{
    char *beg_of_comp_unit = info_ptr;
    //  struct comp_unit_head *cuh = malloc(sizeof(struct comp_unit_head));
    //  memset(cuh, '\0', sizeof(struct comp_unit_head));

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


//---------------------
//
//


/* Read the initial uleb128 in the die at current position in compilation unit CU.
   Return the corresponding abbrev, or NULL if the number is zero (indicating
   an empty DIE).  In either case *BYTES_READ will be set to the length of
   the initial number.  */

static struct abbrev_info * peek_die_abbrev (char *info_ptr, unsigned int *bytes_read, struct dwarf2_cu *cu)
{
    unsigned int abbrev_number;
    struct abbrev_info *abbrev;

    abbrev_number = (unsigned int)read_unsigned_leb128(info_ptr, bytes_read);
    printf("%u\n", abbrev_number);

    if (abbrev_number == 0)
        return NULL;

    abbrev = dwarf2_lookup_abbrev (abbrev_number, cu);
    if (!abbrev)
    {
        printf("Dwarf Error: Could not find abbrev number %d\n", abbrev_number);
    }

    return abbrev;
}




/* Build the partial symbol table by doing a quick pass through the
   .debug_info and .debug_abbrev sections.  */

//static void
//dwarf2_build_psymtabs_hard (struct objfile *objfile, int mainline)
//{
//  /* Instead of reading this into a big buffer, we should probably use
//     mmap()  on architectures that support it. (FIXME) */
//  bfd *abfd = objfile->obfd;
//  char *info_ptr;
//  char *beg_of_comp_unit;
//  struct partial_die_info comp_unit_die;
//  struct partial_symtab *pst;
//  struct cleanup *back_to;
//  CORE_ADDR lowpc, highpc, baseaddr;
//
//  /* APPLE LOCAL begin dwarf repository  */
//  if (bfd_big_endian (abfd) == BFD_ENDIAN_BIG)
//    byte_swap_p = 0;
//  else
//    byte_swap_p = 1;
//  /* APPLE LOCAL end dwarf repository  */
//  info_ptr = dwarf2_per_objfile->info_buffer;
//
//  /* Any cached compilation units will be linked by the per-objfile
//     read_in_chain.  Make sure to free them when we're done.  */
//  back_to = make_cleanup (free_cached_comp_units, NULL);
//
//  create_all_comp_units (objfile);
//
//  /* Since the objects we're extracting from .debug_info vary in
//     length, only the individual functions to extract them (like
//     read_comp_unit_head and load_partial_die) can really know whether
//     the buffer is large enough to hold another complete object.
//
//     At the moment, they don't actually check that.  If .debug_info
//     holds just one extra byte after the last compilation unit's dies,
//     then read_comp_unit_head will happily read off the end of the
//     buffer.  read_partial_die is similarly casual.  Those functions
//     should be fixed.
//
//     For this loop condition, simply checking whether there's any data
//     left at all should be sufficient.  */
//  while (info_ptr < (dwarf2_per_objfile->info_buffer
//		     + dwarf2_per_objfile->info_size))
//    {
//      struct cleanup *back_to_inner;
//      struct dwarf2_cu cu;
//      struct abbrev_info *abbrev;
//      unsigned int bytes_read;
//      struct dwarf2_per_cu_data *this_cu;
//
//      beg_of_comp_unit = info_ptr;
//
//      memset (&cu, 0, sizeof (cu));
//
//      obstack_init (&cu.comp_unit_obstack);
//
//      back_to_inner = make_cleanup (free_stack_comp_unit, &cu);
//
//      cu.objfile = objfile;
//      info_ptr = partial_read_comp_unit_head (&cu.header, info_ptr, abfd);
//
//      /* Complete the cu_header */
//      cu.header.offset = beg_of_comp_unit - dwarf2_per_objfile->info_buffer;
//      cu.header.first_die_ptr = info_ptr;
//      cu.header.cu_head_ptr = beg_of_comp_unit;
//
//      cu.list_in_scope = &file_symbols;
//
//      /* Read the abbrevs for this compilation unit into a table */
//      dwarf2_read_abbrevs (abfd, &cu);
//      make_cleanup (dwarf2_free_abbrev_table, &cu);
//
//      this_cu = dwarf2_find_comp_unit (cu.header.offset, objfile);
//
//      /* Read the compilation unit die */
//      /* APPLE LOCAL Add cast to avoid type mismatch in arg2 warning.  */
//      abbrev = peek_die_abbrev (info_ptr, (int *) &bytes_read, &cu);
//      info_ptr = read_partial_die (&comp_unit_die, abbrev, bytes_read,
//				   abfd, info_ptr, &cu);
//      /* APPLE LOCAL begin dwarf repository  */
//      if (comp_unit_die.has_repository)
//	{
//	  dwarf2_read_repository_abbrevs (&cu);
//	  set_repository_cu_language (comp_unit_die.language, &cu);
//	}
//      /* APPLE LOCAL end dwarf repository  */
//
//      /* Set the language we're debugging */
//      set_cu_language (comp_unit_die.language, &cu);
//
//      /* Allocate a new partial symbol table structure */
//      pst = start_psymtab_common (objfile, objfile->section_offsets,
//				  comp_unit_die.name ? comp_unit_die.name : "",
//				  comp_unit_die.lowpc,
//				  objfile->global_psymbols.next,
//				  objfile->static_psymbols.next);
//
//      if (comp_unit_die.dirname)
//	pst->dirname = xstrdup (comp_unit_die.dirname);
//
//      pst->read_symtab_private = (char *) this_cu;
//
//      baseaddr = ANOFFSET (objfile->section_offsets, SECT_OFF_TEXT (objfile));
//
//      /* Store the function that reads in the rest of the symbol table */
//      pst->read_symtab = dwarf2_psymtab_to_symtab;
//
//      /* If this compilation unit was already read in, free the
//	 cached copy in order to read it in again.  This is
//	 necessary because we skipped some symbols when we first
//	 read in the compilation unit (see load_partial_dies).
//	 This problem could be avoided, but the benefit is
//	 unclear.  */
//      if (this_cu->cu != NULL)
//	free_one_cached_comp_unit (this_cu->cu);
//
//      cu.per_cu = this_cu;
//
//      /* Note that this is a pointer to our stack frame, being
//	 added to a global data structure.  It will be cleaned up
//	 in free_stack_comp_unit when we finish with this
//	 compilation unit.  */
//      this_cu->cu = &cu;
//
//      this_cu->psymtab = pst;
//
//      /* Check if comp unit has_children.
//         If so, read the rest of the partial symbols from this comp unit.
//         If not, there's no more debug_info for this comp unit. */
//      if (comp_unit_die.has_children)
//	{
//	  struct partial_die_info *first_die;
//
//	  lowpc = ((CORE_ADDR) -1);
//	  highpc = ((CORE_ADDR) 0);
//
//	  first_die = load_partial_dies (abfd, info_ptr, 1, &cu);
//
//	  scan_partial_symbols (first_die, &lowpc, &highpc, &cu);
//
//	  /* If we didn't find a lowpc, set it to highpc to avoid
//	     complaints from `maint check'.  */
//	  if (lowpc == ((CORE_ADDR) -1))
//	    lowpc = highpc;
//
//	  /* If the compilation unit didn't have an explicit address range,
//	     then use the information extracted from its child dies.  */
//	  if (! comp_unit_die.has_pc_info)
//	    {
//	      comp_unit_die.lowpc = lowpc;
//	      comp_unit_die.highpc = highpc;
//	    }
//	}
//      pst->textlow = comp_unit_die.lowpc + baseaddr;
//      pst->texthigh = comp_unit_die.highpc + baseaddr;
//
//      pst->n_global_syms = objfile->global_psymbols.next -
//	(objfile->global_psymbols.list + pst->globals_offset);
//      pst->n_static_syms = objfile->static_psymbols.next -
//	(objfile->static_psymbols.list + pst->statics_offset);
//      sort_pst_symbols (pst);
//
//      /* If there is already a psymtab or symtab for a file of this
//         name, remove it. (If there is a symtab, more drastic things
//         also happen.) This happens in VxWorks.  */
//      free_named_symtabs (pst->filename);
//
//      info_ptr = beg_of_comp_unit + cu.header.length
//                                  + cu.header.initial_length_size;
//
//      if (comp_unit_die.has_stmt_list)
//        {
//          /* Get the list of files included in the current compilation unit,
//             and build a psymtab for each of them.  */
//          dwarf2_build_include_psymtabs (&cu, &comp_unit_die, pst);
//        }
//
//      do_cleanups (back_to_inner);
//    }
//  do_cleanups (back_to);
//}


/* Load the DIEs for a secondary CU into memory.  */

//static void load_comp_unit (struct dwarf2_per_cu_data *this_cu, struct objfile *objfile)
//{
//  bfd *abfd = objfile->obfd;
//  char *info_ptr, *beg_of_comp_unit;
//  struct partial_die_info comp_unit_die;
//  struct dwarf2_cu *cu;
//  struct abbrev_info *abbrev;
//  unsigned int bytes_read;
//  struct cleanup *back_to;
//
//  info_ptr = dwarf2_per_objfile->info_buffer + this_cu->offset;
//  beg_of_comp_unit = info_ptr;
//
//  cu = xmalloc (sizeof (struct dwarf2_cu));
//  memset (cu, 0, sizeof (struct dwarf2_cu));
//
//  obstack_init (&cu->comp_unit_obstack);
//
//  cu->objfile = objfile;
//  info_ptr = partial_read_comp_unit_head (&cu->header, info_ptr, abfd);
//
//  /* Complete the cu_header.  */
//  cu->header.offset = beg_of_comp_unit - dwarf2_per_objfile->info_buffer;
//  cu->header.first_die_ptr = info_ptr;
//  cu->header.cu_head_ptr = beg_of_comp_unit;
//
//  /* Read the abbrevs for this compilation unit into a table.  */
//  dwarf2_read_abbrevs (abfd, cu);
//  back_to = make_cleanup (dwarf2_free_abbrev_table, cu);
//
//  /* Read the compilation unit die.  */
//  /* APPLE LOCAL Add cast to avoid type mismatch in arg2 warning.  */
//  abbrev = peek_die_abbrev (info_ptr, (int *) &bytes_read, cu);
//  info_ptr = read_partial_die (&comp_unit_die, abbrev, bytes_read,
//			       abfd, info_ptr, cu);
//
//  /* Set the language we're debugging.  */
//  set_cu_language (comp_unit_die.language, cu);
//
//  /* Link this compilation unit into the compilation unit tree.  */
//  this_cu->cu = cu;
//  cu->per_cu = this_cu;
//
//  /* Check if comp unit has_children.
//     If so, read the rest of the partial symbols from this comp unit.
//     If not, there's no more debug_info for this comp unit. */
//  if (comp_unit_die.has_children)
//    load_partial_dies (abfd, info_ptr, 0, cu);
//
//  do_cleanups (back_to);
//}



static char * read_n_bytes (unsigned char *buf, unsigned int size)
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
            //case 8:
            //    retval = bfd_get_signed_64 (abfd, (bfd_byte *) buf);
            //    break;
        default:
            printf("read address: bad switch, signed\n");
    }
    *bytes_read = arange->aranges_header.addr_size;
    return retval;
}

//CORE_ADDR read_signed_64(char *buf){
//    return (*buf + *(buf+1) << 8 + *(buf + 2) << 16 + *(buf + 3) << 24);
//}
//FIXME add cu_header->addr_size
static CORE_ADDR read_address_of_cu (char *buf, struct dwarf2_cu *cu, int *bytes_read)
{
    struct comp_unit_head *cu_header = &cu->header;
    CORE_ADDR retval = 0;

    //if (cu_header->signed_addr_p)
    //{
    switch (cu_header->addr_size)
    {
        case 2:
            //unsigned
            retval = read_signed_16(buf);
            break;
        case 4:
            retval = read_signed_32(buf);
            break;
            //case 8:
            //    retval = bfd_get_signed_64 (abfd, (bfd_byte *) buf);
            //    break;
        default:
            printf("read address: bad switch, signed\n");
    }
    //}
    //else
    //{
    //    switch (cu_header->addr_size)
    //    {
    //        case 2:
    //            retval = bfd_get_16 (abfd, (bfd_byte *) buf);
    //            break;
    //        case 4:
    //            retval = bfd_get_32 (abfd, (bfd_byte *) buf);
    //            break;
    //        case 8:
    //            retval = bfd_get_64 (abfd, (bfd_byte *) buf);
    //            break;
    //        default:
    //            internal_error (__FILE__, __LINE__,
    //                    _("read_address_of_cu: bad switch, unsigned [in module %s]"),
    //                    bfd_get_filename (abfd));
    //    }
    //}

    *bytes_read = cu_header->addr_size;
    return retval;
}


/* memory allocation interface */

static struct dwarf_block * dwarf_alloc_block (struct dwarf2_cu *cu)
{
    struct dwarf_block *blk;

    blk = (struct dwarf_block *) malloc(sizeof (struct dwarf_block));
    memset(blk, '\0', sizeof(struct dwarf_block));
    return (blk);
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
            printf("Dwarf Error: Cannot handle  in DWARF reader [in module s]");
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

static struct die_info * dwarf_alloc_die ()
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

static char * read_full_die (struct die_info **diep, unsigned char *info_ptr,
        struct dwarf2_cu *cu, int *has_children)
{
    unsigned int abbrev_number, i, offset;
    unsigned int bytes_read;
    struct abbrev_info *abbrev;
    struct die_info *die;
    char *comp_dir = NULL;

    //offset = info_ptr - dwarf2_per_objfile->info_buffer;
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
        printf("Dwarf Error: could not find abbrev number %d\n", abbrev_number);
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
        //   /* APPLE LOCAL end dwarf repository  */

        /* If this attribute is an absolute reference to a different
           compilation unit, make sure that compilation unit is loaded
           also.  */
        //if (die->attrs[i].form == DW_FORM_ref_addr
        //        && (DW_ADDR (&die->attrs[i]) < cu->header.offset
        //            || (DW_ADDR (&die->attrs[i])
        //                >= cu->header.offset + cu->header.length)))
        //{
        //    struct dwarf2_per_cu_data *per_cu;
        //    per_cu = dwarf2_find_containing_comp_unit (DW_ADDR (&die->attrs[i]),
        //            cu->objfile);

        //    /* Mark the dependence relation so that we don't flush PER_CU
        //       too early.  */
        //    dwarf2_add_dependence (cu, per_cu);

        //    /* If it's already on the queue, we have nothing to do.  */
        //    if (per_cu->queued)
        //        continue;

        //    /* If the compilation unit is already loaded, just mark it as
        //       used.  */
        //    if (per_cu->cu != NULL)
        //    {
        //        per_cu->cu->last_used = 0;
        //        continue;
        //    }

        //    /* Add it to the queue.  */
        //    //queue_comp_unit (per_cu);
        //}
    }

    //printf("\n");
    /* APPLE LOCAL begin dwarf repository  */
    //    if (repository_name)
    //       open_dwarf_repository (comp_dir, repository_name, cu->objfile, cu);
    /* APPLE LOCAL end dwarf repository  */

    *diep = die;
    *has_children = abbrev->has_children;
    return info_ptr;
}


static struct die_info * read_die_and_children (char *info_ptr, struct dwarf2_cu *cu, char **new_info_ptr, struct die_info *parent);
/* Read a whole compilation unit into a linked list of dies.  */

static struct die_info * read_comp_unit (char *info_ptr, struct dwarf2_cu *cu)
{
    return read_die_and_children (info_ptr, cu, &info_ptr, NULL);
}
static struct die_info * read_die_and_siblings (char *info_ptr, struct dwarf2_cu *cu, char **new_info_ptr, struct die_info *parent);

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


/* Load the DIEs for a secondary CU into memory.  */

static void load_comp_unit(unsigned char* info_ptr){
    struct dwarf2_cu *cu;
    struct die_info comp_unit_die;
    struct abbrev_info *abbrev;
    unsigned int bytes_read;
    //struct cleanup *back_to;

    //info_ptr = dwarf2_per_objfile->info_buffer + this_cu->offset;
    //beg_of_comp_unit = info_ptr;

    cu = malloc (sizeof (struct dwarf2_cu));
    memset (cu, 0, sizeof (struct dwarf2_cu));

    cu->dwarf2_abbrevs = dwarf2_abbrevs;
    //obstack_init (&cu->comp_unit_obstack);

    //cu->objfile = objfile;
    info_ptr = read_comp_unit_head (&cu->header, info_ptr);

    /* Complete the cu_header.  */
    //cu->header.offset = beg_of_comp_unit - dwarf2_per_objfile->info_buffer;
    //cu->header.first_die_ptr = info_ptr;
    //cu->header.cu_head_ptr = beg_of_comp_unit;

    /* Read the abbrevs for this compilation unit into a table.  */
    //parse_dwarf_abbrev(cu);
    //back_to = make_cleanup (dwarf2_free_abbrev_table, cu);

    /* Read the compilation unit die.  */
    /* APPLE LOCAL Add cast to avoid type mismatch in arg2 warning.  */
    abbrev = peek_die_abbrev (info_ptr, &bytes_read, cu);
    //FIXME
    //info_ptr = read_partial_die (&comp_unit_die, abbrev, bytes_read,
    //	      info_ptr, cu);

    /* Set the language we're debugging.  */
    //FIXME
    //set_cu_language (comp_unit_die.language, cu);

    /* Link this compilation unit into the compilation unit tree.  */
    //this_cu->cu = cu;
    //cu->per_cu = this_cu;

    /* Check if comp unit has_children.
       If so, read the rest of the partial symbols from this comp unit.
       If not, there's no more debug_info for this comp unit. */
    //if (comp_unit_die.has_children)


    //TODO ADDD
    //if (comp_unit_die.has_children)
    //  load_partial_dies (abfd, info_ptr, 0, cu);
}
//---------------------


/* Load the DIEs associated with PST and PER_CU into memory.  */
/* APPLE LOCAL debug map: Accept an optional 2nd parameter ADDR_MAP */

static struct dwarf2_cu * load_full_comp_unit (struct dwarf2_per_cu_data *per_cu)
{
  //struct partial_symtab *pst = per_cu->psymtab;
  //bfd *abfd = pst->objfile->obfd;
  struct dwarf2_cu *cu;
  unsigned long offset;
  char *info_ptr;
  //`struct cleanup *back_to, *free_cu_cleanup;
  struct attribute *attr;
  /* APPLE LOCAL avoid unused var warning. */
  /* CORE_ADDR baseaddr; */

  /* Set local variables from the partial symbol table info.  */
  offset = per_cu->offset;
  //printf("offset: %08lx\n", offset);

  info_ptr = dwarf2_per_objfile->info_buffer + offset;

  cu = malloc (sizeof (struct dwarf2_cu));
  memset (cu, 0, sizeof (struct dwarf2_cu));

  /* If an error occurs while loading, release our storage.  */
  //free_cu_cleanup = make_cleanup (free_one_comp_unit, cu);

  //cu->objfile = pst->objfile;

  /* read in the comp_unit header  */
  info_ptr = read_comp_unit_head (&cu->header, info_ptr);

  /* Read the abbrevs for this compilation unit  */
  //dwarf2_read_abbrevs (cu);
  cu->dwarf2_abbrevs = dwarf2_abbrevs;
  

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

  return cu;
}

/* Generate full symbol information for PST and CU, whose DIEs have
   already been loaded into memory.  */


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
        /* Handle the (non-standard) 64-bit DWARF2 format used by IRIX.  */
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

        if (cu_header->initial_length_size != 0 && cu_header->initial_length_size != *bytes_read){
            printf("asset error ...\n");
        }

        cu_header->initial_length_size = *bytes_read;
        cu_header->offset_size = (*bytes_read == 4) ? 4 : 8;
    }

    return length;
}


/* Create a list of all compilation units in OBJFILE.  We do this only
   if an inter-comp-unit reference is found; presumably if there is one,
   there will be many, and one will occur early in the .debug_info section.
   So there's no point in building this list incrementally.  */

static void create_all_comp_units ()
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
        /* APPLE LOCAL avoid unused var warning.  */
        /* char *beg_of_comp_unit; */
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
    //xfree (all_comp_units);
    dwarf2_per_objfile->n_comp_units = n_comp_units;
}





//free abbrev hash
void parse_dwarf_abbrev(){
    //allocate space form the abbrev hash
    dwarf2_abbrevs = malloc(sizeof(struct abbrev_info *) * ABBREV_HASH_SIZE);
    memset(dwarf2_abbrevs, 0, sizeof(struct abbrev_info *) * ABBREV_HASH_SIZE);
    unsigned char * info_ptr = dwarf2_per_objfile->abbrev_buffer;
    int size = dwarf2_per_objfile->abbrev_size;
    unsigned char* endof_abbrev_pos = info_ptr + size;
    int i = 0;
    while(info_ptr < endof_abbrev_pos && *info_ptr != '\0'){
        unsigned char temp = 0;
        unsigned int bytes_read = 0;
        unsigned long long abbrev_code = read_unsigned_leb128(info_ptr, &bytes_read);
        //printf("%llu %u\n", abbrev_code, bytes_read);
        info_ptr += bytes_read;
        unsigned long long entry_code = read_unsigned_leb128(info_ptr, &bytes_read);
        //printf("%llu\n", entry_code);
        info_ptr += bytes_read;
        unsigned char has_children = *info_ptr;
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
                //printf("%02X ", attr->name);
                //printf("%02X", attr->form);
                //printf("\n");
                //printf("%s %s\n", dwarf_attr_name(attrs[j].name), dwarf_form_name(attrs[j].form));
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
        //printf("\n");
        while(*temp_abbr != NULL){
            *temp_abbr = (*temp_abbr)->next;
        }
        *temp_abbr = ai;
        //if(prev != NULL){
        //    prev->next = ai;
        //}
        //prev = ai;
        //if(i == 0){
        //    dwarf_abbrevs = prev;
        //}
        i++;
    }
    //printf("%02X", *info_ptr);
}

void free_dwarf_cu(struct dwarf2_cu *cu){
    free(cu);
}


void parse_dwarf_info(){
    create_all_comp_units();
    int i = 0;
    struct dwarf2_cu *temp = NULL;
    for (i = 0; i< dwarf2_per_objfile->n_comp_units; i++){
        //printf("Load comp_units %d\n", i);
        temp = load_full_comp_unit(dwarf2_per_objfile->all_comp_units[i]);
    }
    //unsigned char * info_ptr = dwarf2_per_objfile->info_buffer;
    //int size = dwarf2_per_objfile->info_size;
    //unsigned char* endof_info_pos = info_ptr + size;

    //unsigned char temp = 0;
    //unsigned int bytes_read = 0;
    //unsigned long long abbrev_code = read_unsigned_leb128(info_ptr, &bytes_read);
    //printf("%llu %u\n", abbrev_code, bytes_read);
    //info_ptr += bytes_read;

    //dwarf2_lookup_abbrev ((unsigned int)abbrev_code, cu);




}

struct address_range_descriptor{
    CORE_ADDR beginning_addr;
    unsigned int length;
};

unsigned int get_num_arange_descriptor(char *aranges_ptr, struct arange *arange){
    int bytes_read = 0;
    unsigned int num_of_ards = 0;
    CORE_ADDR beginning_addr = 0;
    unsigned int length = 0;
    while(1){
        beginning_addr = read_address_of_arange(aranges_ptr, arange, &bytes_read);
        aranges_ptr += bytes_read;
        length = read_4_bytes(aranges_ptr);
        aranges_ptr += 4;
        if(beginning_addr == 0 && length == 0){
            break;
        }
        num_of_ards ++;
    }
    return num_of_ards;
}

void print_aranges(struct arange **all_aranges, unsigned int num){
    unsigned int i = 0, j = 0;
    for(i = 0; i< num; i++){
        struct arange *arange = all_aranges[i];
        printf("Address Range Header: length = 0x%08x  version = 0x%04x  cu_offset = 0x%08x  addr_size = 0x%02x  seg_size = 0x%02x\n", arange->aranges_header.length, arange->aranges_header.version, arange->aranges_header.info_offset, arange->aranges_header.addr_size, arange->aranges_header.seg_size);
        for (j = 0; j < arange->num_of_ards; j++){
            printf("0x%08x + 0x%08x = 0x%08x\n", arange->address_range_descriptors[j].beginning_addr, arange->address_range_descriptors[j].length, arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length);
        }
    }
}

static void create_all_aranges()
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
    //    /* APPLE LOCAL avoid unused var warning.  */
    //    /* char *beg_of_comp_unit; */
    //    struct dwarf2_per_cu_data *this_cu;
        unsigned long offset;
        int bytes_read;

        offset = aranges_ptr - dwarf2_per_objfile->aranges_buffer;

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
        aranges_ptr += 4;
        unsigned int num_of_ards = get_num_arange_descriptor(aranges_ptr, arange);
        arange->num_of_ards = num_of_ards;
        //printf("num_of_ards: %d\n", num_of_ards);

        arange->address_range_descriptors = malloc(num_of_ards * sizeof(struct address_range_descriptor));
        assert(arange->address_range_descriptors != NULL);
        memset(arange->address_range_descriptors, 0, num_of_ards * sizeof(struct address_range_descriptor));
        struct address_range_descriptor *address_range_descriptors;
        //
        //struct address_range_descriptor ard
        CORE_ADDR beginning_addr = 0;
        unsigned int length = 0;
        int i = 0;
        for(i = 0; i < num_of_ards; i++){
            arange->address_range_descriptors[i].beginning_addr = read_address_of_arange(aranges_ptr, arange, &bytes_read);
            aranges_ptr += bytes_read;

            arange->address_range_descriptors[i].length = read_4_bytes(aranges_ptr);
            aranges_ptr += 4;

        //    printf("beginning_addr: 0X%X\t", arange->address_range_descriptors[i].beginning_addr);
        //    printf("length: 0X%X\n", arange->address_range_descriptors[i].length);
        }
        //skip ending zeros
        aranges_ptr += 8;
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
}



void parse_dwarf_aranges(){
    create_all_aranges();
    //print_aranges(dwarf2_per_objfile->all_aranges, dwarf2_per_objfile->n_aranges);
//    struct aranges_header;
}

int is_target_subprogram(struct die_info *die, struct address_range_descriptor *target_ard){
    int flag = 0;
    unsigned int i = 0;
    for(i = 0; i< die->num_attrs; i++){
        if(die->attrs[i].name == DW_AT_low_pc && die->attrs[i].u.addr == target_ard->beginning_addr){
            flag++;
        }

        if(die->attrs[i].name == DW_AT_high_pc && die->attrs[i].u.addr == (target_ard->beginning_addr + target_ard->length)){
            flag++;
        }

        if(flag == 2){
            return 1;
        }
    }

    return -1;

}

struct die_info *find_target_subprogram(struct die_info *die, struct address_range_descriptor *target_ard){
    while(die != NULL){
        if(die->tag == DW_TAG_subprogram){
            if(is_target_subprogram(die, target_ard) == 1){
                return die;
            }
        }
        
        if(die->sibling != NULL){
            return find_target_subprogram(die->sibling, target_ard); 
        }

        if(die->child != NULL){
            return find_target_subprogram(die->child, target_ard); 
        }
    }
    return NULL;

}
char *get_name_attribute(struct die_info *die){
    unsigned int i = 0;
    for(i = 0; i < die->num_attrs; i++){
        if (die->attrs[i].name == DW_AT_name){
            return die->attrs[i].u.str;
        }
    }
    return NULL;

}

void lookup_by_address(CORE_ADDR address){
    unsigned int num = dwarf2_per_objfile->n_aranges;
    struct arange **all_aranges = dwarf2_per_objfile->all_aranges; 
    struct arange *target_arange = NULL;
    struct address_range_descriptor *target_ard;
    unsigned int i = 0, j = 0;
    for(i = 0; i< num; i++){
        struct arange *arange = all_aranges[i];
        for(j = 0; j < arange->num_of_ards; j++){
            CORE_ADDR beginning_addr = arange->address_range_descriptors[j].beginning_addr;
            CORE_ADDR ending_addr = arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length;
            if (address >= beginning_addr && address < ending_addr){
                target_arange = arange;
                target_ard = &arange->address_range_descriptors[j];
                break;
            }
        }
    }
    struct arange *arange = target_arange;
    //printf("Address Range Header: length = 0x%08x  version = 0x%04x  cu_offset = 0x%08x  addr_size = 0x%02x  seg_size = 0x%02x\n", arange->aranges_header.length, arange->aranges_header.version, arange->aranges_header.info_offset, arange->aranges_header.addr_size, arange->aranges_header.seg_size);
    for (j = 0; j < arange->num_of_ards; j++){
        //printf("0x%08x + 0x%08x = 0x%08x\n", arange->address_range_descriptors[j].beginning_addr, arange->address_range_descriptors[j].length, arange->address_range_descriptors[j].beginning_addr + arange->address_range_descriptors[j].length);
    }

    //find the target compilation unit 
    struct dwarf2_per_cu_data *target_dwarf2_per_cu_data= NULL;
    for (i = 0; i < dwarf2_per_objfile->n_comp_units; i++){
        if (dwarf2_per_objfile->all_comp_units[i]->offset == target_arange->aranges_header.info_offset){
            //printf("offset :0x%08lx\tlength: 0x%08lx\n", dwarf2_per_objfile->all_comp_units[i]->offset, dwarf2_per_objfile->all_comp_units[i]->length);
            target_dwarf2_per_cu_data = dwarf2_per_objfile->all_comp_units[i];
            break;
        }
    }
    struct dwarf2_cu *target_cu = target_dwarf2_per_cu_data->cu;
    printf("compilation unit dir: %s\n", target_cu->comp_dir);

    struct die_info *target_die = find_target_subprogram(target_cu->dies, target_ard);
    char *target_program_full_name = get_name_attribute(target_cu->dies);
    char *target_program_name = strrchr(target_program_full_name, '/');
    if(target_program_name == NULL){
        target_program_name = target_program_full_name;
    }else{
        target_program_name = target_program_name +1;
    }

    char *target_subprogram_name = get_name_attribute(target_die);
    printf("target_program_full_name: %s\n", target_program_full_name);
    printf("target_program_name: %s\n", target_program_name);
    printf("target_subprogram_name: %s\n", target_subprogram_name);
}

int parse_dwarf2_per_objfile(){
    //printf("Parsing Abbrev\n");
    parse_dwarf_abbrev();
    //printf("Parsing Info\n");
    parse_dwarf_info();
    parse_dwarf_aranges();

    lookup_by_address(0x0000a87b);
}

int parse_load_command(FILE *fp, struct load_command *lcp){
    long offset = 0L - sizeof(struct load_command);
//    printf("Command: %u\n", lcp->cmd);
    switch (lcp->cmd){
        case LC_UUID: 
            //printf("load command type: %s\n", "LC_UUID");
            process_lc_uuid(fp, offset);
            break;
        case LC_SEGMENT: 
            //printf("load command type: %s\n", "LC_SEGMENT");
            process_lc_segment(fp, offset);
            break;
        case LC_SEGMENT_64: 
            //printf("load command type: %s\n", "LC_SEGMENT_64");
            process_lc_segment_64(fp, offset);
            break;
        case LC_SYMTAB:
            //printf("load command type: %s\n", "LC_SYMTAB");
            process_lc_symtab(fp, offset);
            break;
        case LC_DYSYMTAB:
            //printf("load command type: %s\n", "LC_DYSYMTAB");
            process_lc_dysymtab(fp, offset);
            break;
        case LC_THREAD:
            //printf("load command type: %s\n", "LC_THREAD");
            process_lc_thread(fp, offset);
            break;
        case LC_UNIXTHREAD:
            //printf("load command type: %s\n", "LC_UNIXTHREAD");
            process_lc_unixthread(fp, offset);
            break;
        case LC_LOAD_DYLIB:
            //printf("load command type: %s\n", "LC_LOAD_DYLIB");
            process_lc_load_dylib(fp, offset);
            break;
        case LC_ID_DYLIB:
            //printf("load command type: %s\n", "LC_ID_DYLIB");
            process_lc_id_dylib(fp, offset);
            break;
        case LC_PREBOUND_DYLIB:
            //printf("load command type: %s\n", "LC_PREBOUND_DYLIB");
            process_lc_prebound_dylib(fp, offset);
            break;
        case LC_LOAD_DYLINKER:
            //printf("load command type: %s\n", "LC_LOAD_DYLINKER");
            process_lc_load_dylinker(fp, offset);
            break;
        case LC_ID_DYLINKER:
            //printf("load command type: %s\n", "LC_ID_DYLINKER");
            process_lc_id_dylinker(fp, offset);
            break;
        case LC_ROUTINES:
            //printf("load command type: %s\n", "LC_ROUTINES");
            process_lc_routines(fp, offset);
            break;
        case LC_ROUTINES_64:
            //printf("load command type: %s\n", "LC_ROUTINES_64");
            process_lc_routines_64(fp, offset);
            break;
        case LC_TWOLEVEL_HINTS:
            //printf("load command type: %s\n", "LC_TWOLEVEL_HINTS");
            process_lc_twolevel_hints(fp, offset);
            break;
        case LC_SUB_FRAMEWORK:
            //printf("load command type: %s\n", "LC_SUB_FRAMEWORK");
            process_lc_sub_framework(fp, offset);
            break;
        case LC_SUB_UMBRELLA:
            //printf("load command type: %s\n", "LC_SUB_UMBRELLA");
            process_lc_sub_umbrella(fp, offset);
            break;
        case LC_SUB_LIBRARY:
            //printf("load command type: %s\n", "LC_SUB_LIBRARY");
            process_lc_sub_library(fp, offset);
            break;
        case LC_SUB_CLIENT:
            //printf("load command type: %s\n", "LC_SUB_CLIENT");
            process_lc_sub_client(fp, offset);
            break;
        case 41:
            //printf("load command type: %s\n", "LC_DATA_IN_CODE");
            process_lc_data_in_code(fp, offset);
            break;
        case 38:
            //printf("load command type: %s\n", "LC_FUNCTION_STARTS");
            process_lc_function_starts(fp, offset);
            break;
        default:
            printf("load commmand type unknown\n");

    }
}

int process_lc_data_in_code(FILE *fp, long offset){
//    printf("parsing LC_DATA_IN_CODE\n");
    struct lc_data_in_code command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct lc_data_in_code), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct lc_data_in_code)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}

int process_lc_function_starts(FILE *fp, long offset){
//    printf("parsing LC_FUNCTION_STARTS\n");
    struct lc_function_starts command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct lc_function_starts), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
//    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct lc_function_starts)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;

}

int process_lc_uuid(FILE *fp, long offset){
//    printf("parsing LC_UUID\n");
    struct uuid_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct uuid_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
        int i = 0;
        int numofbytes = sizeof(command.uuid)/sizeof(*command.uuid);
        //printf("numofbytes: %d\n", numofbytes);
        printf("uuid: ");
        for (; i < numofbytes; i++){
            printf("%02X", command.uuid[i]);
        }
        printf("\n");
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct uuid_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0; 
}

int process_lc_segment(FILE *fp, long offset){
//    printf("parsing LC_SEGMENT\n");
    struct segment_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct segment_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
        //printf("segname: %s\n", command.segname);
        //printf("vmaddr: 0x%x\n", command.vmaddr);
        if(strcmp(command.segname, "__TEXT") == 0){
            doi.text_vmaddr = command.vmaddr;
        }
        if(strcmp(command.segname, "__DWARF") == 0){
            //printf("__DWARF File\n");
            parse_dwarf_segment(fp, &command);
            //todo seek_back
            return 0;
        }
    }
    //printf("\n");
    //in case there are sections, we need to seek the file point to the next load command
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct segment_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0; 
}

int process_lc_sub_client(FILE *fp, long offset){
    return 0;
}

int process_lc_sub_library(FILE *fp, long offset){
    return 0;

}

int process_lc_sub_umbrella(FILE *fp, long offset){
    return 0;

}
int process_lc_sub_framework(FILE *fp, long offset){
    return 0;

}
int process_lc_twolevel_hints(FILE *fp, long offset){
    return 0;

}
int process_lc_routines_64(FILE *fp, long offset){
    return 0;

}
int process_lc_routines(FILE *fp, long offset){
    return 0;

}

int process_lc_id_dylinker(FILE *fp, long offset){
//    printf("parsing LC_ID_DYLINKER\n");
    struct dylinker_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dylinker_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dylinker_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;

}
int process_lc_load_dylinker(FILE *fp, long offset){
//    printf("parsing LC_LOAD_DYLINKER\n");
    struct dylinker_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dylinker_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
        //printf("dylinker_command: %u\n", sizeof(struct dylinker_command));
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dylinker_command)), SEEK_CUR);
    assert(seekreturn == 0);

    return 0;

}
int process_lc_prebound_dylib(FILE *fp, long offset){
    return 0;

}
int process_lc_id_dylib(FILE *fp, long offset){
    return 0;

}
int process_lc_load_dylib(FILE *fp, long offset){
//    printf("parsing LC_LOAD_DYLIB\n");
    struct dylib_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dylib_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dylib_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}

int process_lc_thread(FILE *fp, long offset){
//    printf("parsing LC_THREAD\n");
    struct thread_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct thread_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct thread_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;

}

int process_lc_unixthread(FILE *fp, long offset){
//    printf("parsing LC_UNIXTHREAD\n");
    struct thread_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct thread_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct thread_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}
int process_lc_dysymtab(FILE *fp, long offset){
//    printf("parsing LC_DYSYMTAB\n");
    struct dysymtab_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dysymtab_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dysymtab_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}
int process_lc_symtab(FILE *fp, long offset){
//    printf("parsing LC_SYMTAB\n");
    struct symtab_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct symtab_command), 1, fp)) != 0 )
    {
        //printf("record count: %d\n", rc);
        //printf("cmd: %u\n", command.cmd);
        //printf("cmdsize: %u\n", command.cmdsize);
        //printf("nsyms: %u\n", command.nsyms);
    }
    //printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct symtab_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}
int process_lc_segment_64(FILE *fp, long offse){
    return 0;

}

