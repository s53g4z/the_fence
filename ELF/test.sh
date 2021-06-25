#!/bin/sh

run() {
	clear;
	local output1="`./obj/delf.exe ./obj/delf.exe /usr/lib/libfreetype.so`";
	local output2="`./obj/delf.exe ./kate/usr/bin/kate`";
	echo "$output1";
	echo "$output2";
	echo;
	echo "$output1" |md5sum;
	echo "$output2" |md5sum;
}

run
