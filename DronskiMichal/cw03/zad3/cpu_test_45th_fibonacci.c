#include <stdlib.h>
#include <stdio.h>

int fibonacci(int n){
  if (n == 1 || n == 0) return n;
  else return fibonacci(n-1) + fibonacci(n-2);
}

int main(){
  printf("Started calculating 45'th fibbonaci number\n");
  long int x = fibonacci(45);
  printf("45'th Fibonacci number: %d\n", x);
  perror("Fibbonaci  ");
  return 0;
}
