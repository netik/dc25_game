CFLAGS=-g

all: client udpb

client: client.o

client.o:  client.c player.h util.h game_constants.h

udbp: udbp.c

clean:
	rm *.o
	rm client
	rm udbp

