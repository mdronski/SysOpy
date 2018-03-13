
all: O O0 O1 O2 O3 Os  tests

lib:
	gcc -Wall -c blockArray.c -o blockArray.o
	ar rcs libblockArray.a blockArray.o
	gcc -Wall -c -fPIC blockArray.c -o blockArray.o
	gcc -shared blockArray.o -o libblockArray.so


O: lib
	gcc -O -Wall -std=c11  main.c  libblockArray.a -o staticMainO
	gcc -O main.c -Wl,-rpath=. -Wall -L . -lblockArray -o sharedMainO
	gcc -O -Wall -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO -ldl -D DLL

O0:
	gcc -O0 -Wall -std=c11  main.c  libblockArray.a -o staticMainO0
	gcc -O0 main.c -Wl,-rpath=. -Wall -L . -lblockArray -o sharedMainO0
	gcc -O0 -Wall -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO0 -ldl -D DLL

O1:
	gcc -O1 -Wall -std=c11  main.c  libblockArray.a -o staticMainO1
	gcc -O1 main.c -Wl,-rpath=. -Wall -L . -lblockArray -o sharedMainO1
	gcc -O1 -Wall -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO1 -ldl -D DLL

O2:
	gcc -O2 -Wall -std=c11  main.c  libblockArray.a -o staticMainO2
	gcc -O2 main.c -Wl,-rpath=. -Wall -L . -lblockArray -o sharedMainO2
	gcc -O2 -Wall -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO2 -ldl -D DLL

O3:
	gcc -O3 -Wall -std=c11  main.c  libblockArray.a -o staticMainO3
	gcc -O3 main.c -Wl,-rpath=. -Wall -L . -lblockArray -o sharedMainO3
	gcc -O3 -Wall -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainO3 -ldl -D DLL

Os:
	gcc -Os -Wall -std=c11  main.c  libblockArray.a -o staticMainOs
	gcc -Os main.c -Wl,-rpath=. -Wall -L . -lblockArray -o sharedMainOs
	gcc -Os -Wall -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMainOs -ldl -D DLL


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
