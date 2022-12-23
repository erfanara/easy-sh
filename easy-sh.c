#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "./colors.h"

// TODO: dynamic buffer ?
#define BUF_LIMIT 50000
#define ARGS_LIMIT 5000
pid_t pid = -1;
int run_command(char *const file, char *const *argv) {
  pid = fork();
  int status;

  if (pid < 0) { /* error occurred */
    fprintf(stderr, "Fork Failed");
  } else if (pid == 0) { /* execute cmd */
    execvp(file, argv);
  } else { /* parent waiting */
    pid = wait(&status);
  }
  return status;
}

char *get_newline(char *line, size_t *len) {
  if (fgets(line, BUF_LIMIT, stdin)) {
    *len = strlen(line);
    if (*len > 0 && line[*len - 1] == '\n')
      line[--(*len)] = '\0';
  }
  return line;
}

void split_to_argv(char *str, char *argv[]) {
  char *token;
  int i = 0;
  while ((token = strsep(&str, " \t\0"))) {
    if (*token != '\0' && *token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}
void fcprintf(char* str, char* color, char* after_color){

        fprintf(stdout,"%s%s%s",color,str,after_color);
}

int main(int argc, char *argv[]) {
  char *buf = malloc(sizeof(char) * BUF_LIMIT);
  size_t strlen;
  FILE *history ;
  char *cmd_argv[ARGS_LIMIT];

  while (1) {
    history = fopen(".hist", "a+");
    if (get_newline(buf, &strlen) != NULL && buf[0] != '\0') {
        // store all commands to .hist file TODO: handle max count of commands in history
        fprintf(history,"%s\n", buf);
        fflush(history);
        split_to_argv(buf, cmd_argv);
        if(!strcmp(cmd_argv[0],"exit")) break;
        else{
          run_command(cmd_argv[0], cmd_argv);
          //if errno == 2, then it means theres no such file or directory, in other words, command not found 
          if(!pid && errno == 2){// we exec this on child process, because we want to kill the child after printing err
          fprintf(stdout,"%s%s: command not found!%s\n",RED,cmd_argv[0],RESET);
            /* printf("%d\n",errno); 
             printf("%s:%d\n",cmd_argv[0] ,pid); */
            exit(127);
          }
        }
    }
  }
  free(buf);
  fclose(history);
  exit(0);
}
