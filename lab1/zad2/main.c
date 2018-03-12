#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <stdint-gcc.h>
#include <dlfcn.h>

#ifndef DLL
#include "blockArray.h"
#endif


char* generateRandomString(int maxSize){
    if (maxSize < 1) return NULL;
    char *base = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t baseLength = strlen(base);
    char *newString = (char *) malloc((maxSize) * sizeof(char));
    int newStringLength = maxSize - (rand() % maxSize);

    for (int i = 0; i < newStringLength; ++i) {
        newString[i] = base[rand()%baseLength];
    }
    for (int i = newStringLength; i < maxSize + 1; i++){
        newString[i] = '\0';
    }
    return newString;
}

void fillArray(BlockArray *blockArray){
    for (int i = 0; i < blockArray->size_max; ++i) {
        char* randomString = generateRandomString(blockArray->size_block);
        addBlock(blockArray, i, randomString);
    }
}

void addSpecificNumberOfBlocks(BlockArray *blockArray, int blocksNumber, int startIndex){
    for (int i = 0; i < blocksNumber; ++i) {
        //int index =rand() % blockArray->size_max;
        char *block = generateRandomString(blockArray->size_block);
        addBlock(blockArray, startIndex, block);
        startIndex++;
    }
}

void removeSpecificNumberOfBlocks(BlockArray *blockArray, int blocksNumber, int startIndex){
    for (int i = 0; i < blocksNumber; ++i) {
        //int index =rand() % blockArray->size_max;
        removeBlock(blockArray, startIndex);
        startIndex++;
    }
}

void deleteThenAdd(BlockArray *blockArray, int blocksNumber){
    removeSpecificNumberOfBlocks(blockArray, blocksNumber, 0);
    addSpecificNumberOfBlocks(blockArray, blocksNumber, 0);
}

void alternatelyDeleteAndAdd(BlockArray *blockArray, int blocksNumber){
    for (int i = 0; i < blocksNumber; ++i) {
        removeBlock(blockArray, i);
        addBlock(blockArray, i, generateRandomString(blockArray->size_block));
    }
}

double calculateTime(clock_t start, clock_t end){
    return (double)( end -  start)  / sysconf(_SC_CLK_TCK);
}


void executeCommand(BlockArray *blockArray, char* command, int iterations){
    int commandNumber;
    if (strcmp(command, "find") == 0) {
        commandNumber = 1;
    } else if (strcmp(command, "deleteAndAdd") == 0) {
        commandNumber = 2;
    } else if (strcmp(command, "alternatelyDelAdd") == 0){
        commandNumber = 3;
    } else {
        commandNumber = 333;
    }

    printf("command number: %d\n", commandNumber);
        switch (commandNumber) {
            case 1:
                for (int i = 0; i < iterations; ++i) {
                    findClosestByAscii(blockArray, rand() % blockArray->size_max);
                }
                break;
            case 2:
                deleteThenAdd(blockArray, iterations);
                break;
            case 3:
                alternatelyDeleteAndAdd(blockArray, iterations);
                break;
            default:
                printf("Wrong argument!\n");
                return;
        }

}



int main(int argc, char **argv) {

#ifdef DLL
    void *libraryPtr = dlopen("./blockArray.so", RTLD_LAZY);
    if(!libraryPtr){
        printf("%s\n", "Unable to open library!");
        return 1;
    }
    BlockArray* (*initArray)(int, int, int) = dlsym(handle,"my_library_function");
    void (*deleteArray)(BlockArray*);
    void (*addBlock)(BlockArray*, int, char*);
    void (*removeBlock)(BlockArray*, int);
    char* (*findClosestByAscii)(BlockArray*, int);

#endif

    srand((unsigned int) time(NULL));
    int arraySize = (int) strtol(argv[1], '\0', 10);
    int blockSize = (int) strtol(argv[2], '\0', 10);

    int isDynamic;
    if (strcmp(argv[3], "dynamic") == 0) {
        isDynamic = 1;
    } else if (strcmp(argv[3], "static") == 0){
        isDynamic = 0;
    } else {
        printf("Wrong type of memory allocation! Use \"dynamic\" or \"static\"");
        return 1;
    }

    printf("Array size: %d, Block size: %d, Allocation: %s\n", arraySize, blockSize, argv[3]);

    struct timespec **timespecTime = malloc(sizeof(struct timespec *));
    clock_t *realTime = malloc(5 * sizeof(clock_t));
    struct tms** tmsTime = malloc(5 * sizeof(struct tms*));
    for (int i = 0; i < 5; ++i) {
        realTime[i] = (clock_t) malloc(sizeof(clock_t));
        tmsTime[i] = (struct tms*) malloc(sizeof(struct tms*));
        timespecTime[i] = (struct timespec*) calloc(1, sizeof(struct timespec*));
    }

    realTime[0] = times(tmsTime[0]);

    BlockArray *testArray = initArray(arraySize, blockSize, isDynamic);
    fillArray(testArray);
    realTime[1] = times(tmsTime[1]);

    for (int j = 4; argv[j]; j += 2) {
//        printf("Executing: %s \n", argv[j]);
        executeCommand(testArray, argv[j], (int) strtol(argv[j+1], NULL, 10));
        realTime[j/2] = times(tmsTime[j/2]);
    }

//
//    findClosestByAscii(testArray, rand() % testArray->size_max );
//    realTime[2] = times(tmsTime[2]);
//
//    deleteThenAdd(testArray, testArray->size_max/2);
//    realTime[3] = times(tmsTime[3]);
//
//    alternatelyDeleteAndAdd(testArray, testArray->size_max/2);
//    realTime[4] = times(tmsTime[4]);


    double sum = 0;
    printf("   Real      User      System\n");
        printf("%s: \n", "Allocating");
        printf("%lf   ", calculateTime(realTime[0], realTime[1]));
        printf("%lf   ", calculateTime(tmsTime[0]->tms_utime, tmsTime[1]->tms_utime));
        printf("%lf ", calculateTime(tmsTime[0]->tms_stime, tmsTime[1]->tms_stime));
        sum += calculateTime(realTime[0], realTime[1]);
    printf("\n");

    for (int i = 4; argv[i]; i+=2) {
        printf("%s: \n", argv[i]);
        printf("%lf   ", calculateTime(realTime[i/2-1], realTime[i/2]));
        printf("%lf   ", calculateTime(tmsTime[i/2-1]->tms_utime, tmsTime[i/2]->tms_utime));
        printf("%lf ", calculateTime(tmsTime[i/2-1]->tms_stime, tmsTime[i/2]->tms_stime));
        sum += calculateTime(realTime[i/2-1], realTime[i/2]);
        printf("\n");
    }

    printf("\nŁączny czas pracy prgramu:  %lf\n\n", sum);

    deleteArray(testArray);

#ifdef DLL
    dlclose(libraryPtr);
#endif

    return 0;
}