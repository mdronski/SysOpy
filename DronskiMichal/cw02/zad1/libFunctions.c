#include <stdlib.h>
#include <stdio.h>
#include <libFunctions.h>

void generateLib(char *filePath, int recordsNumber, int recordSize) {
    FILE *file = fopen(filePath, "w+");
    FILE *random = fopen("/dev/urandom", "r");
    char *buffer = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {

        if (fread(buffer, sizeof(char), (size_t) recordSize, random) != recordSize){
            printf("Reading from /dev/urandom failed. Aborting...\n");
            return;
        }

        for (int j = 0; j < recordSize; ++j) {
            buffer[j] = (char) (abs(buffer[j]) % 25 + 97);
        }
        buffer[recordSize-1] = 10;

        if (fwrite(buffer, sizeof(char), (size_t) recordSize, file) != recordSize) {
            printf("Writing to %s failed. Aborting...\n", filePath);
            return;
        }
    }
    free(buffer);
    fclose(file);
    fclose(random);
}

void copyLib(char *sourceFileName, char *destFileName, int recordsNumber, int recordSize){
    FILE *sourceFile = fopen(sourceFileName, "r");
    FILE *destFile = fopen(destFileName, "w+");
    char *buffer = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {

        if (fread(buffer, sizeof(char), (size_t) recordSize, sourceFile) != recordSize){
            printf("Reading from /dev/urandom failed. Aborting...\n");
            return;
        }

        if (fwrite(buffer, sizeof(char), (size_t) recordSize, destFile) != recordSize) {
            printf("Writing to %s failed. Aborting...\n", destFileName);
            return;
        }
    }
    free(buffer);
    fclose(sourceFile);
    fclose(destFile);
}

void sortLib(char *filePath, int recordsNumber, int recordSize){
    FILE *file = fopen(filePath, "r+");
    char* keyBuffer = (char *) calloc((size_t) recordSize, sizeof(char));
    char* tmpBuffer = (char *) calloc((size_t) recordSize, sizeof(char));
    long int offset = (long int) (recordSize * sizeof(char));
    int j;

    for (int i = 1; i < recordsNumber; ++i) {
        fseek(file, i*offset, 0);

        if (fread(keyBuffer, sizeof(char), (size_t) recordSize, file) != recordSize){
            printf("Reading from %s failed. Aborting...\n", filePath);
            return;
        }
        fseek(file, (-2) * offset, 1);
        if (fread(tmpBuffer, sizeof(char), (size_t) recordSize, file) != recordSize){
            printf("Reading from %s failed. Aborting...\n", filePath);
            return;
        }
        j = i ;
        while  ((int)keyBuffer[0] < (int)tmpBuffer[0] && j > 1 ){
            if (fwrite(tmpBuffer, sizeof(char), (size_t) recordSize, file) != recordSize) {
                printf("Writing to %s failed. Aborting...\n", filePath);
                return;
            }
            fseek(file, (-3) * offset, 1);
            if (fread(tmpBuffer, sizeof(char), (size_t) recordSize, file) != recordSize){
                printf("Reading from %s failed. Aborting...\n", filePath);
                return;
            }
            j--;
        }
        if (keyBuffer[0] < tmpBuffer[0] && j == 1 ){
            if (fwrite(tmpBuffer, sizeof(char), (size_t) recordSize, file) != recordSize) {
                printf("Writing to %s failed. Aborting...\n", filePath);
                return;
            }
            fseek(file, (-2) * offset, 1);
        }
        if (fwrite(keyBuffer, sizeof(char), (size_t) recordSize, file) != recordSize) {
            printf("Writing to %s failed. Aborting...\n", filePath);
            return;
        }
    }
    free(keyBuffer);
    free(tmpBuffer);
    fclose(file);
}

