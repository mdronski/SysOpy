#include <malloc.h>
#include <string.h>
#include <stdlib.h>

typedef struct BlockArray{
    char** array;
    int size_max;
    int size_block;
    int isDynamicAllocated;
} BlockArray;

#define ARRAY_SIZE 5000000
#define BLOCK_SIZE 1000

extern char staticAllocatedArray[ARRAY_SIZE][BLOCK_SIZE];

int asciiSum(char* block);

BlockArray *initArray(int array_size, int block_size, int isDynamic);

void deleteArray(BlockArray* blockArray);

void addBlock(BlockArray* blockArray, int index, char* block);

void removeBlock(BlockArray* blockArray, int index);

void printArray(BlockArray* blockArray);

char* findClosestByAscii(BlockArray* blockArray, int value);
