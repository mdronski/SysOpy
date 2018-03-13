CC = gcc -Wall -std=c99
all: O O0 O1 O2 O3 Os tests clean

lib:
	$(CC) -c blockArray.c -o blockArray.o
	ar rcs libblockArray.a blockArray.o
	$(CC) -c -fPIC blockArray.c -o blockArray.o
	$(CC) -shared blockArray.o -o libblockArray.so


O: lib
	$(CC) -O main.c  libblockArray.a -o staticMainO
	$(CC) -O main.c -Wl,-rpath=. -L . -lblockArray -o sharedMainO
	$(CC) -O -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO -ldl -D DLL

O0:
	$(CC) -O0 main.c  libblockArray.a -o staticMainO0
	$(CC) -O0 main.c -Wl,-rpath=. -L . -lblockArray -o sharedMainO0
	$(CC) -O0 -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO0 -ldl -D DLL

O1:
	$(CC) -O1 main.c  libblockArray.a -o staticMainO1
	$(CC) -O1 main.c -Wl,-rpath=. -L . -lblockArray -o sharedMainO1
	$(CC) -O1 -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO1 -ldl -D DLL

O2:
	$(CC) -O2 main.c  libblockArray.a -o staticMainO2
	$(CC) -O2 main.c -Wl,-rpath=. -L . -lblockArray -o sharedMainO2
	$(CC) -O2 -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO2 -ldl -D DLL

O3:
	$(CC) -O3 main.c  libblockArray.a -o staticMainO3
	$(CC) -O3 main.c -Wl,-rpath=. -L . -lblockArray -o sharedMainO3
	$(CC) -O3 -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO3 -ldl -D DLL

Os:
	$(CC) -Os main.c  libblockArray.a -o staticMainOs
	$(CC) -Os main.c -Wl,-rpath=. -L . -lblockArray -o sharedMainOs
	$(CC) -Os -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainOs -ldl -D DLL


tests:
	echo > results3b.txt
	echo Static libraries: >> results3b.txt
	./staticMainO0 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./staticMainO1 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./staticMainO2 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./staticMainO3 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./staticMainOs 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	echo Shared libraries: >> results3b.txt
	./sharedMainO0 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./sharedMainO1 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./sharedMainO2 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./sharedMainO3 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./sharedMainOs 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	echo Dynamic libraries: >> results3b.txt
	./dynamicMainO0 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./dynamicMainO1 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./dynamicMainO2 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./dynamicMainO3 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt
	./dynamicMainOs 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3b.txt

clean:
	rm -f *.o
