CC=gcc
CFLAGS=-lglfw -lm -lGL -Wall -pedantic
CDEBUGFLAGS=-g
PATHLIB=lib

all: default 

default: clean scroll.o
	$(CC) -o edit scroll.o shaders.c edit.c $(CFLAGS) 

debug: clean scroll.o
	$(CC) $(CDEBUGFLAGS) -o edit -g scroll.o shaders.c edit.c $(CFLAGS) 

scroll.o: 
	$(CC) -c scroll.c

clean:
	rm -f scroll.o edit

