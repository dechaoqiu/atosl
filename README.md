atosl
=====

atosl: atos on linux for converting binary addresses into symbols.

## INSTALL

1. make
2. sudo make install

## USAGE

1. atosl -arch ARCH -o DWARF_FILE_PATH BINARY_ADDRESS

## EXAMPLE

1. atosl -arch ARMV7S -o ~/TEST 0X00001100
