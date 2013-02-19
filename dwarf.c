/*
 * =====================================================================================
 *
 *       Filename:  Main.c
 *
 *    Description:  DWARF File Reader
 *
 *        Version:  1.0
 *        Created:  19/02/13 10:25:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reno Qiu
 *   Organization:  
 *
 * =====================================================================================
 */

#include <sys/types.h> /*  For open() */
#include <sys/stat.h>  /*  For open() */
#include <fcntl.h>     /*  For open() */
#include <stdlib.h>     /*  For exit() */
#include <unistd.h>     /*  For close() */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "dwarf.h"
#include "libdwarf.h"

void parse_error_handler(Dwarf_Error *error, Dwarf_Ptr errarg){
    printf("Got Error.\n");
}

int main(int argc, char *argv[]){
//    const char *filename = "/home/reno/Downloads/QuincyDemo.app.dSYM/Contents/Resources/DWARF/QuincyDemo";
//    const char *filename = "/home/reno/Documents/macho/a.out";
    const char *filename = argv[1];
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("Read File Error.\n");
    }
    int fd = fileno(fp);
    printf("fd: %d\n", fd);
    Dwarf_Debug dbg = 0;
    Dwarf_Error error = 0;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    int res = DW_DLV_ERROR; //Return Value
    printf("DW_DLV_ERROR: %d\n", DW_DLV_ERROR);
    res = dwarf_init(fd,DW_DLC_READ,errhand,errarg, &dbg,&error);
    printf("res: %d\n", res);
    //rv = dwarf_init(fd, DW_DLC_READ, errhand, errarg, &dbg, &error);
    //res = dwarf_finish(dbg, &error);
    printf("res: %d\n", res);
    printf("Start Reading DWARF File Format\n");
}

