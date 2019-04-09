monitor:
	gcc -Wall -c monitor.c -o monitor.o
	ar rcs monitor.a monitor.o


main:
	gcc -Wall -o main main.c monitor.c

tester:
	gcc -Wall -o tester tester.c

files:
	mkdir dir
	touch -t 03211112 dir/file1 dir/file2
	touch dir/file3 dir/file4
	

clean:
	rm -f *.a main tester *.o
	rm -rf dir archive

all: clean files monitor main tester

test:
	./main list 15 load

tester1:
	./tester dir/file1 1 2 8

tester2:
	./tester dir/file2 1 3 11

tester3:
	./tester dir/file3 3 7 8

tester4:
	./tester dir/file4 4 6 3
