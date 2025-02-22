CC = gcc
CFLAGS = -Wall

all: server client

server: server.c protocol.c protocol.h
	$(CC) $(CFLAGS) server.c protocol.c -o server

client: client.c protocol.c timer.c protocol.h
	$(CC) $(CFLAGS) client.c protocol.c timer.c -o client

clean:
	rm -f server client