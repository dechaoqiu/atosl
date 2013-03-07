CC = gcc
CFLAGS = -c
LDFLAGS = 
OFLAG = -o
LIBS = 
OBJECTS = main.o macho.o converter.o
DEBUG_OBJECTS =  main_debug.o macho_debug.o converter_debug.o
EXECUTABLE = atos
DEBUG = debug_atos

all: release debug

main.o: main.c macho.h
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

debug: CFLAGS += -g -DDEBUG
debug: $(DEBUG_OBJECTS)
	$(CC) $(OFLAG) $(DEBUG) $(DEBUG_OBJECTS)

release: $(OBJECTS)
	$(CC) $(OFLAG) $(EXECUTABLE) $(OBJECTS)

clean:
	rm -rf $(EXECUTABLE) $(DEBUG) *.o

