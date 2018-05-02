create: 
	gcc -Wall master.c -o master
	gcc -Wall slave.c -o slave
	./master ./fifo & 
	./slave ./fifo 10
clean:
	rm master slave

