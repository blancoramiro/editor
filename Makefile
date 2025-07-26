CC=gcc
EMCC=emcc
CFLAGS=-lglfw -lm -lGL -Wall -pedantic
OPT=03
CDEBUGFLAGS=-g
OUTNAME=editor

$(OUTNAME): edit.c edit.h scroll.o gap.o linenumber.o shaders.c
	$(CC) -O$(OPT) -o $(OUTNAME) scroll.o gap.o linenumber.o edit.c $(CFLAGS) 

emcc: 
	#$(EMCC) -o editor.js \

	$(EMCC) -o test/editor.html \
		-lGL \
		--preload-file assets \
		--extern-pre-js editor_extra_pre.js \
		--extern-post-js editor_extra_post.js \
		-s MIN_WEBGL_VERSION=2 \
		-s USE_GLFW=3 \
		-s GL_ASSERTIONS \
		-s ALLOW_MEMORY_GROWTH=1 \
		-sFULL_ES3 \
		-Wall \
		-O$(OPT) \
		--minify 0 \
		-sEXPORTED_FUNCTIONS=_main,_paste_char,_draw \
		-sEXPORTED_RUNTIME_METHODS=ccall \
		edit.c gap.c scroll.c linenumber.c
#	cp editor.js.mod editor.js

debug: scroll.o gap.o linenumber.o
	$(CC) $(CDEBUGFLAGS) -O$(OPT) -o $(OUTNAME) -g scroll.o gap.o linenumber.o edit.c $(CFLAGS) 

gap.o: gap.c gap.h
	$(CC) -O$(OPT) -c gap.c

scroll.o: scroll.c scroll.h
	$(CC) -O$(OPT) -c scroll.c

linenumber.o: linenumber.c linenumber.h
	$(CC) -O$(OPT) -c linenumber.c
clean:
	rm -f scroll.o gap.o linenumber.o $(OUTNAME) $(OUTNAME).wasm $(OUTNAME).html $(OUTNAME).js $(OUTNAME).data

all: default emcc

.PHONY: all clean debug emcc
