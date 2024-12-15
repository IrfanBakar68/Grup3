CC=gcc
CFLAGS=-Wall -g

all: shell

shell: shell.o
	$(CC) $(CFLAGS) -o shell shell.o

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o shell
