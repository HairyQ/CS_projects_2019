#include <stdio.h>
int n;
void foo();
int main() {
  n = 256;
  foo();
  printf("%d", n);
}
