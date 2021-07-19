#!/bin/sh

clear;
rm -f maybeegl probablyegl gl2;
#gcc -Wall -Wextra -std=c11 -g -O0 maybeegl.c -o maybeegl \
#	-lEGL -lX11 -lGLESv2 "$@";
#gcc -Wall -Wextra -std=c11 -g -O0 probablyegl.c gl.c util.c -o probablyegl \
#	-lEGL -lX11 -lGL -lm -lpthread "$@";
gcc -Wall -Wextra -std=c11 -g -O0 -D USE_GLES2=1 probablyegl.c gl2.c util.c -o gl2 \
	-lEGL -lX11 -lGLESv2 -lm -lpthread "$@";
exit $?;
