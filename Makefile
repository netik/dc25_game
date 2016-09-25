CFLAGS=-g -lm

all: client udpb

client: client.c 

client.c:  player.h util.h

udbp: udbp.c

clean:
	rm client
	rm udbp

