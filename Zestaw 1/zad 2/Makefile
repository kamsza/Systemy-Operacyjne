CC = gcc
FLAGS = -Wall -O0

all: static shared

static:
	$(CC) $(FLAGS) -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) -o main ../zad2/main.c lib.a


shared:
	$(CC) $(FLAGS) -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -shared -fPIC -o lib.so lib.o
	$(CC) -o main main.c -L. lib.so -Wl,-rpath=`pwd`


clean:
	rm -f *.o *.a *.so *.txt tmp* main

test_static:
	make clean
	make static
	./main create_table 1000 search_directory /home lib.a tmp1 search_directory /home tAkIpLiKnIeIsTnIeJe tmp2 search_directory ../.. main_d.c tmp3 search_directory ../.. tAkIpLiKnIeIsTnIeJe tmp4 remove_block 1 remove_block 3

test_shared:
	make clean
	make shared
	./main create_table 1000 search_directory /home lib.a tmp1 search_directory /home tAkIpLiKnIeIsTnIeJe tmp2 search_directory ../.. main_d.c tmp3 search_directory ../.. tAkIpLiKnIeIsTnIeJe tmp4 remove_block 1 remove_block 3
