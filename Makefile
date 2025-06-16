CC=gcc
CFLAGS=-Wall -Wextra -g

all: parse_r parse_e

parse_r: parse_rescuers.c
	$(CC) $(CFLAGS) -o parse_r parse_rescuers.c

parse_e: parse_emergency_types.c
	$(CC) $(CFLAGS) -o parse_e parse_emergency_types.c

run: parse_r parse_e
	./parse_r
	./parse_e

clean:
	rm -f parse_r parse_e *.o