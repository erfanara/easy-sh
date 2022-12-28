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

void fcprintf(FILE * restrict stream, char * str, char * color,
  char * after_color) {
  fprintf(stream, "%s%s%s\n", color, str, after_color);
}

pid_t pid = -1;
void run_command(char *
  const file, char *
    const * argv) {
  // TODO: handle errno
  pid = fork();
  int status;

  if (pid < 0) {
    /* error occurred */
    fprintf(stderr, "Fork Failed\n");
    fcprintf(stderr, strerror(errno), RED, RESET);
  } else if (pid == 0) {
    /* execute cmd */
    if (!strcmp(file, "fw")) { // first word, a command
      if (!argv[1]) {
        printf("fw - print first word of file\nUsage: fw [filename]\n");
        exit(0);
      } else {
        FILE * inp = fopen(argv[1], "r");
        if (!inp) {
          fcprintf(stderr, strerror(errno), RED, RESET);
          exit(2);
        } else {
          char ch;
          fscanf(inp, "%c", & ch);
          while (ch != ' ') {
            printf("%c", ch);
            fscanf(inp, "%c", & ch);
          }
          printf("\n");
          fclose(inp);
          exit(0);
        }
      }
    } else {
      execvp(file, argv);
    }
  } else {
    /* parent waiting */
    pid = wait( & status);
  }

  if (status) {
    if (!pid) {
      fcprintf(stderr, strerror(errno), RED, RESET);
      exit(-1);
    }
  }
}

// TODO: parse smarter (can't parse single qoute and double qoute)
// We don't need to, its not in project scope ( yaay! )
void split_to_argv(char * str, char * argv[]) {
  char * token;
  int i = 0;
  while ((token = strsep( & str, " \t\0"))) {
    if ( * token != '\0' && * token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

void split_pipe_to_commands(char * str, char * argv[]) {
  char * token;
  int i = 0;
  while ((token = strsep( & str, "|"))) {
    if ( * token != '\0' && * token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

// Built-ins
void cd(char * path) {
  int status = chdir(path);
  if (status)
    fcprintf(stderr, strerror(errno), RED, RESET);
}

void sigint_handler(int i) {
  rl_replace_line("", 0);
  rl_redisplay();
  rl_done = 1;
}
int dummy_event(void) {
  return 0;
}

int main(int argc, char * argv[]) {
  rl_event_hook = dummy_event;
  signal(SIGINT, sigint_handler);

  char * cmd_argv[ARGS_LIMIT];

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

    char * input = readline(prompt);
    if (input != NULL && input[0] != '\0') {
      add_history(input);
      write_history(hist_path);
      if (strstr(input, "|") != NULL) {
        size_t i, n;
        int prev_pipe, pfds[2];
        split_pipe_to_commands(input, cmd_argv);
        char * args[ARGS_LIMIT];
        for (n = 0; cmd_argv[n]; n++);
        prev_pipe = STDIN_FILENO;

        for (i = 0; i < n - 1; i++) {
          pipe(pfds);

          if (fork() == 0) {
            // Redirect previous pipe to stdin
            if (prev_pipe != STDIN_FILENO) {
              dup2(prev_pipe, STDIN_FILENO);
              close(prev_pipe);
            }

            // Redirect stdout to current pipe
            dup2(pfds[1], STDOUT_FILENO);
            close(pfds[1]);

            // Start command
            split_to_argv(cmd_argv[i], args);
            execvp(args[0], args);
            perror("execvp Failed");
            exit(1);
          }

          // Close read end of previous pipe (not needed in the parent)
          close(prev_pipe);

          // Close write end of current pipe (not needed in the parent)
          close(pfds[1]);

          // Save read end of current pipe to use in next iteration
          prev_pipe = pfds[0];
        }

        // Get stdin from last pipe
        if (prev_pipe != STDIN_FILENO) {
          dup2(prev_pipe, STDIN_FILENO);
          close(prev_pipe);
        }

        split_to_argv(cmd_argv[i], args);
        // Start last command
        execvp(args[0], args);
        perror("Execvp Failed:");

      } else {
        split_to_argv(input, cmd_argv);
        if (!strcmp(cmd_argv[0], "exit"))
          break;
        else if (!strcmp(cmd_argv[0], "cd")) {
          cd(cmd_argv[1]);
          getcwd(cwd, PATH_MAX);
        } else if (!strcmp(cmd_argv[0], "singline")) {
          if (!cmd_argv[1]) {
            printf("Singline - print all file content without any space or whitespaces\nUsage: singline [filename]\n");
          } else {
            char cmd[500];
            sprintf(cmd, "sed -z s/\\s//g %s", cmd_argv[1]);
            split_to_argv(cmd, cmd_argv);
            run_command(cmd_argv[0], cmd_argv);
            printf("\n");

          }
        } else if (!strcmp(cmd_argv[0], "nocomment")) {
          if (!cmd_argv[1]) {
            printf("Nocomment - print all file content but lines starting with #\nUsage: nocomment [filename]\n");
          } else {
            char cmd[500];
            sprintf(cmd, "grep -v \\s*# %s", cmd_argv[1]);
            split_to_argv(cmd, cmd_argv);
            run_command(cmd_argv[0], cmd_argv);
          }
        } else if (!strcmp(cmd_argv[0], "lc")) {
          if (!cmd_argv[1]) {
            printf("lc - print line counts of file\nUsage: lc [filename]\n");
          } else {
            char cmd[500];
            sprintf(cmd, "wc -l %s", cmd_argv[1]);
            split_to_argv(cmd, cmd_argv);
            run_command(cmd_argv[0], cmd_argv);
          }
        } else if (!strcmp(cmd_argv[0], "firsten")) {
          if (!cmd_argv[1]) {
            printf("firsten - print first ten lines of file\nUsage: firsten [filename]\n");
          } else {
            char cmd[500];
            sprintf(cmd, "head -n10 %s", cmd_argv[1]);
            split_to_argv(cmd, cmd_argv);
            run_command(cmd_argv[0], cmd_argv);
          }
        } else {
          run_command(cmd_argv[0], cmd_argv);
        }
      }
    }
    free(input);
  }
  exit(0);
}
