#include <malloc.h>
#include <string.h>
#include <stdlib.h>
##include "blockArray.h"

int asciiSum(char* block){
    int sum = 0;
    for (int i = 0; i < strlen(block); ++i) {
        sum += (int) block[i];
    }
    return sum;
}

BlockArray initArrayDynamic(int array_size, int block_size){

    BlockArray* array = (BlockArray*) calloc(1, sizeof(BlockArray));
    char** stringArray = (char**) calloc(array_size, sizeof(char*));

    array->array = stringArray;
    array->size_max = array_size;
    array->size_block = block_size;

    return *array;
}

void deleteArray(BlockArray* blockArray){
    for (int i = 0; i < blockArray->size_max; ++i) {
        free(blockArray->array[i]);
    }
}

void addBlock(BlockArray blockArray, int index, char* block){
    if (index >= blockArray.size_max){
        printf("%s\n", "Index is too big");
        return;
    }else if(strlen(block)>=blockArray.size_block) {
        printf("%s\n", "String is too long for this array.");
        return;
        }
        else {
            if (blockArray.array[index] != NULL) removeBlock(&blockArray, index);
            blockArray.array[index] = (char*) calloc(blockArray.size_block, sizeof(char));
            strcpy(blockArray.array[index], block);
        }
}

void removeBlock(BlockArray* blockArray, int index){
    free(blockArray->array[index]);
    blockArray->array[index] = NULL;
}

void printArray(BlockArray blockArray){
    for (int i=0; i<blockArray.size_max; i++){
        if (blockArray.array[i] == NULL) printf("\n");
        else
        printf("%s %d\n", blockArray.array[i], asciiSum(blockArray.array[i]));
    }
    printf("\n");
}

char* findClosestByAscii(BlockArray blockArray, int index){
    char* tmpClolsestBlock;
    int tmpClosestSum;
    int sum = asciiSum(blockArray.array[index]);
    int tmpSum;

    if (index == 0){
        tmpClolsestBlock = blockArray.array[1];
    } else{
        tmpClolsestBlock = blockArray.array[0];
    }

    tmpClosestSum = asciiSum(tmpClolsestBlock);

    for (int i = 0; i < blockArray.size_max; ++i) {
        if (i != index){
            tmpSum = asciiSum(blockArray.array[i]);
            if (abs(sum - tmpSum) < tmpClosestSum ){
                tmpClosestSum = tmpSum;
                tmpClolsestBlock = blockArray.array[i];
            }
        }
    }
    return tmpClolsestBlock;
}
