test: create
	
	./main test dane.txt 50000 4 lib >> wyniki.txt
	./main test dane.txt 50000 4 sys >> wyniki.txt
	./main test dane.txt 25000 4 lib >> wyniki.txt
	./main test dane.txt 25000 4 sys >> wyniki.txt
	./main test dane.txt 4000 512 lib >> wyniki.txt
	./main test dane.txt 4000 512 sys >> wyniki.txt
	./main test dane.txt 2000 512 lib >> wyniki.txt
	./main test dane.txt 2000 512 sys >> wyniki.txt
	./main test dane.txt 500 4096 lib >> wyniki.txt
	./main test dane.txt 500 4096 sys >> wyniki.txt
	./main test dane.txt 250 4096 lib >> wyniki.txt
	./main test dane.txt 250 4096 sys >> wyniki.txt
	./main test dane.txt 125 8192 lib >> wyniki.txt
	./main test dane.txt 125 8192 sys >> wyniki.txt
	./main test dane.txt 50 8192 lib >> wyniki.txt
	./main test dane.txt 50 8192 sys >> wyniki.txt

create:
	gcc -Wall -c libusage.c -o libusage.o
	gcc -Wall -c sysusage.c -o sysusage.o
	ar rcs liblibusage.a libusage.o
	ar rcs libsysusage.a sysusage.o
	gcc -Wall -static main.c -L. -l libusage -l sysusage -o main
clean:
	rm main
	rm -f *.o
	rm -f *.a
