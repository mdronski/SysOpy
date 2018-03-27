#include <stdlib.h>
#include <stdio.h>

int SIZE = 1024*1024*1024;

int main(){
    printf("Begin allocating\n");
    char *bigAray = malloc(SIZE * sizeof(char));
      for (int i = 0; i < SIZE; ++i) {
          bigAray[i] = 'a';
      }
    free(bigAray);
    perror("Allocating: ");
    return 0;
}
