#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

int SIZE = 1000000;

int main(){
//  char bigAray1[1000000];
  printf("Begin allocating\n");
  char *bigAray2 = malloc(SIZE * sizeof(char));
    for (int i = 0; i < SIZE; ++i) {
        bigAray2[i] = 'a';
    }
  free(bigAray2);
  printf("Hello from allocating\n");
  return 0;
}
