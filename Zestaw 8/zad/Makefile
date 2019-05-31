main:
	gcc -Wall -o main main.c -pthread -lm

tests: clean
	gcc -Wall -o tests tests.c

filter_generator:
	gcc -Wall -o filter_generator filter_generator.c -lm

clean:
	rm -f main tests results/* filter_generator

all: clean main tests filter_generator

filters: filter_generator
	./filter_generator filter1 3
	./filter_generator filter2 10
	./filter_generator filter3 25
	./filter_generator filter4 50
	./filter_generator filter5 65

test:
	./tests coins
