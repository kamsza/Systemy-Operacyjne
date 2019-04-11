compile:
	gcc -Wall -o master master.c
	gcc -Wall -o slave slave.c

clear:
	rm -f *.o master slave fifo

all: clear compile
	gnome-terminal -e "./master fifo"
	gnome-terminal -e "./slave fifo 4"
	gnome-terminal -e "./slave fifo 5"
	./slave fifo 6
