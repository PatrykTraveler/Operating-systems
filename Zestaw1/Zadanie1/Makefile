CC=gcc
CFLAGS=-c -Wall

all: static shared clean

static:
	$(CC) $(CFLAGS) library.c
	ar rcs liblibrary.a library.o

shared:
	$(CC) $(CFLAGS) -fPIC library.c
	$(CC) -shared -fPIC -o liblibrary.so library.o

clean:
	rm -f *.o
