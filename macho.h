#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "loader.h"
#include "fat.h"
#include "language.h"
#include "dwarf2.h"
#include "converter.h"

struct data_of_interest{
    uint32_t text_vmaddr;
};

struct thin_macho{
    char *data;
    long int size;
};

struct target_file{
    struct thin_macho** thin_machos;
    uint32_t numofarchs;
};


int parse_fat_arch(FILE *fp, struct fat_arch *fa, struct thin_macho**thin_macho, uint32_t magic_number);
int parse_universal(FILE *fp, uint32_t magic_number, struct target_file *tf);
int parse_file(const char *filename);
int parse_macho(struct thin_macho*thin_macho);
int process_lc_uuid(char *macho_str, long *offset);
int process_lc_segment(char *macho_str, long *offset);
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
int process_lc_dysymtab(char *macho_str, long *offset);
int process_lc_symtab(char *macho_str, long *offset);
int process_lc_segment_64(char *macho_str, long *offset);
int process_lc_data_in_code(char *macho_str, long *offset);
int process_lc_function_starts(char *macho_str, long *offset);
