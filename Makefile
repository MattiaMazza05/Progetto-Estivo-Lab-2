CC=gcc
CFLAGS=-Wall -Wextra -g -pedantic -std=c11
LIBS = -lpthread

OBJS=parse_rescuers.o parse_emergency_types.o parse_env.o log.o gestione_thread.o

all: server client

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

server: main.o $(OBJS)
	$(CC) $(CFLAGS) -o server main.o $(OBJS)

client: client.o $(OBJS)
	$(CC) $(CFLAGS) -o client client.o $(OBJS)

run: server
	./server

clean:
	rm -f server client *.o *.log