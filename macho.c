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
 *         Author:  Reno Qiu
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

/*  This data structure holds the information of an abbrev. */
struct abbrev_info
{
    unsigned int number;    /*  number identifying abbrev */
    enum dwarf_tag tag;     /*  dwarf tag */
    unsigned short has_children;        /*  boolean */
    unsigned short num_attrs;   /*  number of attributes */
    struct attr_abbrev *attrs;  /*  an array of attribute descriptions */
    struct abbrev_info *next;   /*  next in chain */
};

struct attr_abbrev
{
    enum dwarf_attribute name; 
    enum dwarf_form form; 
};

//struct data_of_interest{
//    uint32_t text_vmaddr;
//};
//
struct data_of_interest doi = {0};

struct section *dwarf_section_headers = NULL;
struct abbrev_info *dwarf_abbrevs = NULL;
struct dwarf_sections{
    unsigned char *dwarf_info_section;
    unsigned char *dwarf_abbrev_section;
    unsigned char *dwarf_line_section;
    unsigned char *dwarf_pubnames_section;
    unsigned char *dwarf_aranges_section;
    unsigned char *dwarf_loc_section;
    unsigned char *dwarf_macinfo_section;
    unsigned char *dwarf_str_section;
    unsigned char *dwarf_ranges_section;
    unsigned char *dwarf_inlined_section;
    unsigned char *dwarf_pubtypes_section;
    unsigned char *dwarf_frame_section;
    unsigned char *dwarf_eh_frame_section;

