#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <zconf.h>
#include <stdint-gcc.h>
#include "blockArray.h"

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
    return (double)(  end -  start)  / sysconf(_SC_CLK_TCK);
}

int main(int argc, char **argv) {
//
//    struct timespec* buffer = malloc(sizeof(struct timespec));
//    buffer->tv_sec = 0;
//    buffer->tv_nsec = 0;
//    clock_gettime(CLOCK_REALTIME_ALARM, buffer);
//    printf("%ld %ld \n", buffer->tv_nsec, buffer->tv_sec);
    srand((unsigned int) time(NULL));
    int arraySize = (int) strtol(argv[1], '\0', 10);
    int blockSize = (int) strtol(argv[2], '\0', 10);
//
//  int  arraySize = 500000;
//  int  blockSize = 300;
//
    int isDynamic;
    if (strcmp(argv[3], "dynamic")) {
        isDynamic = 1;
    } else if (strcmp(argv[3], "static")){
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

    findClosestByAscii(testArray, rand() % testArray->size_max );
    realTime[2] = times(tmsTime[2]);

    removeSpecificNumberOfBlocks(testArray, testArray->size_max/2, 0);
    realTime[3] = times(tmsTime[3]);

    alternatelyDeleteAndAdd(testArray, testArray->size_max/2);
    realTime[4] = times(tmsTime[4]);


    double sum = 0;
    printf("   Real      User      System\n");
    for (int i = 1; i < 5; ++i) {
        printf("%d. \n", i);
        printf("%lf   ", calculateTime(realTime[i-1], realTime[i]));
        printf("%lf   ", calculateTime(tmsTime[i-1]->tms_utime, tmsTime[i]->tms_utime));
        printf("%lf ", calculateTime(tmsTime[i-1]->tms_stime, tmsTime[i]->tms_stime));
        sum += calculateTime(realTime[i-1], realTime[i]);
        printf("\n");
    }

    printf("\nŁączny czas pracy prgramu:  %lf\n\n", sum);

    deleteArray(testArray);

    return 0;
}