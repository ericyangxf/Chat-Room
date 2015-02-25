all:server client

server:Server.o list.o
	gcc -std=c99 -o Server Server.o list.o -lpthread

Server.o: Server.c list.h Node.h
	gcc -std=c99 -c Server.c -lpthread

list.o: list.c list.h
	gcc -std=c99 -c list.c

client: Client.c
	gcc -o Client Client.c

clean:
	-rm Server.o list.o

spotless:
	-rm Client Server
