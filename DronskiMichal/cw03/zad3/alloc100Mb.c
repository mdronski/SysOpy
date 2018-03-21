#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

int SIZE = 1024*1024;

int main
  printf("Begin allocating\n");
  char *bigAray2 = malloc(SIZE * sizeof(char));
    for (int i = 0; i < SIZE; ++i) {
        bigAray2[i] = 'a';
    }
  free(bigAray2);
  printf("Hello from allocating\n");
  return 0;
}
