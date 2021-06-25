#!/bin/bash

run_build() {
	clear &&
	gcc -O0 -Wall -Wextra -std=c11 -g \
		src/*.c \
		-o obj/delf.exe
}

run_build;
