CC = gcc
FLAGS = -Wall -O0

all: static shared

static:
	$(CC) $(FLAGS) -c lib.c -o lib.o
	ar rcs lib.a lib.o


shared:
	$(CC) $(FLAGS) -c -fPIC lib.c
	$(CC) $(FLAGS) -shared -fPIC -o lib.so lib.o


clean:
	rm -f *.o *.a *.so main


