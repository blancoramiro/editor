# editor
Building a text editor just for the kick of it

# emcc

```emcc -o index.html -lGL --preload-file assets -s MIN_WEBGL_VERSION=2 -s USE_GLFW=3 -s GL_ASSERTIONS -s ALLOW_MEMORY_GROWTH=1  -sFULL_ES3 -Wall -O3 --minify 0 -sEXPORTED_FUNCTIONS=_main,_paste_char,_draw -sEXPORTED_RUNTIME_METHODS=ccall edit.c gap.c scroll.c shaders.c```
