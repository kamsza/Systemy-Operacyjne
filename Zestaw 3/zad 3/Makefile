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
	gnome-terminal -e "./tester dir/file1 1 3 20000"
	gnome-terminal -e "./tester dir/file2 1 2 30000" 
	gnome-terminal -e "./tester dir/file3 1 2 500"
	./main list 10 load 1 100000
   

