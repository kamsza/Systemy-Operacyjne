CC = gcc -Wall

server: server.c
	$(CC) -o server server.c -lrt
	
client: client.c
	$(CC) -o client client.c -lrt
	
all: clean server client

clean:
	rm -f *.o server client

test: all
	gnome-terminal -t "SERVER" -e "./server"
	gnome-terminal -t "CLIENT" -e "./client"
	gnome-terminal -t "CLIENT" -e "./client"
	gnome-terminal -t "CLIENT" -e "./client"
