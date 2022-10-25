CC = gcc

CFLAGS = -ansi -pedantic -Wall -Werror

all: hdecode hencode

hdecode: hdecode.o
	$(CC) $(CFLAGS) -o hdecode hdecode.o

hencode: hencode.o
	$(CC) $(CFLAGS) -o hencode hencode.o

hencode.o: hencode.c
	$(CC) $(CFLAGS) -c hencode.c

hdecode.o: hdecode.c
	$(CC) $(CFLAGS) -c hdecode.c

clean: hencode hdecode
	rm -f *.o