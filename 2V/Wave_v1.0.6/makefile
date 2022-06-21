# -*- MakeFile -*-

#target: dependencies
#	action
CFLAGS = -g -Wall

all: main 

main: main.o
	gcc -o main main.o -L. libwave.so

main.o: main.c
	gcc -c $(CFLAGS) main.c -fpic

clean:
	rm -f *.o wave main
