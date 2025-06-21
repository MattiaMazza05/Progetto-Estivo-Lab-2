CC=gcc
CFLAGS=-Wall -Wextra -g -pedantic

all: server client

server: main.c parse_rescuers.c parse_emergency_types.c parse_env.c 
	$(CC) $(CFLAGS) -o server main.c parse_rescuers.c parse_emergency_types.c parse_env.c 

client: client.c parse_emergency_types.c  parse_env.c parse_rescuers.c
	$(CC) $(CFLAGS) -o client client.c parse_emergency_types.c parse_env.c parse_rescuers.c
run: server
	./server

clean:
	rm -f server client *.o