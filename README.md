# [WIP] easy-sh
Just another shell using readline that just works. This project is for educational purposes.
- Shell = line editor + parser + execution
  + line editor = readline
  + parser = simply split command using delimeters
  + execution = `fork` and `exec`

## Features
- Line editing using readline (all key handlers readline provides now works: ctrl+e , ctrl+a , ctrl+w, ...)
- Save commands history in `.hist` file using readline (command history navigation using arrow keys)
- Handle ctrl+c with readline
- Easily add your built-in functions
- Pipeline support
- Everything in one file (~250 lines of code)

## Weaknesses
- very simple command parser
  + does not support " and '
  + does not support escape \
  + does not support anything else ...

## compile
`make`

## how to run
`make run`

## tasks
- [x] Command execution
- [x] Command not found
- [x] Built-in functions
  - [x] a (equivalant: `awk '{print $1}' test | grep -v "^$"`), name : fw
  - [x] b (equivalant: `cut -f1 file | sort | uniq -c | sort -rn | head -n1`), name: mostword
  - [x] c (equivalant: `sed -z 's/\s//g' file`), name : singline
  - [x] d (equivalant: `grep -v "\s*#" file`), name: nocomment
  - [x] f (equivalant: `wc -l file`), name: lc
  - [x] g (equivalant: `head -n10 file`), name : firsten
  - [x] cd (https://stackoverflow.com/questions/34998152/how-exactly-does-the-cd-command-work-in-bash)
  - [x] exit command (should exit the shell)
- [x] prompt (pwd, cd)
- [x] print errors in stderr
- [x] commands history (store commands in .hist)

---

Extra:

- [x] ctrl+c (SIGINT)
- [x] up and down keys (navigate over commands history)
- [x] pipe (https://stackoverflow.com/questions/9834086/what-is-a-simple-explanation-for-how-pipes-work-in-bash)

---

Super Extra:

- [ ] Fix todos and test for bugs
- [x] Clean up and refactor project 
- [x] Add Features sections in README.md
- [ ] Add tab completion for Built-in commands
- [x] Add comment and doc
- [x] Add pipeline support for Built-in commands
- [ ] Refactor built-in commands into builtins.c and make it easier to add new commands
- [ ] alias 

