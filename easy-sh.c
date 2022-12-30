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

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define ARGS_LIMIT 5000
#define PIPE_LIMIT 100

// init in main
char cwd[PATH_MAX];
char prompt[PATH_MAX + 11];

/* Print to stream with color support */
void fcprintf(FILE *restrict stream, char *str, char *color,
              char *after_color) {
  fprintf(stream, "%s%s%s\n", color, str, after_color);
}

// TODO: parse smarter (can't parse single qoute and double qoute)
// We don't need to, its not in project scope ( yaay! )
/* Parser function: split 'str' to 'argv'.
 * delimiter:whitespace characters
 */
void split_to_argv(char *str, char *argv[]) {
  char *token;
  int i = 0;
  while ((token = strsep(&str, " \t\0"))) {
    if (*token != '\0' && *token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

/* Parser function: split 'str' to 'argv'.
 * delimiter: '|'
 */
void split_pipe_to_commands(char *str, char *argv[]) {
  char *token;
  int i = 0;
  while ((token = strsep(&str, "|"))) {
    if (*token != '\0' && *token != '\t')
      argv[i++] = token;
  }
  argv[i] = NULL;
}

/* Core function of easy-sh.
 *
 * Able of running any function with name 'executor' after calling fork().
 *          int executor(const char *, char *const *argv)
 * you can pass the 'execvp' function to run your program using it.
 *
 * Able of using pipes to redirect STDIN and STDOUT (*in_pipe and *out_pipe).
 * set *in_pipe and *out_pipe to NULL if you don't want to redirect STDIN and
 * STDOUT of your function.
 *
 * This function does not use any parser function, thus you need to split your
 * command string to argv before passing it.
 */
void run_function(int (*executor)(const char *file, char *const *argv),
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
    (*executor)(argv[0], argv);
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

/* This is just a 'run_function(...)' with execvp as of its executor ,
 * look at 'run_function(...)' for more details
 */
void run_with_exec(char *const *argv, int *in_pipe, int *out_pipe) {
  run_function(execvp, argv, in_pipe, out_pipe);
}

/* This is just a run_function(...) with execvp as of its executor.
 * This function splits your command itself before using it,
 * so you don't need to think about parsing stuff.
 */
void run_with_exec_with_spliting(char *input, int *in_pipe, int *out_pipe) {
  char *argv[ARGS_LIMIT];
  split_to_argv(input, argv);
  run_function(execvp, argv, in_pipe, out_pipe);
}

/* Runs your commands using pipeline.
 *
 * You need to pass your commands as a list ('char *const *pipe_cmd')
 * Also you need to specify your executor function (your executor should support
 * splitting itself)
 *
 * You can redirect STDIN and STDOUT of your command using pipes with *in_pipe
 * and *out_pipe ,
 * set them to NULL if you don't want to redirect STDIN and STDOUT.
 */
void run_function_with_pipeline(
    void (*executor_with_spliting)(char *input, int *in_pipe, int *out_pipe),
    char *const *pipe_cmds, int *in_pipe, int *out_pipe) {
  int pipe_fd0[2], pipe_fd1[2], i = 0;
  if (pipe(pipe_fd0))
    fcprintf(stderr, strerror(errno), RED, RESET);

  executor_with_spliting(pipe_cmds[i], in_pipe, pipe_fd0);

  for (i = 1; pipe_cmds[i + 1] != NULL; i++) {
    if (pipe(pipe_fd1))
      fcprintf(stderr, strerror(errno), RED, RESET);

    executor_with_spliting(pipe_cmds[i], pipe_fd0, pipe_fd1);

    pipe_fd0[0] = pipe_fd1[0];
    pipe_fd0[1] = pipe_fd1[1];
  }
  executor_with_spliting(pipe_cmds[i], pipe_fd0, out_pipe);
}

// Built-ins
/* Built-in command: Change directory */
void cd(char *path) {
  int status = chdir(path);
  getcwd(cwd, PATH_MAX);
  sprintf(prompt, "%s%s%s$ ", GRN, cwd, RESET);
  if (status)
    fcprintf(stderr, strerror(errno), RED, RESET);
}

/* Built-in command: print first word of file */
int fw(const char *file, char *const *argv) {
  if (!argv[1]) {
    fcprintf(stderr, "fw - print first word of file\nUsage: fw [filename]\n",
             YEL, RESET);
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

/* Built-in command: print all file content without any space or whitespaces */
void singline(char *file, int *in_pipe, int *out_pipe) {
  if (!file) {
    fcprintf(stderr,
             "Singline - print all file content without any space or "
             "whitespaces\nUsage: singline [filename]\n",
             YEL, RESET);
  } else {
    char *cmd_argv[] = {"sed", "-z", "s/\\s//g", file, NULL};
    run_with_exec(cmd_argv, in_pipe, out_pipe);
    printf("\n");
  }
}

/* Built-in command: print all file content but lines starting with '#' */
void nocomment(char *file, int *in_pipe, int *out_pipe) {
  if (!file) {
    fcprintf(stderr,
             "Nocomment - print all file content but lines starting with "
             "#\nUsage: nocomment [filename]\n",
             YEL, RESET);
  } else {
    char *cmd_argv[] = {"grep", "-v", "\\s*#", file, NULL};
    run_with_exec(cmd_argv, in_pipe, out_pipe);
  }
}

/* Built-in command: print line counts of file */
void lc(char *file, int *in_pipe, int *out_pipe) {
  if (!file) {
    fcprintf(stderr, "lc - print line counts of file\nUsage: lc [filename]\n",
             YEL, RESET);
  } else {
    char *cmd_argv[] = {"wc", "-l", file, NULL};
    run_with_exec(cmd_argv, in_pipe, out_pipe);
  }
}

/* Built-in command: print first ten lines of file */
void firsten(char *file, int *in_pipe, int *out_pipe) {
  if (!file) {
    fcprintf(stderr,
             "firsten - print first ten lines of file\nUsage: firsten "
             "[filename]\n",
             YEL, RESET);
  } else {
    char *cmd_argv[] = {"head", "-n10", file, NULL};
    run_with_exec(cmd_argv, in_pipe, out_pipe);
  }
}

/* Built-in command: print most ferequent word in a file */
void mostword(char *file, int *in_pipe, int *out_pipe) {
  if (!file) {
    fcprintf(stderr,
             "mostword - print most ferequent word in a file "
             "\nUsage: mostword [filename]\n",
             YEL, RESET);
  } else {
    char cmd0[PATH_MAX + 84] = "cat ";
    strcat(cmd0, file);
    char *cmd1 = " | sed -r s/[[:space:]]+/\\n/g | sed /^$/d | sort | "
                 "uniq -c | sort -n | tail -n1";
    strcat(cmd0, cmd1);
    char *pipe_cmds[PIPE_LIMIT];
    split_pipe_to_commands(cmd0, pipe_cmds);
    run_function_with_pipeline(run_with_exec_with_spliting, pipe_cmds, in_pipe,
                               out_pipe);
  }
}

/* Runs your command plus you can use built-in commands declared above
 *
 * You can redirect STDIN and STDOUT of your command using pipes with *in_pipe
 * and *out_pipe ,
 * set them to NULL if you don't want to redirect STDIN and STDOUT.
 * */
void run_command_with_splitting(char *command, int *in_pipe, int *out_pipe) {
  char *argv[ARGS_LIMIT];
  split_to_argv(command, argv);

  if (!strcmp(argv[0], "exit"))
    exit(0);
  else if (!strcmp(argv[0], "cd")) {
    cd(argv[1]);
  } else if (!strcmp(argv[0], "fw")) {
    run_function(fw, argv, in_pipe, out_pipe);
  } else if (!strcmp(argv[0], "singline")) {
    singline(argv[1], in_pipe, out_pipe);
  } else if (!strcmp(argv[0], "nocomment")) {
    nocomment(argv[1], in_pipe, out_pipe);
  } else if (!strcmp(argv[0], "lc")) {
    lc(argv[1], in_pipe, out_pipe);
  } else if (!strcmp(argv[0], "firsten")) {
    firsten(argv[1], in_pipe, out_pipe);
  } else if (!strcmp(argv[0], "mostword")) {
    mostword(argv[1], in_pipe, out_pipe);
  } else {
    run_with_exec(argv, in_pipe, out_pipe);
  }
}

/* Same as 'run_function_with_pipeline(...)' with in_pipe and out_pipe set as
 * NULL, that means your commands will be run using pipeline support but, at
 * start, input is STDIN and output is STDOUT.
 */
void run_command_with_pipeline(char *const *pipe_cmds) {
  run_function_with_pipeline(run_command_with_splitting, pipe_cmds, NULL, NULL);
}

// main functions

/* Ctrl+C handler */
void sigint_handler(int i) {
  rl_replace_line("", 0);
  rl_redisplay();
  rl_done = 1;
}

/* A function that does nothing.
 * Used to bypass readline events.
 */
int dummy_event(void) { return 0; }

/* init
 * Should be run at the start of easy-sh
 * Used for initialization of global variables
 */
void init() {
  rl_event_hook = dummy_event;
  signal(SIGINT, sigint_handler);

  // init cwd (current working directory) and prompt
  getcwd(cwd, PATH_MAX);
  sprintf(prompt, "%s%s%s$ ", GRN, cwd, RESET);

  // tab completion
  rl_bind_key('\t', rl_complete);
}

int main(int argc, char *argv[]) {
  init();

  // readline history support
  char hist_path[PATH_MAX + 6];
  sprintf(hist_path, "%s/.hist", cwd);
  read_history(hist_path);
  using_history();

  char *pipe_cmds[PIPE_LIMIT];
  while (1) {
    char *input = readline(prompt);
    if (input == NULL)
      break;
    else if (input[0] == '\0')
      continue;

    add_history(input);
    write_history(hist_path);

    split_pipe_to_commands(input, pipe_cmds);
    if (pipe_cmds[1] == NULL) {
      run_command_with_splitting(pipe_cmds[0], NULL, NULL);
    } else {
      run_command_with_pipeline(pipe_cmds);
    }
    free(input);
  }
  exit(0);
}
