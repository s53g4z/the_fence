#!/bin/sh

clear;
#gcc -Wall -Wextra -std=c11 -g -O0 maybeegl.c -o maybeegl \
#	-lEGL -lX11 -lGLESv2 "$@";
gcc -Wall -Wextra -std=c11 -g -O0 probablyegl.c gl.c util.c -o probablyegl \
	-lEGL -lX11 -lGL -lm "$@";
exit $?;
