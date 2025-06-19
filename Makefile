CC=gcc
CFLAGS=-Wall -Wextra -g -pedantic

all: output

output: main.c parse_rescuers.c parse_emergency_types.c parse_env.c
	$(CC) $(CFLAGS) -o output main.c parse_rescuers.c parse_emergency_types.c parse_env.c

run: output
	./output

clean:
	rm -f output *.o