#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// TODO: dynamic buffer ?
#define BUF_LIMIT 50000
#define ARGS_LIMIT 5000

int run_command(char *const file, char *const *argv) {
  pid_t pid = fork();
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
  // TODO: strsep compatbility with windows
  while ((token = strsep(&str, " "))) {
    if (*token != '\0')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

int main(int argc, char *argv[]) {
  char *buf = malloc(sizeof(char) * BUF_LIMIT);
  size_t strlen;

  char *cmd_argv[ARGS_LIMIT];

  while (1) {
    if (get_newline(buf, &strlen) != NULL) {
      split_to_argv(buf, cmd_argv);
      run_command(cmd_argv[0], cmd_argv);
    }
  }

  free(buf);
}
