all: ex1 ex3

ex1:
	gcc ex1.c ./utils/dlist.c -L./wavelib -lwave -lasound -o ex1 -g -Wall

debugex1:
	insight ./ex1 --args wave_capture1.wav

ex3:
	gcc ex3.c -L./utils -lutils -ldlist -L./wavelib -lwave -lasound -o ex3 -g -Wall

clean:
	rm -rf *.o *.i