    int dwarf_info_section_size;
    int dwarf_abbrev_section_size;
    int dwarf_line_section_size;
    int dwarf_pubnames_section_size;
    int dwarf_aranges_section_size;
    int dwarf_loc_section_size;
    int dwarf_macinfo_section_size;
    int dwarf_str_section_size;
    int dwarf_ranges_section_size;
    int dwarf_inlined_section_size;
    int dwarf_pubtypes_section_size;
    int dwarf_frame_section_size;
    int dwarf_eh_frame_section_size;
};
struct dwarf_sections dwarf_sections = {0};
uint32_t numofdwarfsections = 0;
void free_dwarf_sections(){
    //uint32_t i = 0;
    //while(i < numofdwarfsections){
    //    free(dwarf_sections[i]);
    //    i++;
    //}
    if(dwarf_sections.dwarf_info_section){
        free(dwarf_sections.dwarf_info_section); 
    }
    if(dwarf_sections.dwarf_abbrev_section){
        free(dwarf_sections.dwarf_abbrev_section); 
    }
    if(dwarf_sections.dwarf_line_section){
        free(dwarf_sections.dwarf_line_section); 
    }
    if(dwarf_sections.dwarf_pubnames_section){
        free(dwarf_sections.dwarf_pubnames_section); 
    }
    if(dwarf_sections.dwarf_aranges_section){
        free(dwarf_sections.dwarf_aranges_section); 
    }
    if(dwarf_sections.dwarf_loc_section){
        free(dwarf_sections.dwarf_loc_section); 
    }
    if(dwarf_sections.dwarf_macinfo_section){
        free(dwarf_sections.dwarf_macinfo_section); 
    }
    if(dwarf_sections.dwarf_str_section){
        free(dwarf_sections.dwarf_str_section); 
    }
    if(dwarf_sections.dwarf_ranges_section){
        free(dwarf_sections.dwarf_ranges_section); 
    }
    if(dwarf_sections.dwarf_inlined_section){
        free(dwarf_sections.dwarf_inlined_section); 
    }
    if(dwarf_sections.dwarf_pubtypes_section){
        free(dwarf_sections.dwarf_pubtypes_section); 
    }
    if(dwarf_sections.dwarf_frame_section){
        free(dwarf_sections.dwarf_frame_section); 
    }
    if(dwarf_sections.dwarf_eh_frame_section){
        free(dwarf_sections.dwarf_eh_frame_section); 
    }

    free(dwarf_section_headers);
    //todo
    //free();
}
int parse_dwarf_segment(FILE* fp, struct segment_command *command){
    numofdwarfsections = command->nsects;
    dwarf_section_headers = malloc(numofdwarfsections * sizeof(struct section));
    memset(dwarf_section_headers, '\0', numofdwarfsections * sizeof (struct section));
    int rc = 0;
    uint32_t i = 0;
    long current_pos = ftell (fp);
    while(i < numofdwarfsections){
        if( (rc = fread(&(dwarf_section_headers[i]) ,sizeof(struct section), 1, fp)) != 0 ){
            printf("%s %s\n", dwarf_section_headers[i].segname, dwarf_section_headers[i].sectname); 
            //dwarf_sections.dwarf_info_section
            unsigned char *temp = malloc(dwarf_section_headers[i].size);
            if (temp == NULL){
                printf("Malloc Error!\n");
                return -1;
            }
            //memset(dwarf_sections.dwarf_info_section, '\0', dwarf_section_headers[i].size);
            memset(temp, '\0', dwarf_section_headers[i].size);

            long temp_position = ftell(fp);
            fseek(fp, dwarf_section_headers[i].offset, SEEK_SET);
            int numofbytes = fread(temp, sizeof(char), dwarf_section_headers[i].size, fp);
            assert(numofbytes == dwarf_section_headers[i].size);

            fseek(fp, temp_position, SEEK_SET);
            printf("SEEKSET: %d\n", SEEK_SET);
            //todo asset

            if(strcmp(dwarf_section_headers[i].sectname, "__debug_abbrev") == 0){ 
                dwarf_sections.dwarf_abbrev_section = temp;
                dwarf_sections.dwarf_abbrev_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_aranges") == 0){
                dwarf_sections.dwarf_aranges_section = temp;
                dwarf_sections.dwarf_aranges_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_info") == 0){
                dwarf_sections.dwarf_info_section = temp;
                dwarf_sections.dwarf_info_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_inlined") == 0){
                dwarf_sections.dwarf_inlined_section = temp;
                dwarf_sections.dwarf_inlined_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_line") == 0){
                dwarf_sections.dwarf_line_section = temp;
                dwarf_sections.dwarf_line_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_loc") == 0){
                dwarf_sections.dwarf_loc_section = temp;
                dwarf_sections.dwarf_loc_section_size = dwarf_section_headers[i].size;
            }else if((strcmp(dwarf_section_headers[i].sectname, "__debug_pubnames") == 0) ||
                    (strcmp(dwarf_section_headers[i].sectname, "__debug_pubnames__DWARF") == 0)
                    ){
                dwarf_sections.dwarf_pubnames_section = temp;
                dwarf_sections.dwarf_pubnames_section_size = dwarf_section_headers[i].size;
            }else if((strcmp(dwarf_section_headers[i].sectname, "__debug_pubtypes") == 0) ||
                    (strcmp(dwarf_section_headers[i].sectname, "__debug_pubtypes__DWARF") == 0) 
                    ){
                dwarf_sections.dwarf_pubtypes_section = temp;
                dwarf_sections.dwarf_pubtypes_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_ranges") == 0){
                dwarf_sections.dwarf_ranges_section = temp;
                dwarf_sections.dwarf_ranges_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__debug_str") == 0){
                dwarf_sections.dwarf_str_section = temp;
                dwarf_sections.dwarf_str_section_size = dwarf_section_headers[i].size;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_names") == 0){
                //dwarf_sections.dwarf_abbrev_section = temp;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_types") == 0){
                //dwarf_sections.dwarf_abbrev_section = temp;
            }else if((strcmp(dwarf_section_headers[i].sectname, "__apple_namespa") == 0) ||
                    (strcmp(dwarf_section_headers[i].sectname, "__apple_namespac__DWARF") == 0) 
                    ){
                //dwarf_sections.dwarf_abbrev_section = temp;
            }else if(strcmp(dwarf_section_headers[i].sectname, "__apple_objc") == 0){
                //dwarf_sections.dwarf_abbrev_section = temp;
            }else{
                printf("╮(╯▽╰)╭, %s \n", dwarf_section_headers[i].sectname);
                free(temp);
            }
        }
        rc = 0;
        i++;
    }
    int seekreturn = 0;
    seekreturn = fseek (fp, current_pos, SEEK_SET); 
    assert(seekreturn == 0);
}

