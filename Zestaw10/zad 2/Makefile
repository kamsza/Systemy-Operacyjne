server:
	gcc -Wall server.c -o server -lpthread

client:
	gcc -Wall client.c -o client -lpthread

clean:
	rm -f server client

all: clean server client


# TEST
# make all
# ./server 40000 gn
# ./client klient_1 UNIX gn
# ./client klient_2 INET 40000 0.0.0.0
# t/1 t/2 t/3 t/4 t/5 t/6 t/6
