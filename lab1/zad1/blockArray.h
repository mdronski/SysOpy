#include <malloc.h>
#include <string.h>
#include <stdlib.h>

typedef struct BlockArray{
    char** array;
    int size_max;
    int size_block;
} BlockArray;

int asciiSum(char* block);

BlockArray initArrayDynamic(int array_size, int block_size);

void deleteArray(BlockArray* blockArray);

void addBlock(BlockArray blockArray, int index, char* block);

void removeBlock(BlockArray* blockArray, int index);

void printArray(BlockArray blockArray);

char* findClosestByAscii(BlockArray blockArray, int index);
