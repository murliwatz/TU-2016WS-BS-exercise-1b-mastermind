CC			=	gcc
CFLAGS	=	-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g

all: server client
	$(CC) $(CFLAGS) -o server server.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

server: server.o
	$(CC) $(CFLAGS) -o server server.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

client: client.o
	$(CC) $(CFLAGS) -o client client.o

clean:
	rm -f client
	rm -f server
	rm -f -R *.o