all: mush2

mush2: mush2.o
	gcc -g -Wall -L ~pn-cs357/Given/Mush/lib64 -o mush2 mush2.o -lmush
mush2.o: mush2.c
	gcc -g -Wall -I ~pn-cs357/Given/Mush/include -c mush2.c

clean: mush2
	rm -f *.o
