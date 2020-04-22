#include <stdio.h>
#include <stdlib.h>

// Write 5 random bytes to stdout
// Sleep for [1 .. 3] seconds between each
int main() {
  srand(time(NULL));

  int i;
  for(i = 0; i < 5; i++) {
    char b = rand() % 256;
    write(1, &b, 1);
    sleep(rand() % 3 + 1);
  }
}
