#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

int main()
{
  int length = 500;
  unsigned int bufp;

  bufp = (unsigned int *) mmap(NULL, length, PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);
 
  printf("%x\n", bufp);


  bufp = (unsigned int *) mmap(NULL, length, PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);
 
  printf("%x\n", bufp);

  bufp = (unsigned int *) mmap(NULL, length, PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);
 
  printf("%x\n", bufp);

}

