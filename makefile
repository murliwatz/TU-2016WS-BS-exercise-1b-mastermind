CC			=	gcc
CFLAGS	=	-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g

all: server.o
	$(CC) $(CFLAGS) -o server server.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f server
	rm -f -R *.o
