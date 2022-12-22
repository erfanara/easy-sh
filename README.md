# [WIP] easy-sh
Just another shell that just works. This project is for educational purposes.

## compile
`make`

## how to run
`make run`

## tasks
- [x] Command execution
- [ ] Command not found
- [ ] Command executed correctly 
- [ ] Built-in functions
  - [ ] a (equivalant: `awk '{print $1}' test | grep -v "^$"`)
  - [ ] b (equivalant: `cut -f1 file | sort | uniq -c | sort -rn | head -n1`)
  - [ ] c (equivalant: `sed -z 's/\s//g' file`)
  - [ ] d (equivalant: `grep -v "\s*#" file`)
  - [ ] f (equivalant: `wc -l file`)
  - [ ] g (equivalant: `head -n10 file`)
  - [ ] cd
- [ ] prompt (pwd, cd)
- [ ] print errors in stderr
- [ ] commands history (store commands in .hist)
---
- [ ] ctrl+c (SIGTERM)
- [ ] up and down keys (navigate over commands history)
- [ ] pipe
