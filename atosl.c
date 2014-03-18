/*
 * =====================================================================================
 *
 *       Filename:  atosl.c
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
#include <ruby.h>

extern char *project_name;

static int lookup_by_address(struct thin_macho *thin_macho, CORE_ADDR integer_address){
    int result = -1;
    if(thin_macho->dwarf2_per_objfile != NULL){
        result = lookup_by_address_in_dwarf(thin_macho, integer_address);
    }
    if(result == -1){
        result = lookup_by_address_in_symtable(thin_macho, integer_address);
    }
    return result;
}

static void numeric_to_symbols(struct thin_macho *thin_macho, const char **addresses, int numofaddresses){
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

static void set_project_name(const char* full_filename){
    char *filename = strrchr(full_filename, '/');
    if(filename == NULL){
        filename = (char *)full_filename;
    }else{
        filename = filename + 1;
    }
    project_name = filename;
}


int symbolicate(const char* arch, const char *executable, char *addresses[], int numofaddresses){
    debug("in symbolicate arch: %s executable: %s\n", arch, executable);
    set_project_name(executable);
    debug("about to parse file.");
    struct target_file *tf = parse_file(executable);
    if (tf == NULL){
        debug("parse target file error.");
        return -1;
    }
    debug("parse file finished.");

    struct thin_macho *thin_macho = NULL;
    //TODO performance
    int i = select_thin_macho_by_arch(tf, arch);
    if(i == -1){
        printf("atosl: Can not find macho for architecture: %s.\n", arch);
        return -1;
    }
    thin_macho = tf->thin_machos[i];
    //#ifdef DEBUG
    //    print_all_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);
    //#endif

    debug("thin_macho->dwarf2_per_objfile: %p.", thin_macho->dwarf2_per_objfile);
    if(thin_macho->dwarf2_per_objfile != NULL){
        debug("about to parse dwarf2 objfile.");
        parse_dwarf2_per_objfile(thin_macho->dwarf2_per_objfile);
        debug("parse dwarf2 objfile finished.");
    }

    #ifdef DEBUG
        print_thin_macho_aranges(thin_macho);
    #endif

    debug("about to invoke numeric_to_symbols.");
    numeric_to_symbols(thin_macho, (const char **)addresses, numofaddresses);
    free_target_file(tf);
    return 0;
}

// Initial setup function, takes no arguments and returns nothing. Some API
// notes:
//
// * rb_define_module() creates and returns a top-level module by name
//
// * rb_define_module_under() takes a module and a name, and creates a new
//   module within the given one
//
// * rb_define_singleton_method() take a module, the method name, a reference to
//   a C function, and the method's arity, and exposes the C function as a
//   single method on the given module
VALUE Atosl;

VALUE symbolicate_wrapper(VALUE self, VALUE arch, VALUE executable, VALUE addresses){
    int numofaddresses = RARRAY_LEN(addresses);
    char *arch_str = RSTRING_PTR(StringValue(arch));
    char *executable_str = RSTRING_PTR(StringValue(executable));
    char **addresses_array = malloc(numofaddresses * sizeof(char *));
    for (int i = 0; i < numofaddresses; i++){
        VALUE ret = rb_ary_entry(addresses, i);
        addresses_array[i] = RSTRING_PTR(StringValue(ret));
    }
    int result = symbolicate(arch_str, executable_str, addresses_array, numofaddresses);
    free(addresses_array);
    return INT2NUM(result);
}
void Init_atosl(){
    Atosl = rb_define_module("Atosl");
    rb_define_singleton_method(Atosl, "symbolicate", symbolicate_wrapper, 3);
}

int main(int argc, char *argv[]){
    if (argc < 6){
        printf("usage:  atosl -arch architecture -o executable [address ...]\n");
        exit(-1);
    }
    assert(strcmp(argv[1], "-arch") == 0);
    char *arch = argv[2];
    assert(strcmp(argv[3], "-o") == 0);

    char *executable = argv[4];

    int numofaddresses = argc - 5;
    char **addresses = argv + 5;
    int result = symbolicate(arch, executable, addresses, numofaddresses);
    return result;
}

