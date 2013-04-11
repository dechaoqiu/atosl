#ifndef CONVERTER_H
#define CONVERTER_H

#include "dwarf2.h"

char* dwarf_attr_name(unsigned int attr);
char * dwarf_form_name (unsigned form);
char * dwarf_tag_name (unsigned tag);
#endif /* CONVERTER_H */
