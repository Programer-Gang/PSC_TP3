all: libutils.a libdlist.a

libutils.a: utils.o
	ar -rcs libutils.a utils.o

libdlist.a: dlist.o
	ar -rcs libdlist.a dlist.o
	
utils.o: utils.c utils.h
	gcc -c utils.c -o utils.o -Wall

dlist.o: dlist.c dlist.h
	gcc -c dlist.c -o dlist.o -Wall

clean:
	rm -rf *.o *.is