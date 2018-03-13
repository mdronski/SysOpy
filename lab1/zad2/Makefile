CC = gcc -Wall -std=c99
all: compile tests clean

compile:
	$(CC) -c -Wall blockArray.c
	$(CC) -c -Wall main.c
	$(CC) main.o blockArray.o -o main

tests:
	echo "" > raport2.txt
	./main 1000000 100 dynamic find 1 deleteAndAdd 100000 alternatelyDelAdd 100000 >> raport2.txt
	./main 500000 100 dynamic find 1 deleteAndAdd 100000 alternatelyDelAdd 100000 >> raport2.txt

clean:
	rm -f *.o
