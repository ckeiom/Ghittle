all:
	make -C kernel16
	make -C kernel32
	make -C kernel64

clean: 
	rm -f boot.bin
	rm -f ./kernel16/kernel16.bin
	rm -f ./kernel32/kernel32.bin
	rm -f ./kernel64/kernel64.bin
