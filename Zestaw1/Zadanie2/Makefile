CC=gcc -Wall -std=c99

all: compile tests clean

compile:
	$(CC) -c library.c
	$(CC) -c main.c
	$(CC) main.o library.o -o main

tests:
	./main -s 5000 -b 50 -m 0 -c -f sadasds -r 100 >> raport2.txt
	./main -s 5000 -b 50 -m 1 -c -f sadasds -r 100 >> raport2.txt
	./main -s 10000 -b 50 -m 0 -c -f asdadsa -r 100 >> raport2.txt
	./main -s 10000 -b 50 -m 1 -c -f asdadsa -r 100 >> raport2.txt

clean:
	rm -f *.o
