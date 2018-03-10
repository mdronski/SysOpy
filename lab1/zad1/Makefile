all: static shared clean

static:
	gcc -c -Wall blockArray.c
	ar rcs libblockArray.a blockArray.o

shared:
	gcc -c -Wall -fPIC blockArray.c
	gcc -shared -fPIC -o libblockArray.so blockArray.o

clean:
	rm -f *.o
