GHITTLE_HOME:=$(shell pwd)
export GHITTLE_HOME

all: Utils Device Task Memory Filesys Boot Disk.img

Utils:
	@echo
	@echo ========== Build Utils ==========
	@echo
	make -C utils

Device:
	@echo
	@echo ========== Build Devices ==========
	@echo
	make -C device
	
Task:
	@echo
	@echo ========== Build Task ==========
	@echo
	make -C task

Memory:
	@echo
	@echo ========== Build Memory ==========
	@echo
	make -C memory

Filesys:
	@echo
	@echo ========== Build Filesys ==========
	@echo
	make -C filesys

Boot:
	@echo
	@echo ========== Build Boot Loader ==========
	@echo
	make -C boot

Disk.img: boot/kernel16/kernel16.bin \
		  boot/kernel32/kernel32.bin \
		  boot/kernel64/kernel64.bin \
		  filesys.img
	@echo
	@echo ========== Make image ==========
	@echo
	./mkimage/mkimage $^
	@echo
	@echo ========== All Build Complete ==========
	@echo

clean:
	make -C boot clean
	make -C task clean
	make -C device clean
	make -C utils clean
	make -C memory clean
	make -C filesys clean
	rm -f disk.img
	
