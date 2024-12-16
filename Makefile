#Hüseyin Göbekli (G211210041) 2B 
#Okan Başol (G211210083) 2B 
#Şimal Ece Kazdal (G221210068) 2B
#Muhammed İrfan Bakar (G221210596) 2B 
#Betül Kurt (G221210054) - 2C 
CC=gcc
CFLAGS=-Wall -g

all: shell

shell: shell.o
	$(CC) $(CFLAGS) -o shell shell.o

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o shell
