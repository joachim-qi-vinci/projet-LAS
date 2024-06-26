CC=gcc

CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

ALL= server client

all: $(ALL)

client: client.o utils_v1.o ipc.o network.o game.o
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o ipc.o network.o game.o

client.o: client.c client.h messages.h game.h
	$(CC) $(CCFLAGS) -c client.c

server: server.o utils_v1.o game.o network.o ipc.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o game.o network.o ipc.o

server.o: server.c server.h ipc.h
	$(CC) $(CCFLAGS) -c server.c

network.o: network.c network.h
	$(CC) $(CCFLAGS) -c network.c

utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c

game.o: game.c game.h utils_v1.h messages.h
	$(CC) $(CCFLAGS) -c game.c

ipc.o: ipc.c ipc.h utils_v1.h
	$(CC) $(CCFLAGS) -c ipc.c

clean:
	rm -f *.o
	rm -f $(ALL)
