CC = gcc
FLAGS = -Wall

all: static shared

static:
	$(CC) $(FLAGS) -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) $(FLAGS) -o main ../zad2/main.c lib.a

static_o:
	$(CC) $(FLAGS) -O -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) $(FLAGS) -O -o main ../zad2/main.c lib.a

static_o1:
	$(CC) $(FLAGS) -O1 -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) $(FLAGS) -O1 -o main ../zad2/main.c lib.a

static_os:
	$(CC) $(FLAGS) -Os -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) $(FLAGS) -Os -o main ../zad2/main.c lib.a

static_o2:
	$(CC) $(FLAGS) -O2 -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) $(FLAGS) -O2 -o main ../zad2/main.c lib.a

static_o3:
	$(CC) $(FLAGS) -O3 -c ../zad1/lib.c -o lib.o
	ar rcs lib.a lib.o
	$(CC) $(FLAGS) -O3 -o main ../zad2/main.c lib.a

shared:
	$(CC) $(FLAGS) -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -shared -fPIC -o lib.so lib.o
	$(CC) -o main ../zad2/main.c -L. lib.so -Wl,-rpath=`pwd`

shared_o:
	$(CC) $(FLAGS) -O -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O -shared -fPIC -o lib.so lib.o
	$(CC) -o main ../zad2/main.c -L. lib.so -Wl,-rpath=`pwd`

shared_o1:
	$(CC) $(FLAGS) -O1 -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O1 -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -O1 -o main ../zad2/main.c -L. lib.so -Wl,-rpath=`pwd`

shared_os:
	$(CC) $(FLAGS) -Os -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -Os -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -Os -o main ../zad2/main.c -L. lib.so -Wl,-rpath=`pwd`

shared_o2:
	$(CC) $(FLAGS) -O2 -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O2 -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -O2 -o main ../zad2/main.c -L. lib.so -Wl,-rpath=`pwd`

shared_o3:
	$(CC) $(FLAGS) -O3 -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O3 -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -O3 -o main ../zad2/main.c -L. lib.so -Wl,-rpath=`pwd`

dynamic:
	$(CC) $(FLAGS) -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -o main_d ../zad3a/main_d.c -I . -ldl

dynamic_o1:
	$(CC) $(FLAGS) -O1 -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O1 -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -O1 -o main_d ../zad3a/main_d.c -I . -ldl

dynamic_os:
	$(CC) $(FLAGS) -Os -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -Os -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -Os -o main_d ../zad3a/main_d.c -I . -ldl

dynamic_o2:
	$(CC) $(FLAGS) -O2 -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O2 -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -O2 -o main_d ../zad3a/main_d.c -I . -ldl

dynamic_o3:
	$(CC) $(FLAGS) -O3 -c -fPIC ../zad1/lib.c
	$(CC) $(FLAGS) -O3 -shared -fPIC -o lib.so lib.o
	$(CC) $(FLAGS) -O3 -o main_d ../zad3a/main_d.c -I . -ldl

clean:
	rm -f *.o *.a *.so *.txt tmp* main

test_static:
	./main create_table 1000 search_directory /home lib.a tmp1 search_directory /home tAkIpLiKnIeIsTnIeJe tmp2 search_directory ../.. main_d.c tmp3 search_directory ../.. tAkIpLiKnIeIsTnIeJe tmp4 remove_block 1 remove_block 3

test_shared:
	./main create_table 1000 search_directory /home lib.a tmp1 search_directory /home tAkIpLiKnIeIsTnIeJe tmp2 search_directory ../.. main_d.c tmp3 search_directory ../.. tAkIpLiKnIeIsTnIeJe tmp4 remove_block 1 remove_block 3

test_dynamic:
	./main_d create_table 1000 search_directory /home lib.a tmp1 search_directory /home tAkIpLiKnIeIsTnIeJe tmp2 search_directory ../.. main_d.c tmp3 search_directory ../.. tAkIpLiKnIeIsTnIeJe tmp4 remove_block 1 remove_block 3
