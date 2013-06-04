from distutils.core import setup, Extension

module1 = Extension('atos', sources = ["converter.c", "macho.c", "main.c", "wrapper.c"])

setup (name = 'atos',
    version = '1.0',
    description = 'atos for linux',
    ext_modules = [module1])

