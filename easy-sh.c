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

#include "./colors.h"

#define ARGS_LIMIT 5000
#define PIPE_LIMIT 100

void fcprintf(FILE *restrict stream, char *str, char *color,
              char *after_color) {
  fprintf(stream, "%s%s%s\n", color, str, after_color);
}

void run_function(int (*f)(const char *file, char *const *argv),
                  char *const *argv, int *in_pipe, int *out_pipe) {
  pid_t pid = fork();
  int status;

  if (pid < 0) {
    /* error occurred */
    fprintf(stderr, "Fork Failed\n");
    fcprintf(stderr, strerror(errno), RED, RESET);
  } else if (pid == 0) {
    /* execute function */
    if (in_pipe) {
      close(in_pipe[1]);
      dup2(in_pipe[0], STDIN_FILENO);
      close(in_pipe[0]);
    }
    if (out_pipe) {
      close(out_pipe[0]);
      dup2(out_pipe[1], STDOUT_FILENO);
      close(out_pipe[1]);
    }
    (*f)(argv[0], argv);
  } else {
    /* parent waiting */
    pid = wait(&status);
    if (out_pipe)
      close(out_pipe[1]);
  }

  if (status) {
    if (!pid) {
      fcprintf(stderr, strerror(errno), RED, RESET);
      exit(-1);
    }
  }
}
void run_command(char *const *argv, int *in_pipe, int *out_pipe) {
  run_function(execvp, argv, in_pipe, out_pipe);
}

// TODO: parse smarter (can't parse single qoute and double qoute)
// We don't need to, its not in project scope ( yaay! )
void split_to_argv(char *str, char *argv[]) {
  char *token;
  int i = 0;
  while ((token = strsep(&str, " \t\0"))) {
    if (*token != '\0' && *token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

void split_pipe_to_commands(char *str, char *argv[]) {
  char *token;
  int i = 0;
  while ((token = strsep(&str, "|"))) {
    if (*token != '\0' && *token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

// Built-ins
void cd(char *path) {
  int status = chdir(path);
  if (status)
    fcprintf(stderr, strerror(errno), RED, RESET);
}

int fw(const char *file, char *const *argv) {
  if (!argv[1]) {
    printf("fw - print first word of file\nUsage: fw [filename]\n");
    exit(0);
  } else {
    FILE *inp = fopen(argv[1], "r");
    if (!inp) {
      fcprintf(stderr, strerror(errno), RED, RESET);
      exit(2);
    } else {
      char ch;
      fscanf(inp, "%c", &ch);
      while (ch != ' ') {
        printf("%c", ch);
        fscanf(inp, "%c", &ch);
      }
      printf("\n");
      fclose(inp);
      exit(0);
    }
  }
}

void singline(char *file) {
  if (!file) {
    printf("Singline - print all file content without any space or "
           "whitespaces\nUsage: singline [filename]\n");
  } else {
    char *cmd_argv[] = {"sed", "-z", "s/\\s//g", file, NULL};
    run_command(cmd_argv, NULL, NULL);
    printf("\n");
  }
}

void nocomment(char *file) {
  if (!file) {
    printf("Nocomment - print all file content but lines starting with "
           "#\nUsage: nocomment [filename]\n");
  } else {
    char *cmd_argv[] = {"grep", "-v", "\\s*#", file, NULL};
    run_command(cmd_argv, NULL, NULL);
  }
}

void lc(char *file) {
  if (!file) {
    printf("lc - print line counts of file\nUsage: lc [filename]\n");
  } else {
    char *cmd_argv[] = {"wc", "-l", file, NULL};
    run_command(cmd_argv, NULL, NULL);
  }
}

void firsten(char *file) {
  if (!file) {
    printf("firsten - print first ten lines of file\nUsage: firsten "
           "[filename]\n");
  } else {
    char *cmd_argv[] = {"head", "-n10", file, NULL};
    run_command(cmd_argv, NULL, NULL);
  }
}

//
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
  char *pipe_cmds[PIPE_LIMIT];

  // init cwd (current working directory)
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);

  rl_bind_key('\t', rl_complete);

  char hist_path[PATH_MAX + 6];
  sprintf(hist_path, "%s/.hist", cwd);

  read_history(hist_path);
  using_history();

  char prompt[PATH_MAX + 11];
  sprintf(prompt, "%s%s%s$ ", GRN, cwd, RESET);

  while (1) {
    char *input = readline(prompt);
    if (input == NULL)
      break;
    else if (input[0] == '\0')
      continue;

    add_history(input);
    write_history(hist_path);


    split_pipe_to_commands(input, pipe_cmds);
    split_to_argv(pipe_cmds[0], cmd_argv);
    if (!strcmp(cmd_argv[0], "exit"))
      break;
    else if (!strcmp(cmd_argv[0], "cd")) {
      cd(cmd_argv[1]);
      getcwd(cwd, PATH_MAX);
      sprintf(prompt, "%s%s%s$ ", GRN, cwd, RESET);
    } else if (!strcmp(cmd_argv[0], "fw")) {
      run_function(fw, cmd_argv, NULL, NULL);
    } else if (!strcmp(cmd_argv[0], "singline")) {
      singline(cmd_argv[1]);
    } else if (!strcmp(cmd_argv[0], "nocomment")) {
      nocomment(cmd_argv[1]);
    } else if (!strcmp(cmd_argv[0], "lc")) {
      lc(cmd_argv[1]);
    } else if (!strcmp(cmd_argv[0], "firsten")) {
      firsten(cmd_argv[1]);
    } else {
      if (pipe_cmds[1] == NULL) {
        run_command(cmd_argv, NULL, NULL);
      } else {
        int pipe_fd0[2], pipe_fd1[2], i = 0;
        if (pipe(pipe_fd0))
          fcprintf(stderr, strerror(errno), RED, RESET);

        run_command(cmd_argv, NULL, pipe_fd0);

        for (i = 1; pipe_cmds[i + 1] != NULL; i++) {
          if (pipe(pipe_fd1))
            fcprintf(stderr, strerror(errno), RED, RESET);

          split_to_argv(pipe_cmds[i], cmd_argv);
          run_command(cmd_argv, pipe_fd0, pipe_fd1);

          pipe_fd0[0] = pipe_fd1[0];
          pipe_fd0[1] = pipe_fd1[1];
        }
        split_to_argv(pipe_cmds[i], cmd_argv);
        run_command(cmd_argv, pipe_fd0, NULL);
      }
    }
    free(input);
  }
  exit(0);
}
