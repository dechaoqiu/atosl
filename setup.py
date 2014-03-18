from distutils.core import setup, Extension

module1 = Extension('atos', sources = ["converter.c", "macho.c", "atosl.c", "python_wrapper.c"])

setup (name = 'atosl',
    version = '0.0.1',
    description = 'atosl for linux',
    ext_modules = [module1])

