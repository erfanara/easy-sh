# [WIP] easy-sh
Just another shell that just works. This project is for educational purposes.

## compile
`make`

## how to run
`make run`

## tasks
- [x] Command execution
- [x] Command not found
- [ ] Built-in functions
  - [x] a (equivalant: `awk '{print $1}' test | grep -v "^$"`), name : fw
  - [ ] b (equivalant: `cut -f1 file | sort | uniq -c | sort -rn | head -n1`)
  - [ ] c (equivalant: `sed -z 's/\s//g' file`)
  - [ ] d (equivalant: `grep -v "\s*#" file`)
  - [ ] f (equivalant: `wc -l file`)
  - [ ] g (equivalant: `head -n10 file`)
  - [x] cd (https://stackoverflow.com/questions/34998152/how-exactly-does-the-cd-command-work-in-bash)
  - [x] exit command (should exit the shell)
- [x] prompt (pwd, cd)
- [x] print errors in stderr
- [x] commands history (store commands in .hist)

---

Extra:

- [x] ctrl+c (SIGINT)
- [x] up and down keys (navigate over commands history)
- [ ] pipe (https://stackoverflow.com/questions/9834086/what-is-a-simple-explanation-for-how-pipes-work-in-bash)

---

Super Extra:

- [ ] Clean up and refactor project (Add abstractions like readline+bash)
- [ ] Add Features sections in README.md
- [ ] One-file code (is a good idea?)

