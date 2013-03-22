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

extern struct data_of_interest doi;
char *project_name;

int lookup_by_address(struct thin_macho *thin_macho, CORE_ADDR integer_address){
    int result = -1;
    if(thin_macho->dwarf2_per_objfile != NULL){
        result = lookup_by_address_in_dwarf(thin_macho, integer_address); 
    }

    if(result == -1){
        //look in symtable
        result = lookup_by_address_in_symtable(thin_macho, integer_address);
    }
    return result;
}

void numeric_to_symbols(struct thin_macho *thin_macho, const char **addresses, int numofaddresses){
    char *address_info = NULL;
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
        //TODO Arch
        //printf("usage:  atos [-p pid] [-o executable] [-f file] [-s slide | -l loadAddress] [-arch architecture] [-printHeader] [address ...]");
        //printf("usage:  atos [-o executable] [-arch architecture] [address ...]");
        printf("argc: %d\t\n", argc);
        printf("usage:  atos -arch architecture -o executable [address ...]");
        exit(1);
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
    //printf("Project Name: %s\n", project_name);
    
    int numofaddresses = argc - 5;
    char **numeric_addresses = argv + 5;
    struct target_file *tf = NULL;
    tf = parse_file(full_filename);

    struct thin_macho *thin_macho = NULL;
    //select thin_macho according to the arch
    //performance
    int i = select_thin_macho_by_arch(tf, arch);
    if(i == -1){
        printf("atos: Unknown architecture: %s\n", arch);
    }
    thin_macho = tf->thin_machos[i];
    //print_all_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);

    //dwarf2 file?
    if(thin_macho->dwarf2_per_objfile != NULL){
        parse_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);
    }
    
    //print_thin_macho_aranges(thin_macho);
    
    numeric_to_symbols(thin_macho, (const char **)numeric_addresses, numofaddresses);
    //printf("vmaddr for text segment: 0x%x\n", doi.text_vmaddr);
    //printf("vmaddr_64 for text segment: 0x%llx\n", doi.text_vmaddr_64);
    free_target_file(tf);
}

