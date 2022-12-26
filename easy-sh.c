#include "./colors.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#define ARGS_LIMIT 5000

void fcprintf(FILE *restrict stream, char *str, char *color,
              char *after_color) {
  fprintf(stream, "%s%s%s\n", color, str, after_color);
}

pid_t pid = -1;
void run_command(char *const file, char *const *argv) {
  // TODO: handle errno
  pid = fork();
  int status;

  if (pid < 0) { /* error occurred */
    fprintf(stderr, "Fork Failed\n");
    fcprintf(stderr, strerror(errno), RED, RESET);
  } else if (pid == 0) { /* execute cmd */
    execvp(file, argv);
  } else { /* parent waiting */
    pid = wait(&status);
  }

  if (status) {
    if (!pid){
      fcprintf(stderr, strerror(errno), RED, RESET);
      exit(-1);
    }
  }
}

// TODO: parse smarter (can't parse single qoute and double qoute)
void split_to_argv(char *str, char *argv[]) {
  char *token;
  int i = 0;
  while ((token = strsep(&str, " \t\0"))) {
    if (*token != '\0' && *token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

<<<<<<< HEAD
// Built-ins
void cd(char *path) {
  int status = chdir(path);
  if (status)
    fcprintf(stderr, strerror(errno), RED, RESET);
}

void sigint_handler(int i) {
  rl_replace_line("", 0);
  rl_redisplay();
  rl_done = 1;
}
int dummy_event(void) { return 0; }

int main(int argc, char *argv[]) {
  rl_event_hook = dummy_event;
  signal(SIGINT, sigint_handler);

  char *cmd_argv[ARGS_LIMIT];

  // init cwd (current working directory)
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);

  rl_bind_key('\t', rl_complete);

  char hist_path[PATH_MAX + 6];
  sprintf(hist_path, "%s/.hist", cwd);

  read_history(hist_path);
  using_history();

  char prompt[PATH_MAX + 11];

  while (1) {
    sprintf(prompt, "%s%s%s$ ", GRN, cwd, RESET);

    char *input = readline(prompt);
    if (input != NULL && input[0] != '\0') {
      add_history(input);
      write_history(hist_path);

      split_to_argv(input, cmd_argv);
      if (!strcmp(cmd_argv[0], "exit"))
        break;
      else if (!strcmp(cmd_argv[0], "cd")) {
        cd(cmd_argv[1]);
        getcwd(cwd, PATH_MAX);
      } else {
        run_command(cmd_argv[0], cmd_argv);
      }
    }
    free(input);
  }
  exit(0);
}
