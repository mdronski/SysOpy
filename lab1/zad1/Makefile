CC = gcc -Wall -std=c99
all: static shared clean

static:
	$(CC) -c blockArray.c
	ar rcs libblockArray.a blockArray.o

shared:
	$(CC) -c -fPIC blockArray.c
	$(CC) -shared -fPIC -o libblockArray.so blockArray.o

clean:
	rm -f *.o
