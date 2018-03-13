#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <stdint-gcc.h>
#include <dlfcn.h>
#include <malloc.h>

#ifndef DLL
#include "blockArray.h"
#endif

#ifdef DLL
typedef struct BlockArray{
    char** array;
    int size_max;
    int size_block;
    int isDynamicAllocated;
} BlockArray;
    BlockArray* (*initArray)(int, int, int);
    void (*deleteArray)(BlockArray*);
    void (*addBlock)(BlockArray*, int, char*);
    void (*removeBlock)(BlockArray*, int);
    char* (*findClosestByAscii)(BlockArray*, int);
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
#ifdef DLL
    void *libraryPtr = dlopen("./libblockArray.so", RTLD_LAZY);
    void (*addBlock)(BlockArray*, int, char*) = dlsym(libraryPtr,"addBlock");
#endif

    for (int i = 0; i < blockArray->size_max; ++i) {
        char* randomString = generateRandomString(blockArray->size_block);
        addBlock(blockArray, i, randomString);
    }
}

void addSpecificNumberOfBlocks(BlockArray *blockArray, int blocksNumber, int startIndex){
#ifdef DLL
    void *libraryPtr = dlopen("./libblockArray.so", RTLD_LAZY);
    void (*addBlock)(BlockArray*, int, char*) = dlsym(libraryPtr,"addBlock");
#endif
    for (int i = 0; i < blocksNumber; ++i) {
        //int index =rand() % blockArray->size_max;
        char *block = generateRandomString(blockArray->size_block);
        addBlock(blockArray, startIndex, block);
        startIndex++;
    }
}

void removeSpecificNumberOfBlocks(BlockArray *blockArray, int blocksNumber, int startIndex){
#ifdef DLL
    void *libraryPtr = dlopen("./libblockArray.so", RTLD_LAZY);
    void (*removeBlock)(BlockArray*, int) = dlsym(libraryPtr,"removeBlock");
#endif
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
#ifdef DLL
    void *libraryPtr = dlopen("./libblockArray.so", RTLD_LAZY);
    void (*removeBlock)(BlockArray*, int) = dlsym(libraryPtr,"removeBlock");
    void (*addBlock)(BlockArray*, int, char*) = dlsym(libraryPtr,"addBlock");
#endif
    for (int i = 0; i < blocksNumber; ++i) {
        removeBlock(blockArray, i);
        addBlock(blockArray, i, generateRandomString(blockArray->size_block));
    }
}

double calculateTime(clock_t start, clock_t end){
    return (double)( end -  start)  / sysconf(_SC_CLK_TCK);
}


void executeCommand(BlockArray *blockArray, char* command, int iterations){
#ifdef DLL
    void *libraryPtr = dlopen("./libblockArray.so", RTLD_LAZY);
    char* (*findClosestByAscii)(BlockArray*, int) = dlsym(libraryPtr,"findClosestByAscii");
#endif
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

    switch (commandNumber) {
        case 1:
            findClosestByAscii(blockArray, iterations);
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
    void *libraryPtr = dlopen("./libblockArray.so", RTLD_LAZY);
    if(!libraryPtr){
        printf("%s\n", "Unable to open library!");
        return 1;
    }

    BlockArray* (*initArray)(int, int, int) = dlsym(libraryPtr,"initArray");
    void (*deleteArray)(BlockArray*) = dlsym(libraryPtr,"deleteArray");
#endif

    srand((unsigned int) time(NULL));

    if (argc < 4){
        printf("Wrong arguments! \n");
        return 1;
    }

    int arraySize = (int) strtol(argv[1], '\0', 10);
    int blockSize = (int) strtol(argv[2], '\0', 10);

    if (arraySize <= 0 || blockSize <= 0){
        printf("Wrong array size! \n");
        return 1;
    }

    int isDynamic;
    if (strcmp(argv[3], "dynamic") == 0) {
        isDynamic = 1;
    } else if (strcmp(argv[3], "static") == 0){
        isDynamic = 0;
    } else {
        printf("Wrong type of memory allocation! Use \"dynamic\" or \"static\"");
        return 1;
    }

    printf("Array size: %d, Block size: %d, Allocation: %s", arraySize, blockSize, argv[3]);
    for (int i = 4; argv[i]; i+=2) {
        printf("%s: %s, ", argv[i], argv[i+1]);
    }
    printf("\n");
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
        executeCommand(testArray, argv[j], (int) strtol(argv[j+1], NULL, 10));
        realTime[j/2] = times(tmsTime[j/2]);
    }

    double sum = 0;
    printf("Real    User    System\n");
    printf("%s: \n", "Allocating");
    printf("%.2lf    ", calculateTime(realTime[0], realTime[1]));
    printf("%.2lf     ", calculateTime(tmsTime[0]->tms_utime, tmsTime[1]->tms_utime));
    printf("%.2lf ", calculateTime(tmsTime[0]->tms_stime, tmsTime[1]->tms_stime));
    sum += calculateTime(realTime[0], realTime[1]);
    printf("\n");

    for (int i = 4; argv[i]; i+=2) {
        printf("%s: \n", argv[i]);
        printf("%.2lf    ", calculateTime(realTime[i/2-1], realTime[i/2]));
        printf("%.2lf     ", calculateTime(tmsTime[i/2-1]->tms_utime, tmsTime[i/2]->tms_utime));
        printf("%.2lf \n", calculateTime(tmsTime[i/2-1]->tms_stime, tmsTime[i/2]->tms_stime));
        sum += calculateTime(realTime[i/2-1], realTime[i/2]);
    }
    printf("Full time:              %.2lf\n", sum);

    deleteArray(testArray);

#ifdef DLL
    dlclose(libraryPtr);
#endif

    return 0;
}


