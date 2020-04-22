#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char* writer_args[] = {argv[1], NULL};
  char* reader_args[] = {argv[2], NULL};
  char* env[] = {NULL};
  int fds[2];

  pipe(fds);
 
  // First child
  if(fork() == 0)
    {
      dup2(fds[1], 1);

      close(fds[0]);
      close(fds[1]);
      
      if(execve(writer_args[0], writer_args, env) == -1)
	exit(1);
    }
  
  // Second child
  if(fork() == 0)
    {
      dup2(fds[0], 0);

      close(fds[0]);
      close(fds[1]);
      
      if(execve(reader_args[0], reader_args, env) == -1)
	exit(1);

    }

  close(fds[0]);
  close(fds[1]);

  // Parent
  int status;
  wait(&status);
  wait(&status);
}

