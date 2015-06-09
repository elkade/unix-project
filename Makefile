all: server client

client: client.o communication.o
	gcc -Wall -o client client.o communication.o -lrt -lpthread -lm

client.o: client.c header.h
	gcc -Wall -o client.o -c client.c
	
server: server.o communication.o
	gcc -Wall -o server server.o communication.o -lrt -lpthread -lm

server.o: server.c header.h
	gcc -Wall -o server.o -c server.c

communication.o: communication.c
	gcc -Wall -o communication.o -c communication.c

.PHONY: clean

clean:
	rm -f server.o client.o communication.o communication server client 
