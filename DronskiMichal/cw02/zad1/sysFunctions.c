#include "sysFunctions.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void generateSys(char *filePath, int recordsNumber, int recordSize) {
    int fileDesc = open(filePath, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    int devRandDesc = open("/dev/urandom", O_RDONLY);
    char *buffer = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {
        if (read(devRandDesc, buffer, (size_t) recordSize) != recordSize){
            printf("Reading from /dev/urandom failed. Aborting...\n");
            return;
        }
        for (int j = 0; j < recordSize; ++j) {
            buffer[j] = (char) (abs(buffer[j]) % 25 + 97);
        }
        buffer[recordSize-1] = 10;
        if (write(fileDesc, buffer, (size_t) recordSize) != recordSize) {
            printf("Writing to %s failed. Aborting...\n", filePath);
            return;
        }
    }
    close(fileDesc);
    close(devRandDesc);
}

void copySys(char *sourceFileName, char *destFileName, int recordsNumber, int recordSize){
    int destFileDesc = open(destFileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    int sourceFileDesc = open(sourceFileName, O_RDONLY);
    char *buffer = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {
        if (read(sourceFileDesc, buffer, (size_t) recordSize) != recordSize){
            printf("Reading from %s failed. Aborting...\n", sourceFileName);
            return;
        }
        if (write(destFileDesc, buffer, (size_t) recordSize) != recordSize) {
            printf("Writing to %s failed. Aborting...\n", destFileName);
            return;
        }
    }
    close(sourceFileDesc);
    close(destFileDesc);
}

void sortSys(char *filePath, int recordsNumber, int recordSize){
    int fileDesc = open(filePath, O_RDWR, O_TRUNC);
    char* keyBuffer = (char *) calloc((size_t) recordSize, sizeof(char));
    char* tmpBuffer = (char *) calloc((size_t) recordSize, sizeof(char));
    long int offset = (long int) (recordSize * sizeof(char));
    int j;

    for (int i = 1; i < recordsNumber; ++i) {

        printf("%d\n", lseek(fileDesc, i*offset, SEEK_SET));

        if (read(fileDesc, keyBuffer, (size_t) recordSize ) != recordSize){
            printf("Reading from %s failed. Aborting...\n", filePath);
            return;
        }
        printf("%d\n", lseek(fileDesc, 0, 1));
        printf("%s", keyBuffer);

        lseek(fileDesc, (-2) * offset, SEEK_CUR);
        printf("%d\n", lseek(fileDesc, 0, 1));
        if (read(fileDesc, tmpBuffer, (size_t) recordSize ) != recordSize) {
            printf("Reading from %s failed. Aborting...\n", filePath);
            return;
        }

        j = i ;

        while  ((int)keyBuffer[0] < (int)tmpBuffer[0] && j > 1 ){
            if (write(fileDesc, tmpBuffer, (size_t) recordSize * sizeof(char)) != recordSize) {
                printf("Writing to %s failed. Aborting...\n", filePath);
                return;
            }
            lseek(fileDesc, (-3) * offset, SEEK_CUR);
            if (read(fileDesc, tmpBuffer, (size_t) recordSize * sizeof(char)) != recordSize) {
                printf("Writing to %s failed. Aborting...\n", filePath);
                return;
            }
            j--;
        }

        if (keyBuffer[0] < tmpBuffer[0] && j == 1 ){
            if (write(fileDesc, tmpBuffer, (size_t) recordSize * sizeof(char)) != recordSize) {
                printf("Writing to %s failed. Aborting...\n", filePath);
                return;
            }
            lseek(fileDesc, (-2) * offset, SEEK_CUR);
        }

        if (write(fileDesc, keyBuffer, (size_t) recordSize * sizeof(char)) != recordSize) {
            printf("Writing to %s failed. Aborting...\n", filePath);
            return;
        }

    }
    free(tmpBuffer);
    free(keyBuffer);
    close(fileDesc);
}
