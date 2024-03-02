CC=gcc
CFLAGS=-lglfw -lm -lGL -Wall -pedantic
CDEBUGFLAGS=-g

all: default 

default: clean scroll.o gap.o
	$(CC) -o edit scroll.o gap.o shaders.c edit.c $(CFLAGS) 

debug: clean scroll.o gap.o
	$(CC) $(CDEBUGFLAGS) -o edit -g scroll.o gap.o shaders.c edit.c $(CFLAGS) 

gap.o: 
	$(CC) -c gap.c

scroll.o: 
	$(CC) -c scroll.c
clean:
	rm -f scroll.o gap.o edit

