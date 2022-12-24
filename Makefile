#OS detection
# ifeq ($(OS),Windows_NT) 
# detected_OS := Windows
# else
# detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
# endif

CC=gcc
compile=$(CC) -Wall -O0 -g -lreadline easy-sh.c -o a.out

# ifeq ($(detected_OS),Windows)
# main : main.c lib/*.c
# 	$(CC) -Wall -static-libgcc -I$(INCLUDES) $(LIBS)  main.c -o main.exe
# endif


# ifeq ($(detected_OS),Linux)
main : easy-sh.c
	$(compile)
# endif


run :
	$(compile)
	./a.out

