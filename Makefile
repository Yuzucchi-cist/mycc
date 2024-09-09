CFLAGS=-std=c11 -g -static

mycc: mycc.c

db_mycc: mycc.c
	cc -g -O0 mycc.c -o db_mycc

test: mycc
	./test.sh

debug: db_mycc
	gdb ./db_mycc

clean:
	rm -f mycc *.o *~ tmp* db_mycc

.PHONY: test debug clean
