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
int main(int argc, char *argv[]){
    //printf("Start Reading Mach-O File Format\n");
    //parse_macho("/Users/Reno/Quincy/local/QuincyDemo_1_0/QuincyDemo2.app/QuincyDemo");
    //parse_macho("/home/reno/Downloads/QuincyDemo2.app/QuincyDemo");
    char *full_filename = "../QuincyDemo";
    char *filename = strrchr(full_filename, '/');
    if(filename == NULL){
        filename = full_filename;
    }else{
        filename = filename + 1;
    }
    project_name = filename;
    printf("Project Name: %s\n", project_name);
    
    //parse_macho("/home/reno/Downloads/QuincyDemo.app.dSYM/Contents/Resources/DWARF/QuincyDemo");
    parse_macho("QuincyDemo");
    //printf("vmaddr for text segment: 0x%x\n", doi.text_vmaddr);
}

