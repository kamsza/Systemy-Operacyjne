trucker:
	gcc -Wall -o trucker trucker.c line.c -lpthread -lrt

loader:
	gcc -Wall -o loader loader.c line.c -lpthread -lrt
 
loader_gen:
	gcc -Wall -o loader_gen loader_gen.c line.c -lpthread -lrt
 
clean:
	rm -f trucker loader loader_gen

all: clean trucker loader loader_gen