void print_all_dwarf_sections(){
    int i = 0;
    for (i =0; i< dwarf_sections.dwarf_abbrev_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_abbrev_section[i]);
    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_aranges_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_aranges_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_info_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_info_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_inlined_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_inlined_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_line_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_line_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_loc_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_loc_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_pubnames_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_pubnames_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_pubtypes_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_pubtypes_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_ranges_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_ranges_section[i]);

    }
    printf("\n");
    printf("\n");

    for (i =0; i< dwarf_sections.dwarf_str_section_size; i++){
        printf("%02x", dwarf_sections.dwarf_str_section[i]);

    }
    printf("\n");
    printf("\n");
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
        printf("sizeof struct mach_header: %u\n", sizeof(struct mach_header));
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
    //print_all_dwarf_sections();
    parse_dwarf_sections();
    return 0; 
}



signed long long 
decode_signed_leb128(unsigned char* leb128, unsigned long* leb128_length)
{
    signed long long number = 0;
    int sign = 0;
    signed long shift = 0;
    unsigned char byte = *leb128;
    signed long byte_length = 1;

    /*  byte_length being the number of bytes of data absorbed so far in 
     *         turning the leb into a Dwarf_Signed. */

    for (;;) {
        sign = byte & 0x40;
        number |= ((signed long long) ((byte & 0x7f))) << shift;
        shift += 7;

        if ((byte & 0x80) == 0) {
            break;
        }
        ++leb128;
        byte = *leb128;
        byte_length++;
    }

    if ((shift < sizeof(signed long long) * 8) && sign) {
        number |= -((signed long long) 1 << shift);
    }

    if (leb128_length != NULL)
        *leb128_length = byte_length;
    return (number);
}

unsigned long long
decode_unsigned_leb128(unsigned char* leb128, unsigned long* leb128_length)
{
    unsigned char byte;
    unsigned long word_number;
    unsigned long long number;
    signed long shift;
    signed long byte_length;

    /*  The following unrolls-the-loop for the first few bytes and
     *  unpacks into 32 bits to make this as fast as possible.
     *  word_number is assumed big enough that the shift has a defined
     *  result. */
    if ((*leb128 & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 1;
        return (*leb128);
    } else if ((*(leb128 + 1) & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 2;

        word_number = *leb128 & 0x7f;
        word_number |= (*(leb128 + 1) & 0x7f) << 7;
        return (word_number);
    } else if ((*(leb128 + 2) & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 3;

        word_number = *leb128 & 0x7f;
        word_number |= (*(leb128 + 1) & 0x7f) << 7;
        word_number |= (*(leb128 + 2) & 0x7f) << 14;
        return (word_number);
    } else if ((*(leb128 + 3) & 0x80) == 0) {
        if (leb128_length != NULL)
            *leb128_length = 4;

        word_number = *leb128 & 0x7f;
        word_number |= (*(leb128 + 1) & 0x7f) << 7;
        word_number |= (*(leb128 + 2) & 0x7f) << 14;
        word_number |= (*(leb128 + 3) & 0x7f) << 21;
        return (word_number);
    }

    /*  The rest handles long numbers Because the 'number' may be larger 
     *  than the default int/unsigned, we must cast the 'byte' before
     *  the shift for the shift to have a defined result. */
    number = 0;
    shift = 0;
    byte_length = 1;
    byte = *(leb128);
    for (;;) {
        number |= ((unsigned int) (byte & 0x7f)) << shift;

        if ((byte & 0x80) == 0) {
            if (leb128_length != NULL)
                *leb128_length = byte_length;
            return (number);
        }
        shift += 7;

        byte_length++;
        ++leb128;
        byte = *leb128;
    }
}
unsigned int get_num_attr_spec_pair(unsigned char* current_abbrev_pos){
    unsigned long length = 0;
    unsigned int num_attr_spec_pair = 0;
    unsigned int attr_name_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos, &length);
    current_abbrev_pos += length;
    unsigned int attr_form_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos, &length);
    current_abbrev_pos += length;
    while(attr_name_code != 0 || attr_form_code != 0){
        attr_name_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos ,&length);
        current_abbrev_pos += length;
        attr_form_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos ,&length);
        current_abbrev_pos += length;
        num_attr_spec_pair ++;
    }
    return num_attr_spec_pair;
}

