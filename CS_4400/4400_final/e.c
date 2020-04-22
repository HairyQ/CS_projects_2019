#include <stdio.h>
#include <stdlib.h>

int main()
{
  int x = 2147483646;
  printf("%d\n", (int)(float)x);
}
