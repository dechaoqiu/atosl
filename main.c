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
    if (argc < 4){
        //TODO Arch
        //printf("usage:  atos [-p pid] [-o executable] [-f file] [-s slide | -l loadAddress] [-arch architecture] [-printHeader] [address ...]");
        //printf("usage:  atos [-o executable] [-arch architecture] [address ...]");
        printf("argc: %d\t\n", argc);
        printf("usage:  atos -o executable [address ...]");
        exit(1);
    }
    assert(strcmp(argv[1], "-o") == 0);

    char *full_filename = argv[2];
    char *filename = strrchr(full_filename, '/');
    if(filename == NULL){
        filename = full_filename;
    }else{
        filename = filename + 1;
    }
    project_name = filename;
    //printf("Project Name: %s\n", project_name);
    
    int numofaddresses = argc - 3;
    char **numeric_addresses = argv + 3;
    struct target_file *tf = NULL;
    tf = parse_file(full_filename);
    //TODO
    struct thin_macho *thin_macho = NULL;
    if(tf->numofarchs == 1){
        thin_macho = tf->thin_machos[0];
    }else{
        thin_macho = tf->thin_machos[0];
    }
    //print_all_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);

    parse_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);
    
    //print_thin_macho_aranges(thin_macho);
    
    numeric_to_symbols(thin_macho, (const char **)numeric_addresses, numofaddresses);
    //printf("vmaddr for text segment: 0x%x\n", doi.text_vmaddr);
    //printf("vmaddr_64 for text segment: 0x%llx\n", doi.text_vmaddr_64);
    free_target_file(tf);
}

