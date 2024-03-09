CC=gcc
EMCC=emcc
CFLAGS=-lglfw -lm -lGL -Wall -pedantic
OPT=03
CDEBUGFLAGS=-g
OUTNAME=editor

all: default emcc

default: clean scroll.o gap.o
	$(CC) -O$(OPT) -o $(OUTNAME) scroll.o gap.o shaders.c edit.c $(CFLAGS) 

emcc: 
	$(EMCC) -o editor.js -lGL --preload-file assets --extern-pre-js editor_extra_pre.js --extern-post-js editor_extra_post.js -s MIN_WEBGL_VERSION=2 -s USE_GLFW=3 -s GL_ASSERTIONS -s ALLOW_MEMORY_GROWTH=1  -sFULL_ES3 -Wall -O$(OPT) --minify 0 -sEXPORTED_FUNCTIONS=_main,_paste_char,_draw -sEXPORTED_RUNTIME_METHODS=ccall edit.c gap.c scroll.c shaders.c

debug: default
	$(CC) $(CDEBUGFLAGS) -O$(OPT) -o edit -g scroll.o gap.o shaders.c edit.c $(CFLAGS) 

gap.o: 
	$(CC) -c gap.c

scroll.o: 
	$(CC) -c scroll.c
clean:
	rm -f scroll.o gap.o $(OUTNAME) $(OUTNAME).wasm $(OUTNAME).html $(OUTNAME).js $(OUTNAME).data
