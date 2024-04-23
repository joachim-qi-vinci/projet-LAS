CC=gcc

CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

ALL= server

all: $(ALL)

server : server.o utils_v1.o game.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o game.o

server.o: server.c server.h
	$(CC) $(CCFLAGS) -c server.c

utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c

game.o: game.c game.h utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c game.c

clean:
	rm -f *.o
	rm -f $(ALL)
