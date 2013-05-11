from distutils.core import setup, Extension

#module1 = Extension('atos', sources = ['wrapper.c'])
module1 = Extension('atos', sources = ["converter.c", "macho.c", "main.c", "wrapper.c"])

setup (name = 'atos',
    version = '1.0',
    description = 'atos for linux',
    ext_modules = [module1])


#"converter.c"
#"macho.c"
#"main.c"
#"wrapper.c"
#"converter.h"
#"cputype.h"
#"debug.h"
#"dwarf2.h"
#"fat.h"
#"loader.h"
#"macho.h"
#"nlist.h"
