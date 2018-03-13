CC = gcc -Wall -std=c99
all: static shared dynamic tests clean

static:
	$(CC) -c blockArray.c -o blockArray.o
	ar rcs libblockArray.a blockArray.o
	$(CC) -std=c11  main.c  libblockArray.a -o staticMain

shared:
	$(CC) -c -fPIC blockArray.c -o blockArray.o
	$(CC) -shared blockArray.o -o libblockArray.so
	$(CC) main.c -Wl,-rpath=. -L . -lblockArray -o sharedMain

dynamic:
	$(CC) -c -fPIC blockArray.c -o blockArray.o
	$(CC) -shared blockArray.o -o libblockArray.so
	$(CC) -Wl,-rpath=. -L. main.c -lblockArray -o dynamicMain -ldl -D DLL

tests:
	echo > results3a.txt
	echo Static libraries: >> results3a.txt
	./staticMain 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./staticMain 1000000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./staticMain 500000 300 static find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./staticMain 1000000 300 static find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	echo >> results3a.txt
	echo Shared libraries: >> results3a.txt
	./sharedMain 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./sharedMain 1000000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./sharedMain 500000 300 static find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./sharedMain 1000000 300 static find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	echo >> results3a.txt
	echo Dynamic libraries: >> results3a.txt
	./dynamicMain 500000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./dynamicMain 1000000 300 dynamic find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./dynamicMain 500000 300 static find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt
	./dynamicMain 1000000 300 static find 1 deleteAndAdd 250000 alternatelyDelAdd 250000 >> results3a.txt

clean:
	rm -f *.o *.a

clean_exe:
	rm -f *Main*
