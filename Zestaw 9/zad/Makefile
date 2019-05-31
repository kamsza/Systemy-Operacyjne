main:
	gcc -Wall -o main main.c main.h -lpthread

clean:
	rm -f main

all: clean main

test: all
	./main  30 6 4 3

debug: clean
	gcc -Wall -o main main.c main.h -g -lpthread

main2: clean2
	gcc -Wall -o main2 main2.c -lpthread
	./main2 3 10 3

clean2:
	rm -f main2