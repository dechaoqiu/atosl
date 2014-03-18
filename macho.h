#ifndef MACHO_H
#define MACHO_H

#ifdef Py_PYTHON_H
#include "python_wrapper.h"
#else
#define ATOSLError 0
#define PyErr_NoMemory()
#define PyErr_Format(stream, format,...)
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "loader.h"
#include "fat.h"
#include "dwarf2.h"
#include "cputype.h"
#include "converter.h"
#include "nlist.h"
#include "debug.h"

/* We hold several abbreviation tables in memory at the same time. */
#ifndef ABBREV_HASH_SIZE
#define ABBREV_HASH_SIZE 121
#endif
#define INITIAL_LINE_VECTOR_LENGTH  1000


/* Languages represented in the symbol table and elsewhere.
*/

enum language
{
    language_unknown,		/* Language not known */
    language_auto,		/* Placeholder for automatic setting */
    language_c,			/* C */
    language_cplus,		/* C++ */
    language_objc,		/* Objective-C */
    /* APPLE LOCAL objcplus */
    language_objcplus,		/* Objective-C++ */
    language_java,		/* Java */
    language_fortran,		/* Fortran */
    language_m2,		/* Modula-2 */
    language_asm,		/* Assembly language */
    language_scm,    		/* Scheme / Guile */
    language_pascal,		/* Pascal */
    language_ada,		/* Ada */
    language_minimal,		/* All other languages, minimal support only */
    nr_languages
};


typedef uint64_t CORE_ADDR;
// typedef unsigned int CORE_ADDR;
//struct address_range_descriptor AND CORE_ADDR should be changed for 64bits

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

struct data_of_interest{
    uint32_t text_vmaddr;
    uint64_t text_vmaddr_64;
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

/* Persistent data held for a compilation unit, even when not
   processing it.  We put a pointer to this structure in the
   read_symtab_private field of the psymtab.  If we encounter
   inter-compilation-unit references, we also maintain a sorted
   list of all compilation units.  */

struct dwarf2_per_cu_data
{
    unsigned long offset;
    unsigned long length;
    struct dwarf2_cu *cu;
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

struct address_range_descriptor{
    CORE_ADDR beginning_addr;
    uint64_t length;
    //unsigned int length;
};

struct arange{
    struct aranges_header aranges_header;
    struct address_range_descriptor *address_range_descriptors;
    unsigned int num_of_ards;
};

struct function_range
{
    const char *name;
    CORE_ADDR lowpc, highpc;
    int seen_line;
    struct function_range *next;
};



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
    struct abbrev_info **dwarf2_abbrevs;
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



/* Internal state when decoding a particular compilation unit.  */
struct dwarf2_cu
{
    /* The objfile containing this compilation unit.  */
    //struct objfile *objfile;

    /* The header of the compilation unit.

       FIXME drow/2003-11-10: Some of the things from the comp_unit_head
       should logically be moved to the dwarf2_cu structure.  */
    struct comp_unit_head header;

    /* The language we are debugging.  */
    enum language language;
    //const struct language_defn *language_defn;

    const char *producer;

    /* APPLE LOCAL: Retain the compilation directory pathname for header
       file relative pathnames (via gcc parameters like "-I../../../include").  */
    char *comp_dir;

    struct dwarf2_per_objfile *dwarf2_per_objfile;

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

    /* Hash table holding all the loaded partial DIEs.  */
    //htab_t partial_dies;

    /* Storage for things with the same lifetime as this read-in compilation
       unit, including partial DIEs.  */
    //struct obstack comp_unit_obstack;

    /* When multiple dwarf2_cu structures are living in memory, this field
       chains them all together, so that they can be released efficiently.
       We will probably also want a generation counter so that most-recently-used
       compilation units are cached...  */

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

/* .debug_line statement program prologue
   Because of alignment constraints, this structure has padding and cannot
   be mapped directly onto the beginning of the .debug_info section.  */
struct statement_prologue
{
    unsigned int total_length;	/* byte length of the statement
                                   information */
    unsigned short version;	/* version number -- 2 for DWARF
                               version 2 */
    unsigned int prologue_length;	/* # bytes between prologue &
                                       stmt program */
    unsigned char minimum_instruction_length;	/* byte size of
                                                   smallest instr */
    unsigned char default_is_stmt;	/* initial value of is_stmt
                                       register */
    char line_base;
    unsigned char line_range;
    unsigned char opcode_base;	/* number assigned to first special
                                   opcode */
    unsigned char *standard_opcode_lengths;
};

/* The line number information for a compilation unit (found in the
   .debug_line section) begins with a "statement program header",
   which contains the following information.  */
struct line_header
{
    unsigned int total_length;
    unsigned short version;
    unsigned int header_length;
    unsigned char minimum_instruction_length;
    unsigned char default_is_stmt;
    int line_base;
    unsigned char line_range;
    unsigned char opcode_base;

