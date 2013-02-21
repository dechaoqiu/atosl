CC = gcc
objects = mydwarfreader.o dwarf2read.o dwarf2expr.o \
		  dwarf2-fame.o dwarf2loc.o
 
mydwarfreader : $(objects)
	cc -o $(objects)

mydwarfreader.o : mydwarfreader.c
	$(CC) -c mydwarfreader.c

dwarf2read.o : dwarf2read.c dwarf2.h defs.h
	$(CC) -c dwarf2read.c 

dwarf2expr.o : dwarf2expr.c dwarf2expr.h
	$(CC) -c dwarf2expr.c 

dwarf2-frame.o : dwarf2-frame.c dwarf2-frame.h 
	$(CC)  -c dwarf2-frame.c 

dwarf2loc.o : dwarf2loc.c dwarf2loc.h 
	$(CC)  -c dwarf2loc.c 

clean : 
	rm mydwarfreader $(objects)
