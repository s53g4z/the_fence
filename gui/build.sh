#!/bin/sh

clear;
rm -f gl2 gui;
gcc -Wall -Wextra -std=c11 -g -O0 -D USE_GLES2=1 \
	probablyegl.c gl2.c util.c -o gui \
	-lEGL -lX11 -lGLESv2 -lm -lpthread "$@";
exit $?;
