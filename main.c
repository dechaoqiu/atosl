/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Main Function
 *
 *        Version:  1.0
 *        Created:  02/17/2013 19:00:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reno Qiu
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include "macho.h"

extern char *project_name;

int lookup_by_address(struct thin_macho *thin_macho, CORE_ADDR integer_address){
    int result = -1;
    if(thin_macho->dwarf2_per_objfile != NULL){
        result = lookup_by_address_in_dwarf(thin_macho, integer_address); 
    }
    if(result == -1){
        result = lookup_by_address_in_symtable(thin_macho, integer_address);
    }
    return result;
}

void numeric_to_symbols(struct thin_macho *thin_macho, const char **addresses, int numofaddresses){
    int i = 0;
    const char *address = NULL;
    CORE_ADDR integer_address = 0;

    for (i = 0; i < numofaddresses; i++){
        address = addresses[i];
        if(address[0] == '0' && (address[1] == 'x' || address[1] == 'X')){
            //address start with 0x
            integer_address = strtoll(address, NULL, 0);
        }else{
            integer_address= strtoll(address, NULL, 16);
        }
        
        if (lookup_by_address(thin_macho, integer_address) != 0){
            printf("%s\n", addresses[i]);
        }
    }
}

int main(int argc, char *argv[]){
    if (argc < 6){
        printf("usage:  atos -arch architecture -o executable [address ...]\n");
        exit(-1);
    }
    assert(strcmp(argv[1], "-arch") == 0);

    char *arch = argv[2];

    assert(strcmp(argv[3], "-o") == 0);

    char *full_filename = argv[4];
    char *filename = strrchr(full_filename, '/');
    if(filename == NULL){
        filename = full_filename;
    }else{
        filename = filename + 1;
    }
    project_name = filename;
    
    int numofaddresses = argc - 5;
    char **numeric_addresses = argv + 5;
    struct target_file *tf = NULL;
    debug("about to parse file.");
    tf = parse_file(full_filename);
    debug("parse file finished.");

    struct thin_macho *thin_macho = NULL;
    //performance
    int i = select_thin_macho_by_arch(tf, arch);
    if(i == -1){
        printf("atos: Can not find macho for architecture: %s.\n", arch);
        exit(-1);
    }
    thin_macho = tf->thin_machos[i];
    //#ifdef DEBUG
    //    print_all_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);
    //#endif

    //dwarf2 file?

    debug("thin_macho->dwarf2_per_objfile: %p.", thin_macho->dwarf2_per_objfile);
    if(thin_macho->dwarf2_per_objfile != NULL){
        debug("about to parse dwarf2 objfile.");
        parse_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);
        debug("parse dwarf2 objfile failed.");
    }
    
    #ifdef DEBUG
        print_thin_macho_aranges(thin_macho);
    #endif
    
    debug("about to invoke numeric_to_symbols.");
    numeric_to_symbols(thin_macho, (const char **)numeric_addresses, numofaddresses);
    free_target_file(tf);
    return 0;
}