    /* standard_opcode_lengths[i] is the number of operands for the
       standard opcode whose value is i.  This means that
       standard_opcode_lengths[0] is unused, and the last meaningful
       element is standard_opcode_lengths[opcode_base - 1].  */
    unsigned char *standard_opcode_lengths;

    /* The include_directories table.  NOTE!  These strings are not
       allocated with xmalloc; instead, they are pointers into
       debug_line_buffer.  If you try to free them, `free' will get
       indigestion.  */
    unsigned int num_include_dirs, include_dirs_size;
    char **include_dirs;

    /* The file_names table.  NOTE!  These strings are not allocated
       with xmalloc; instead, they are pointers into debug_line_buffer.
       Don't try to free them directly.  */
    unsigned int num_file_names, file_names_size;
    struct file_entry
    {
        char *name;
        unsigned int dir_index;
        unsigned int mod_time;
        unsigned int length;
        int included_p; /* Non-zero if referenced by the Line Number Program.  */
    } *file_names;

    /* The start and end of the statement program following this
       header.  These point into dwarf2_per_objfile->line_buffer.  */
    char *statement_program_start, *statement_program_end;
};

/* Each item represents a line-->pc (or the reverse) mapping.  This is
   somewhat more wasteful of space than one might wish, but since only
   the files which are actually debugged are read in to core, we don't
   waste much space.  */

struct linetable_entry
{
    int line;
    CORE_ADDR pc;
};

/* The order of entries in the linetable is significant.  They should
   be sorted by increasing values of the pc field.  If there is more than
   one entry for a given pc, then I'm not sure what should happen (and
   I not sure whether we currently handle it the best way).

Example: a C for statement generally looks like this

10   0x100   - for the init/test part of a for stmt.
20   0x200
30   0x300
10   0x400   - for the increment part of a for stmt.

If an entry has a line number of zero, it marks the start of a PC
range for which no line number information is available.  It is
acceptable, though wasteful of table space, for such a range to be
zero length.  */

struct linetable
{
    int nitems;
    int lines_are_chars;

    /* Actually NITEMS elements.  If you don't like this use of the
       `struct hack', you can shove it up your ANSI (seriously, if the
       committee tells us how to do it, we can probably go along).  */
    struct linetable_entry item[1];
};

/* The list of sub-source-files within the current individual
   compilation.  Each file gets its own symtab with its own linetable
   and associated info, but they all share one blockvector.  */

struct subfile
{
    //struct subfile *next;
    //char *name;
    //char *dirname;
    struct linetable *line_vector;
    int line_vector_length;
    enum language language;
    //char *debugformat;
};

struct thin_macho{
    uint8_t uuid[16];
    char *data;
    char *strings;
    long int size;
    cpu_type_t	cputype;	/* cpu specifier */
    cpu_subtype_t	cpusubtype;	/* machine specifier */
    struct dwarf2_per_objfile* dwarf2_per_objfile;
    struct nlist *all_symbols;
    struct nlist_64 *all_symbols64;
    uint32_t nsyms;
    uint32_t strsize;
    /* * The binary image's dynamic symbol information, if any. */
    struct {
        /* * Symbol table index for global symbols. */
        uint32_t firstGlobalSymbol;

        /* * Number of global symbols. */
        uint32_t numGlobalSymbols;

        /* * Symbol table index for local symbols. */
        uint32_t firstLocalSymbol;

