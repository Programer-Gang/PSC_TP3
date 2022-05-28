all: ex1 ex3

ex1:
	gcc ex1.c -L./utils -lutils -ldlist -o ex1 -Wall

ex3:
	gcc ex3.c -L./utils -lutils -ldlist -L./wavelib -lwave -lasound -o ex3 -Wall

clean:
	rm -rf *.o *.i
