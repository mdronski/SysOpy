#include "blockArray.h"

char staticAllocatedArray[10000000][600];

int asciiSum(char* block){
    int sum = 0;
    for (int i = 0; i < strlen(block); ++i) {
        sum += (int) block[i];
    }
    return sum;
}

BlockArray *initArray(int array_size, int block_size, int isDynamic) {
    BlockArray *array = (BlockArray *) calloc(1, sizeof(BlockArray));
    if (isDynamic) {
        char **stringArray = (char **) calloc((size_t) array_size, sizeof(char *));
        array->array = stringArray;
        array->size_max = array_size;
        array->size_block = block_size;
        array->isDynamicAllocated = 1;
        return array;
    }
    array->array =(char **) staticAllocatedArray;
    array->size_max = array_size;
    array->size_block = block_size;
    array->isDynamicAllocated = 0;
    return  array;
}

void cleanStaticArray(BlockArray *staticArray){
    for (int i = 0; i < staticArray->size_max; ++i) {
            staticArray->array[i] = "";
    }
}

void deleteArray(BlockArray* blockArray){
    if (blockArray->isDynamicAllocated) {
        for (int i = 0; i < blockArray->size_max; ++i) {
            free(blockArray->array[i]);
        }
        free(blockArray->array);
        free(blockArray);
        return;
    } else{
        cleanStaticArray(blockArray);
    }
}

void addBlock(BlockArray* blockArray, int index, char* block){
    if (index >= blockArray->size_max){
        printf("%s\n", "Index is too big");
        return;
    }else if(strlen(block)>blockArray->size_block) {
        printf("%s\n", "String is too long for this array.");
        return;
    }
    else {
        if (blockArray->isDynamicAllocated) {
            if (blockArray->array[index] != NULL) removeBlock(blockArray, index);
            blockArray->array[index] = (char *) calloc((size_t) blockArray->size_block, sizeof(char));
            strcpy(blockArray->array[index], block);
            free(block);
            return;
        } else {
            removeBlock(blockArray, index);
            blockArray->array[index] = block;
        }
    }
}

void removeBlock(BlockArray* blockArray, int index){
    if (blockArray->isDynamicAllocated) {
        free(blockArray->array[index]);
        blockArray->array[index] = NULL;
        return;
    }
        blockArray->array[index] = "";
}

void printArray(BlockArray* blockArray){
    for (int i=0; i<blockArray->size_max; i++){
        if (blockArray->array[i] == NULL) printf("\n");
        else
            printf("%s %d\n", blockArray->array[i], asciiSum(blockArray->array[i]));
    }
    printf("\n");
}

char* findClosestByAscii(BlockArray* blockArray, int index){
    char* tmpClosestBlock;
    int tmpClosestSum = 9999999;
    int sum = asciiSum(blockArray->array[index]);
    int tmpSum;

    for (int i = 0; i < blockArray->size_max; ++i) {
        if (i != index){
            tmpSum = asciiSum(blockArray->array[i]);
            if ((abs(sum - tmpSum) < tmpClosestSum) && tmpSum!=0){
                tmpClosestSum = tmpSum;
                tmpClosestBlock = blockArray->array[i];
            }
        }
    }
    return tmpClosestBlock;
}
