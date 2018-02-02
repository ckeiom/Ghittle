#!/bin/sh

qemu-system-x86_64 	-L . -m 64 -fda ./disk.img -hda ./filesys.img \
					-bios ./bios.bin -nographic\
					-localtime -M pc -curses
