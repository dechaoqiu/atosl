/*
 * =====================================================================================
 *
 *       Filename:  uuid_reader.c
 *
 *    Description
 *
 *        Version:  1.0
 *        Created:  24/03/13 13:44:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <assert.h>
#include "macho.h"

int main(int argc, char *argv[]){
    char *full_filename = argv[1];
    struct target_file *tf = parse_file(full_filename);
    int i = 0;
    while (i < tf->numofarchs){
        char uuid[32] = {0};
        get_uuid_of_thin(tf->thin_machos[i], uuid);
        printf("%s\n", uuid);
        i++;
    }
    return 0;
}
