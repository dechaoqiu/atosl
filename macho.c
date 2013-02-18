/*
 * =====================================================================================
 *
 *       Filename:  macho.c
 *
 *    Description:  Mach-O Reader
 *
 *        Version:  1.0
 *        Created:  02/17/2013 22:51:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "macho.h"

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

//struct data_of_interest{
//    uint32_t text_vmaddr;
//};
//
struct data_of_interest doi = {0};

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
        printf("magic: %u\n", mh.magic);
        printf("cputype: %u\n", mh.cputype);
        printf("cpusubtype: %u\n", mh.cpusubtype);
        printf("filetype: %u\n", mh.filetype);
        printf("MH_EXECUTE: %u\n", MH_EXECUTE);
        num_load_cmds = mh.ncmds;
        printf("ncmds: %u\n", mh.ncmds);
        printf("sizeofcmds: %u\n", mh.sizeofcmds);
        printf("flags: %u\n", mh.flags);
        printf("record count %d\n", rc);
    } 

    printf("Parsing Load Commands\n");
    struct load_command lc = {0}; 
    int i = 0;
    //num_load_cmds = 3;
    while (i < num_load_cmds){
        printf("Start Reading Load Command %d\n", i);
        //todo try continue when find the data that we are interested with.
        if( (rc = fread(&lc ,sizeof(struct load_command), 1, fp)) != 0 )
        {
            assert(rc == 1);
            printf("record count: %d\n", rc);
            parse_load_command(fp, &lc);
        }else{
            printf("Read Load Command Error\n");
        }
        i++;
    }
     
    fclose(fp);
    return 0; 
}

int parse_load_command(FILE *fp, struct load_command *lcp){
    long offset = 0L - sizeof(struct load_command);
    printf("Command: %u\n", lcp->cmd);
    switch (lcp->cmd){
        case LC_UUID: 
            printf("load command type: %s\n", "LC_UUID");
            process_lc_uuid(fp, offset);
            break;
        case LC_SEGMENT: 
            printf("load command type: %s\n", "LC_SEGMENT");
            process_lc_segment(fp, offset);
            break;
        case LC_SEGMENT_64: 
            printf("load command type: %s\n", "LC_SEGMENT_64");
            process_lc_segment_64(fp, offset);
            break;
        case LC_SYMTAB:
            printf("load command type: %s\n", "LC_SYMTAB");
            process_lc_symtab(fp, offset);
            break;
        case LC_DYSYMTAB:
            printf("load command type: %s\n", "LC_DYSYMTAB");
            process_lc_dysymtab(fp, offset);
            break;
        case LC_THREAD:
            printf("load command type: %s\n", "LC_THREAD");
            process_lc_thread(fp, offset);
            break;
        case LC_UNIXTHREAD:
            printf("load command type: %s\n", "LC_UNIXTHREAD");
            process_lc_unixthread(fp, offset);
            break;
        case LC_LOAD_DYLIB:
            printf("load command type: %s\n", "LC_LOAD_DYLIB");
            process_lc_load_dylib(fp, offset);
            break;
        case LC_ID_DYLIB:
            printf("load command type: %s\n", "LC_ID_DYLIB");
            process_lc_id_dylib(fp, offset);
            break;
        case LC_PREBOUND_DYLIB:
            printf("load command type: %s\n", "LC_PREBOUND_DYLIB");
            process_lc_prebound_dylib(fp, offset);
            break;
        case LC_LOAD_DYLINKER:
            printf("load command type: %s\n", "LC_LOAD_DYLINKER");
            process_lc_load_dylinker(fp, offset);
            break;
        case LC_ID_DYLINKER:
            printf("load command type: %s\n", "LC_ID_DYLINKER");
            process_lc_id_dylinker(fp, offset);
            break;
        case LC_ROUTINES:
            printf("load command type: %s\n", "LC_ROUTINES");
            process_lc_routines(fp, offset);
            break;
        case LC_ROUTINES_64:
            printf("load command type: %s\n", "LC_ROUTINES_64");
            process_lc_routines_64(fp, offset);
            break;
        case LC_TWOLEVEL_HINTS:
            printf("load command type: %s\n", "LC_TWOLEVEL_HINTS");
            process_lc_twolevel_hints(fp, offset);
            break;
        case LC_SUB_FRAMEWORK:
            printf("load command type: %s\n", "LC_SUB_FRAMEWORK");
            process_lc_sub_framework(fp, offset);
            break;
        case LC_SUB_UMBRELLA:
            printf("load command type: %s\n", "LC_SUB_UMBRELLA");
            process_lc_sub_umbrella(fp, offset);
            break;
        case LC_SUB_LIBRARY:
            printf("load command type: %s\n", "LC_SUB_LIBRARY");
            process_lc_sub_library(fp, offset);
            break;
        case LC_SUB_CLIENT:
            printf("load command type: %s\n", "LC_SUB_CLIENT");
            process_lc_sub_client(fp, offset);
            break;
        case 41:
            printf("load command type: %s\n", "LC_DATA_IN_CODE");
            process_lc_data_in_code(fp, offset);
            break;
        case 38:
            printf("load command type: %s\n", "LC_FUNCTION_STARTS");
            process_lc_function_starts(fp, offset);
            break;
        default:
            printf("load commmand type unknown\n");

    }
}

int process_lc_data_in_code(FILE *fp, long offset){
    printf("parsing LC_DATA_IN_CODE\n");
    struct lc_data_in_code command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct lc_data_in_code), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct lc_data_in_code)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}

int process_lc_function_starts(FILE *fp, long offset){
    printf("parsing LC_FUNCTION_STARTS\n");
    struct lc_function_starts command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct lc_function_starts), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct lc_function_starts)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;

}

int process_lc_uuid(FILE *fp, long offset){
    printf("parsing LC_UUID\n");
    struct uuid_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct uuid_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
        int i = 0;
        int numofbytes = sizeof(command.uuid)/sizeof(*command.uuid);
        printf("numofbytes: %d\n", numofbytes);
        printf("uuid: ");
        for (; i < numofbytes; i++){
            printf("%02X", command.uuid[i]);
        }
        printf("\n");
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct uuid_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0; 
}

int process_lc_segment(FILE *fp, long offset){
    printf("parsing LC_SEGMENT\n");
    struct segment_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct segment_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
        printf("segname: %s\n", command.segname);
        printf("vmaddr: 0x%x\n", command.vmaddr);
        if(strcmp(command.segname, "__TEXT") == 0){
            doi.text_vmaddr = command.vmaddr;
        }
    }
    printf("\n");
    //in case there are sections, we need to seek the file point to the next load command
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct segment_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0; 
}

int process_lc_sub_client(FILE *fp, long offset){
    return 0;
}

int process_lc_sub_library(FILE *fp, long offset){
    return 0;

}

int process_lc_sub_umbrella(FILE *fp, long offset){
    return 0;

}
int process_lc_sub_framework(FILE *fp, long offset){
    return 0;

}
int process_lc_twolevel_hints(FILE *fp, long offset){
    return 0;

}
int process_lc_routines_64(FILE *fp, long offset){
    return 0;

}
int process_lc_routines(FILE *fp, long offset){
    return 0;

}

int process_lc_id_dylinker(FILE *fp, long offset){
    printf("parsing LC_ID_DYLINKER\n");
    struct dylinker_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dylinker_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dylinker_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;

}
int process_lc_load_dylinker(FILE *fp, long offset){
    printf("parsing LC_LOAD_DYLINKER\n");
    struct dylinker_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dylinker_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
        printf("dylinker_command: %lu\n", sizeof(struct dylinker_command));
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dylinker_command)), SEEK_CUR);
    assert(seekreturn == 0);

    return 0;

}
int process_lc_prebound_dylib(FILE *fp, long offset){
    return 0;

}
int process_lc_id_dylib(FILE *fp, long offset){
    return 0;

}
int process_lc_load_dylib(FILE *fp, long offset){
    printf("parsing LC_LOAD_DYLIB\n");
    struct dylib_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dylib_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dylib_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}

int process_lc_thread(FILE *fp, long offset){
    printf("parsing LC_THREAD\n");
    struct thread_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct thread_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct thread_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;

}

int process_lc_unixthread(FILE *fp, long offset){
    printf("parsing LC_UNIXTHREAD\n");
    struct thread_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct thread_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct thread_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}
int process_lc_dysymtab(FILE *fp, long offset){
    printf("parsing LC_DYSYMTAB\n");
    struct dysymtab_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct dysymtab_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct dysymtab_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}
int process_lc_symtab(FILE *fp, long offset){
    printf("parsing LC_SYMTAB\n");
    struct symtab_command command = {0};
    int rc = 0;
    int seekreturn = 0;
    seekreturn = fseek (fp, offset, SEEK_CUR); 
    assert(seekreturn == 0);
    if( (rc = fread(&command ,sizeof(struct symtab_command), 1, fp)) != 0 )
    {
        printf("record count: %d\n", rc);
        printf("cmd: %u\n", command.cmd);
        printf("cmdsize: %u\n", command.cmdsize);
        printf("nsyms: %u\n", command.nsyms);
    }
    printf("\n");
    seekreturn = fseek(fp, (command.cmdsize - sizeof(struct symtab_command)), SEEK_CUR);
    assert(seekreturn == 0);
    return 0;
}
int process_lc_segment_64(FILE *fp, long offse){
    return 0;

}

