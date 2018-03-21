#include <stdlib.h>
#include <stdio.h>

int fibbonacci(int n){
  if (n == 1 || n == 0) return n;
  else return fibbonacci(n-1) + fibbonacci(n-2);
}

int main(){
  printf("Started calculating 45'th fibbonaci number\n");
  long int x = fibbonacci(45);
  printf("%d\n", x);
  perror("Fibbonaci  ");
  return 0;
}
