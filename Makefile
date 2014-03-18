CC = gcc
CFLAGS = -Wall -c
LDFLAGS = 
OFLAG = -o
LIBS = 
OBJECTS = atosl.o macho.o converter.o
DEBUG_OBJECTS =  atosl_debug.o macho_debug.o converter_debug.o
EXECUTABLE = atosl
DEBUG = debug_atosl

all: release debug

atosl.o: atosl.c macho.h atosl.h
	$(CC) $(CFLAGS) atosl.c $(OFLAG) atosl.o

converter.o: converter.c converter.h
	$(CC) $(CFLAGS) converter.c $(OFLAG) converter.o

macho.o: macho.c macho.h
	$(CC) $(CFLAGS) macho.c $(OFLAG) macho.o

atosl_debug.o: atosl.c macho.h
	$(CC) $(CFLAGS) atosl.c $(OFLAG) atosl_debug.o

converter_debug.o: converter.c converter.h
	$(CC) $(CFLAGS) converter.c $(OFLAG) converter_debug.o

macho_debug.o: macho.c macho.h
	$(CC) $(CFLAGS) macho.c $(OFLAG) macho_debug.o

uuid_reader.o: uuid_reader.c macho.h
	$(CC) $(CFLAGS) uuid_reader.c $(OFLAG) uuid_reader.o

debug: CFLAGS += -g -DDEBUG
debug: $(DEBUG_OBJECTS)
	$(CC) $(OFLAG) $(DEBUG) $(DEBUG_OBJECTS)

release: CFLAGS += -O2
release: $(OBJECTS)
	$(CC) $(OFLAG) $(EXECUTABLE) $(OBJECTS)

uuid: $(OBJECTS) uuid_reader.o
	$(CC) $(OFLAG) uuid_reader macho.o converter.o uuid_reader.o

clean:
	rm -rf $(EXECUTABLE) $(DEBUG) *.o

install:
	cp atosl /usr/local/bin/atosl
	cp atosl /opt/bin/atosl

test:
	sh ./test/test.sh

.PHONY: install clean test

