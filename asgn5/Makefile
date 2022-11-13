CC = gcc

CFLAGS = -pedantic -Wall -Werror

all: mytalk

mytalk: mytalk.o
	$(CC) $(CFLAGS) -o mytalk mytalk.o

mytalk.o: mytalk.c
	$(CC) $(CFLAGS) -c mytalk.c

clean: mytalk
	rm -f *.o
