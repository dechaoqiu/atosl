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

void numeric_to_symbols(const char **addresses, int numofaddresses){
    char *address_info = NULL;
    int i = 0;
    const char *address = NULL;
    long int integer_address = 0;
    for (i = 0; i < numofaddresses; i++){
        address = addresses[i];
        if(address[0] == '0' && (address[1] == 'x' || address[1] == 'X')){
            //address start with 0x
            integer_address = strtol(address, NULL, 0);
            //printf("0x%lx;\n", integer_address);
        }else{
            integer_address= strtol(address, NULL, 16);
            //printf("0x%lx;\n", integer_address);
        }

        lookup_by_address(integer_address);
        printf("==============================\n");
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
    //char *full_filename = "./CrashTest3";
    //char *full_filename = "./CrashTest2";
    char *full_filename = argv[2];
    printf("full_filename: %s\n", full_filename);
    char *filename = strrchr(full_filename, '/');
    if(filename == NULL){
        filename = full_filename;
    }else{
        filename = filename + 1;
    }
    project_name = filename;
    printf("Project Name: %s\n", project_name);
    
    //get address
    int numofaddresses = argc - 3;
    char **numeric_addresses = argv + 3;
    int rc = 0;
    rc = parse_file(full_filename);
    if (rc = -1){
        printf("parse file error.");
        exit(-1);
    }

    parse_dwarf2_per_objfile();
    numeric_to_symbols((const char **)numeric_addresses, numofaddresses);
    //printf("vmaddr for text segment: 0x%x\n", doi.text_vmaddr);
    free_dwarf2_per_objfile();
}

