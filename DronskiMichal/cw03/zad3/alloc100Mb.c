#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <zconf.h>
#include <memory.h>
int SIZE = 1024*1024*1024;

void printRusage(struct rusage rUsage[2]){
    long int stackMemStart = rUsage[0].ru_ixrss;
    long int stackMemEnd = rUsage[1].ru_ixrss;
    double stackMem = (double) (stackMemEnd); //- stackMemStart) / (1024*1024);

    long int dataStart = rUsage[0].ru_idrss;
    long int dataEnd = rUsage[1].ru_idrss;
    double data = (double) (dataEnd);// - dataStart) / (1024*1024);

    printf("Stack size: %ld\n", stackMemEnd);
    printf("Data  size: %ld\n", dataEnd);

}


int main(){
    struct rusage rUsage[2];
    getrusage(RUSAGE_SELF, &rUsage[0]);
    printf("Begin allocating\n");
    char *bigAray2 = malloc(SIZE * sizeof(char));
      for (int i = 0; i < SIZE; ++i) {
          bigAray2[i] = 'a';
      }
    getrusage(RUSAGE_SELF, &rUsage[1]);
//      printRusage(rUsage);
    free(bigAray2);
    printf("Ended allocating\n");
    perror("Allocating: ");
    return 0;
}
