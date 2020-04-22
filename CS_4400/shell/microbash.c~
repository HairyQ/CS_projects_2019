#include <unistd.h>
#include <stdlib.h>

int main() {
  char* writer_args[] = {"./writer", NULL};
  char* reader_args[] = {"./reader", NULL};
  char* env[] = {NULL};
 
  // First child
  if(fork() == 0) 
    if(execve(writer_args[0], writer_args, env) == -1)
      exit(1);
  
  // Second child
  if(fork() == 0) 
    if(execve(reader_args[0], reader_args, env) == -1)
      exit(1);

  // Parent
  int status;
  wait(&status);
  wait(&status);
}

