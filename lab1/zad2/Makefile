all: compile

compile:
	gcc -c -Wall blockArray.c
	gcc -c -Wall main.c
	gcc main.o blockArray.o -Wall -o main

dynamic:
	gcc -Wall -c -fPIC blockArray.c -o blockArray.o
	gcc -shared blockArray.o -o libblockArray.so
	gcc -Wall -Wl,-rpath=. -L. main.c -lblockArray -o mainDynamic -ldl -D DLL