void free_dwarf_abbrev(){

}

void parse_dwarf_info(){
    unsigned char * current_abbrev_pos = dwarf_sections.dwarf_abbrev_section;
    int size = dwarf_sections.dwarf_abbrev_section_size;
}

void parse_dwarf_abbrev(){
    unsigned char * current_abbrev_pos = dwarf_sections.dwarf_abbrev_section;
    int size = dwarf_sections.dwarf_abbrev_section_size;
    unsigned char* endof_abbrev_pos = current_abbrev_pos + size;
    dwarf_abbrevs = NULL;
    struct abbrev_info *prev = NULL;
    int i = 0;
    while(current_abbrev_pos < endof_abbrev_pos && *current_abbrev_pos != '\0'){
        unsigned char temp = 0;
        unsigned long length = 0;
        unsigned long long abbrev_code = decode_unsigned_leb128(current_abbrev_pos, &length);
        //printf("%llu %lu\n", abbrev_code, length);
        current_abbrev_pos += length;
        unsigned long long entry_code = decode_unsigned_leb128(current_abbrev_pos, &length);
        //printf("%llu\n", entry_code);
        current_abbrev_pos += length;
        unsigned char has_children = *current_abbrev_pos;
        //printf("%u\n", has_children);
        current_abbrev_pos ++;

        unsigned int num_attr_spec_pair = 0;
        num_attr_spec_pair = get_num_attr_spec_pair(current_abbrev_pos);

        struct abbrev_info *ai = malloc(sizeof(struct abbrev_info));
        memset(ai, '\0', sizeof(struct abbrev_info));
        ai->number = (unsigned int)abbrev_code;
        ai->tag = (unsigned int)entry_code;
        ai->has_children = (unsigned short)has_children;
        ai->num_attrs = (unsigned short)num_attr_spec_pair;
        ai->next = NULL;
        printf("%s\t", dwarf_tag_name(ai->tag));
        printf("num_attr_spec_pair: %d\n", num_attr_spec_pair);
        if (num_attr_spec_pair != 0){
            struct attr_abbrev *attrs = malloc(num_attr_spec_pair * sizeof(struct attr_abbrev));
            memset(attrs, '\0', num_attr_spec_pair * sizeof(struct attr_abbrev));
            unsigned int attr_name_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos, &length);
            current_abbrev_pos += length;
            unsigned int attr_form_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos, &length);
            current_abbrev_pos += length;
            int j = 0;
            while(attr_name_code != 0 || attr_form_code != 0){
                attrs[j].name = attr_name_code;
                attrs[j].form = attr_form_code;
                //printf("%02X ", attr->name);
                //printf("%02X", attr->form);
                //printf("\n");
                printf("%s %s\n", dwarf_attr_name(attrs[j].name), dwarf_form_name(attrs[j].form));
                attr_name_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos ,&length);
                current_abbrev_pos += length;
                attr_form_code = (unsigned int)decode_unsigned_leb128(current_abbrev_pos ,&length);
                current_abbrev_pos += length;
                j++;
            }
            ai->attrs = attrs;
        }else{
            current_abbrev_pos += 2;
            ai->attrs = NULL;
        }
        printf("\n");
        if(prev != NULL){
            prev->next = ai;
        }
        prev = ai;
        if(i == 0){
            dwarf_abbrevs = prev;
        }
        i++;
    }
    //printf("%02X", *current_abbrev_pos);
}

int parse_dwarf_sections(){
    parse_dwarf_abbrev();
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
        if(strcmp(command.segname, "__DWARF") == 0){
            printf("__DWARF File\n");
            parse_dwarf_segment(fp, &command);
            //todo seek_back
            return 0;
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
        printf("dylinker_command: %u\n", sizeof(struct dylinker_command));
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

