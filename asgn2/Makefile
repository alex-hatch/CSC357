CC = gcc

CFLAGS = -ansi -pedantic

PROGRAM = fw

fw: fw.o hashtable.o
	$(CC) $(CFLAGS) -o $(PROGRAM) fw.o hashtable.o

all: fw.o hashtable.o
	$(CFLAGS) -o $(PROGRAM) fw.o

fw.o: fw.c
	$(CC) $(CFLAGS) -c fw.c

hashtable.o: hashtable.c
	$(CC) $(CFLAGS) -c hashtable.c

clean: fw
	rm -f *.o