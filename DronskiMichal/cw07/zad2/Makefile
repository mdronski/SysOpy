all: compile

compile:
	gcc global.c barber.c -pthread -DSO -std=gnu99  -o barber -lrt
	gcc -pthread -DSO -std=gnu99 global.c client.c -o client -lrt
