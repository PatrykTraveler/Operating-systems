opt ?= 0
CC = gcc -Wall -std=c99
all: static shared dynamic tests clean

static:
	$(CC) -c library.c -o library.o
	ar rcs liblibrary.a library.o
	$(CC) -std=c11  main.c  liblibrary.a -o staticMain -O$(opt)

shared:
	$(CC) -c -fPIC library.c -o library.o
	$(CC) -shared library.o -o liblibrary.so
	$(CC) main.c -Wl,-rpath=. -L . -llibrary -o sharedMain -O$(opt)

dynamic:
	$(CC) -c -fPIC library.c -o library.o
	$(CC) -shared library.o -o liblibrary.so
	$(CC) -Wl,-rpath=. -L. main.c -llibrary -o dynamicMain -ldl -D DLL -O$(opt)

tests:
	echo -O$(opt) >> results3b.txt
	echo Static: >> results3b.txt
	./staticMain -s 5000 -b 50 -m 0 -c -f sadasds -r 100 >> results3b.txt
	./staticMain -s 5000 -b 50 -m 1 -c -f sadasds -r 100 >> results3b.txt
	./staticMain -s 10000 -b 50 -m 0 -c -f asdadsa -r 100 >> results3b.txt
	./staticMain -s 10000 -b 50 -m 1 -c -f asdadsa -r 100 >> results3b.txt
	echo >> results3b.txt
	echo Shared: >> results3b.txt
	./sharedMain -s 5000 -b 50 -m 0 -c -f sadasds -r 100 >> results3b.txt
	./sharedMain -s 5000 -b 50 -m 1 -c -f sadasds -r 100 >> results3b.txt
	./sharedMain -s 10000 -b 50 -m 0 -c -f asdadsa -r 100 >> results3b.txt
	./sharedMain -s 10000 -b 50 -m 1 -c -f asdadsa -r 100 >> results3b.txt
	echo >> results3b.txt
	echo Dynamic: >> results3b.txt
	./dynamicMain -s 5000 -b 50 -m 0 -c -f sadasds -r 100 >> results3b.txt
	./dynamicMain -s 5000 -b 50 -m 1 -c -f sadasds -r 100 >> results3b.txt
	./dynamicMain -s 10000 -b 50 -m 0 -c -f asdadsa -r 100 >> results3b.txt
	./dynamicMain -s 10000 -b 50 -m 1 -c -f asdadsa -r 100 >> results3b.txt

clean:
	rm -f *.o *.a

clean_exe:
	rm -f *Main*
