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

int parse_fat_arch(FILE *fp, struct fat_arch *fa, char **thin_macho);
int parse_universal(FILE *fp, uint32_t magic_number);
int parse_file(const char *filename);
int parse_macho(FILE *fp);
int process_lc_uuid(FILE *fp, long offset);
int process_lc_segment(FILE *fp, long offset);
int process_lc_sub_client(FILE *fp, long offset);
int process_lc_sub_library(FILE *fp, long offset);
int process_lc_sub_umbrella(FILE *fp, long offset);
int process_lc_sub_framework(FILE *fp, long offset);
int process_lc_twolevel_hints(FILE *fp, long offset);
int process_lc_routines_64(FILE *fp, long offset);
int process_lc_routines(FILE *fp, long offset);
int process_lc_id_dylinker(FILE *fp, long offset);
int process_lc_load_dylinker(FILE *fp, long offset);
int process_lc_prebound_dylib(FILE *fp, long offset);
int process_lc_id_dylib(FILE *fp, long offset);
int process_lc_load_dylib(FILE *fp, long offset);
int process_lc_thread(FILE *fp, long offset);
int process_lc_unixthread(FILE *fp, long offset);
int process_lc_dysymtab(FILE *fp, long offset);
int process_lc_symtab(FILE *fp, long offset);
int process_lc_segment_64(FILE *fp, long offset);
int process_lc_data_in_code(FILE *fp, long offset);
int process_lc_function_starts(FILE *fp, long offset);
