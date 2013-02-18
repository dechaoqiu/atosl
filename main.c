/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Mach Object Reader
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
#include <stdlib.h>
#include <assert.h>
#include "macho.h"
#include "mach-o/loader.h"

int main(int argc, char *argv[]){
    printf("Start Reading Mach-O File Format\n");
    parse_macho("/Users/Reno/Quincy/local/QuincyDemo_1_0/QuincyDemo2.app/QuincyDemo");
}

int parse_macho(const char *filename){
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("Read File Error.\n");
        return 0;
    }

    printf("Parsing Mach Header\n");

    struct mach_header mh = {0};
    int rc = 0;
    int num_load_cmds = 0;
    if( (rc = fread(&mh ,sizeof(struct mach_header), 1, fp)) != 0 )
    {
        printf("sizeof struct mach_header: %lu\n", sizeof(struct mach_header));
        printf("magic: %d\n", mh.magic);
        printf("cputype: %d\n", mh.cputype);
        printf("cpusubtype: %d\n", mh.cpusubtype);
        printf("filetype: %d\n", mh.filetype);
        printf("MH_EXECUTE: %d\n", MH_EXECUTE);
        ncmds = mh.ncmds;
        printf("ncmds: %d\n", mh.ncmds);
        printf("sizeofcmds: %d\n", mh.sizeofcmds);
        printf("flags: %d\n", mh.flags);
        printf("record count %d\n", rc);
    } 

    printf("Parsing Load Commands\n");
    struct load_command lc = {0}; 
    int i = 0;
    while (i < num_load_cmds){
        if( (rc = fread(&lc ,sizeof(struct load_command), 1, fp)) != 0 )
        {
            printf("record count: %d\n", rc);
            //printf("cmd: %d\n", lc.cmd);
            //printf("cmdsize: %d\n", lc.cmdsize);
            switch (lc.cmd){
                case LC_UUID: 
                    printf("load command type: %s\n", "LC_UUID");
                    break;
                case LC_SEGMENT: 
                    printf("load command type: %s\n", "LC_SEGMENT");
                    process_lc_segment(fp, 0L - sizeof(struct load_command));
                    break;
                case LC_SEGMENT_64: 
                    printf("load command type: %s\n", "LC_SEGMENT_64");
                    break;
                default:
                    printf("load commmand type unknown");

            }
        }
    }
     
    fclose(fp);

}

int process_lc_segment(FILE *fp, long offset){
    printf("parsing LC_SEGMENT\n");
    struct segment_command sc = {0};
    int rc = 0;
    int c = 0;
    c = fseek (fp, offset, SEEK_CUR); 
    assert(c == 0);
    if( (rc = fread(&sc ,sizeof(struct segment_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %d\n", sc.cmd);
        printf("cmdsize: %d\n", sc.cmdsize);
        printf("segname: %s\n", sc.segname);
    }
    printf("\n");
    return 0; 
}
