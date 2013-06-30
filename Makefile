CC = gcc
CFLAGS =  -DNORMAL_COMPILE -Wall -c
LDFLAGS = 
OFLAG = -o
LIBS = 
OBJECTS = main.o macho.o converter.o
DEBUG_OBJECTS =  main_debug.o macho_debug.o converter_debug.o
EXECUTABLE = atos
DEBUG = debug_atos

all: release debug

main.o: main.c macho.h main.h
	$(CC) $(CFLAGS) main.c $(OFLAG) main.o

converter.o: converter.c converter.h
	$(CC) $(CFLAGS) converter.c $(OFLAG) converter.o

macho.o: macho.c macho.h
	$(CC) $(CFLAGS) macho.c $(OFLAG) macho.o

main_debug.o: main.c macho.h
	$(CC) $(CFLAGS) main.c $(OFLAG) main_debug.o

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
	cp atos /usr/local/bin/atos
	#cp atos /opt/bin/atos
	cp atos /home/web/Crab/env/bin/atos

.PHONY: install clean
