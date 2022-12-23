#include <asm-generic/errno-base.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include "./colors.h"


// TODO: dynamic buffer ?
#define BUF_LIMIT 50000
#define ARGS_LIMIT 5000

void fcprintf(FILE *restrict stream, char* str, char* color, char* after_color){
        fprintf(stream, "%s%s%s", color, str, after_color);
}

pid_t pid = -1;
void run_command(char *const file, char *const *argv) {
  pid = fork();
  int status;

  if (pid < 0) { /* error occurred */
    fprintf(stderr, "Fork Failed");
  } else if (pid == 0) { /* execute cmd */
    execvp(file, argv);
  } else { /* parent waiting */
    pid = wait(&status);
  }

  if(status){
    switch (errno) {
      default:
        if(!pid)
          fprintf(stderr, "%serrno %i occurred!\n%s" , RED, errno, RESET);
        break;
      case ENOENT:
        if(!pid)
          fcprintf(stderr, "Command not found!\n", RED, RESET);
        break;
      case EACCES:
        if(!pid)
          fcprintf(stderr, "Permission denied!\n", RED, RESET);
        break;
      case ELOOP:
        if(!pid)
          fcprintf(stderr, "A loop exists in symbolic links (Maximum recursion limit exceeded)!\n", RED, RESET);
        break;
      case ENAMETOOLONG:
        if(!pid)
          fcprintf(stderr, "Path too long!\n", RED, RESET);
        break;
    }
    if(!pid)
      exit(-1);
  }
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


// Built-ins
void cd(char* path){
  int status = chdir(path);
  if(status){
    switch (errno) {
      default:
        break;
      case EACCES:
        fcprintf(stderr, "Permission denied!\n", RED, RESET);
        break;
      case ELOOP:
        fcprintf(stderr, "A loop exists in symbolic links (Maximum recursion limit exceeded)!\n", RED, RESET);
        break;
      case ENAMETOOLONG:
        fcprintf(stderr, "Path too long!\n", RED, RESET);
        break;
      case ENOENT:
        fcprintf(stderr, "Wrong path!\n", RED, RESET);
        break;
      case ENOTDIR:
        fcprintf(stderr, "Not a directory!\n", RED, RESET);
        break;
    }
  }
}

int main(int argc, char *argv[]) {
  char *buf = malloc(sizeof(char) * BUF_LIMIT);
  size_t strlen;
  FILE *history ;
  char *cmd_argv[ARGS_LIMIT];

  // TODO: full path
  history = fopen(".hist", "a+");

  while (1) {
    if (get_newline(buf, &strlen) != NULL && buf[0] != '\0') {
        // store all commands to .hist file TODO: handle max count of commands in history
        fprintf(history,"%s\n", buf);
        fflush(history);
        split_to_argv(buf, cmd_argv);
        if(!strcmp(cmd_argv[0],"exit")) break;
        else if(!strcmp(cmd_argv[0],"cd")){
          cd(cmd_argv[1]);
        }else{
          run_command(cmd_argv[0], cmd_argv);
        }
    }
  }
  free(buf);
  fclose(history);
  exit(0);
}
