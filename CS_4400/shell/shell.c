#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN 50

/* Read a command from the keyboard and return whether or not it is
 * requested as a background process (i.e., ends in &).
 */
int read_command(char* cmd_str) {
  size_t size = MAX_LEN; 
  getline(&cmd_str, &size, stdin);

  // remove the newline
  cmd_str[strlen(cmd_str) - 1] = '\0';
  
  if(cmd_str[strlen(cmd_str) - 1] == '&') {
    // remove the space and & to make this an executable command
    cmd_str[strlen(cmd_str) - 2] = '\0';
    return 1;
  }
  
  return 0;
}

int main() {
  pid_t pid;

  // allocate space for command string
  char* command = (char*)malloc(MAX_LEN);

  // shells work forever ... 
  while(1) {
    
    // print prompt
    printf("[tiny shell]$ ");

    // read the next command from the user
    // fill in command string and return whether to run in background
    int is_bg = read_command(command);

    // handle one built-in command: 
    if(!strcmp(command, "quit"))
      break;

    // create child process to run (not built-in) command
    pid = fork();

    if(pid == 0) {
      // build argv and envp arrays
      char* argv[2] = {command, NULL};
      char* envp[1] = {NULL};

      if(execve(argv[0], argv, envp) == -1) {
	printf("unknown command: %s\n", command);
	exit(1);
      }
    }

    // parent: wait on foreground process to finish
    if(!is_bg) {
      int status;
      waitpid(pid, &status, 0);
    }
  }
}
