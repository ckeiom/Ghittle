#!/bin/sh

qemu-system-x86_64 	-L . -m 64 -fda ./disk.img -hda ./filesys.img \
					-bios ./qemu-2.5.0/roms/seabios/out/bios.bin \
					-localtime -M pc -curses
