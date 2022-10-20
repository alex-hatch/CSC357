CC = gcc

CFLAGS = -ansi -pedantic -Wall -Werror

PROGRAM = hencode

hencode: hencode.o
	$(CC) $(CFLAGS) -o $(PROGRAM) hencode.o

all: hencode.o
	$(CFLAGS) -o $(PROGRAM) hencode.o

hencode.o: hencode.c
	$(CC) $(CFLAGS) -c hencode.c

clean: hencode
	rm -f *.o