        /* * Number of local symbols. */
        uint32_t numLocalSymbols;
    } symbolInformation;
};

struct target_file{
    struct thin_macho** thin_machos;
    uint32_t numofarchs;
};
int select_thin_macho_by_arch(struct target_file *tf, const char *arch);
void print_thin_macho_aranges(struct thin_macho *thin_macho);
int parse_load_command(char *macho_str, long *offset, struct load_command *lc, struct thin_macho*tm);
void print_all_dwarf2_per_objfile(struct dwarf2_per_objfile *dwarf2_per_objfile);
void get_uuid_of_thin(struct thin_macho*thin_macho, char *uuid);
void free_target_file(struct target_file *tf);
int lookup_by_address_in_dwarf(struct thin_macho *thin_macho, CORE_ADDR integer_address);
int lookup_by_address_in_symtable(struct thin_macho *thin_macho, CORE_ADDR integer_address);
int parse_fat_arch(FILE *fp, struct fat_arch *fa, struct thin_macho**thin_macho, uint32_t magic_number);
int parse_universal(FILE *fp, uint32_t magic_number, struct target_file *tf);
int parse_normal(FILE *fp, uint32_t magic_number, struct target_file *tf);
struct target_file *parse_file(const char *filename);
int parse_macho(struct thin_macho*thin_macho);
int parse_dwarf2_per_objfile(struct dwarf2_per_objfile *dwarf2_per_objfile);
int process_lc_command(char *macho_str, long *offset);
int process_lc_dyld_info(char *macho_str, long *offset);
int process_lc_dyld_info_only(char *macho_str, long *offset);
int process_lc_version_min_iphoneos(char *macho_str, long *offset);
int process_lc_version_min_macosx(char *macho_str, long *offset);
int process_lc_source_version(char *macho_str, long *offset);
int process_lc_reexport_dylib(char *macho_str, long *offset);
int process_lc_uuid(char *macho_str, long *offset, struct thin_macho*tm);
int process_lc_segment(char *macho_str, long *offset, struct thin_macho*tm);
int process_lc_segment_64(char *macho_str, long *offset, struct thin_macho*tm);
int process_lc_sub_client(char *macho_str, long *offset);
int process_lc_sub_library(char *macho_str, long *offset);
int process_lc_sub_umbrella(char *macho_str, long *offset);
int process_lc_sub_framework(char *macho_str, long *offset);
int process_lc_twolevel_hints(char *macho_str, long *offset);
int process_lc_routines_64(char *macho_str, long *offset);
int process_lc_routines(char *macho_str, long *offset);
int process_lc_id_dylinker(char *macho_str, long *offset);
int process_lc_load_dylinker(char *macho_str, long *offset);
int process_lc_prebound_dylib(char *macho_str, long *offset);
int process_lc_id_dylib(char *macho_str, long *offset);
int process_lc_load_dylib(char *macho_str, long *offset);
int process_lc_thread(char *macho_str, long *offset);
int process_lc_unixthread(char *macho_str, long *offset);
int process_lc_dysymtab(char *macho_str, long *offset, struct thin_macho*tm);
int process_lc_symtab(char *macho_str, long *offset, struct thin_macho*tm);
int process_lc_data_in_code(char *macho_str, long *offset);
int process_lc_function_starts(char *macho_str, long *offset);
int process_lc_symseg(char *macho_str, long *offset);
int process_lc_loadfvmlib(char *macho_str, long *offset);
int process_lc_idfvmlib(char *macho_str, long *offset);
int process_lc_ident(char *macho_str, long *offset);
int process_lc_fvmfile(char *macho_str, long *offset);
int process_lc_prepage(char *macho_str, long *offset);
int process_lc_sub_client(char *macho_str, long *offset);
int process_lc_prebind_cksum(char *macho_str, long *offset);
int process_lc_load_weak_dylib(char *macho_str, long *offset);
int process_lc_rpath(char *macho_str, long *offset);
int process_lc_code_signature(char *macho_str, long *offset);
int process_lc_segment_split_info(char *macho_str, long *offset);
int process_lc_lazy_load_dylib(char *macho_str, long *offset);
int process_lc_encryption_info(char *macho_str, long *offset);
int process_lc_load_upward_dylib(char *macho_str, long *offset);
int process_lc_dyld_environment(char *macho_str, long *offset);
int process_lc_main(char *macho_str, long *offset);
int process_lc_dylib_code_sign_drs(char *macho_str, long *offset);
#endif /*  MACHO_H */
