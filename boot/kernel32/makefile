NASM = nasm
GCC32 = gcc -c -m32 -ffreestanding -I$(GHITTLE_HOME)/include
LD32 = ld -melf_i386 -nostdlib -e main -Ttext 0x10200
OBJCOPY32 = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJ_DIR = obj
SRC_DIR = src
all: prepare kernel32.bin

prepare: 
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/entry.bin: $(SRC_DIR)/entry.s
	$(NASM) -o $@ $<

dep:
	make -C $(OBJ_DIR) -f ../makefile internal_dep

execute_internal_build: dep
	make -C $(OBJ_DIR) -f ../makefile kernel32.elf

$(OBJ_DIR)/kernel32.elf.bin: execute_internal_build
	$(OBJCOPY32) $(OBJ_DIR)/kernel32.elf $@

kernel32.bin: $(OBJ_DIR)/entry.bin $(OBJ_DIR)/kernel32.elf.bin
	cat $^ > $@


clean:
	rm -f *.bin
	rm -f $(OBJ_DIR)/*.*

CENTRY_OBJ_FILE=main.o
CSRC_FILES = $(wildcard ../$(SRC_DIR)/*.c)
ASRC_FILES = $(wildcard ../$(SRC_DIR)/*.asm)
COBJ_FILES = $(subst main.o, , $(notdir $(patsubst %.c, %.o, $(CSRC_FILES))))
AOBJ_FILES = $(notdir $(patsubst %.asm,%.o,$(ASRC_FILES)))



%.o: ../$(SRC_DIR)/%.c
	$(GCC32) -c $<

%.o: $(GHITTLE_HOME)/memory/%.c
	$(GCC32) -c $<

%.o: ../$(SRC_DIR)/%.asm
	$(NASM) -f elf32 -o $@ $<

internal_dep:
	$(GCC32) -MM $(CSRC_FILES) > dependency.dep

kernel32.elf: $(CENTRY_OBJ_FILE) $(COBJ_FILES) $(AOBJ_FILES)
	$(LD32) -o $@ $^

ifeq (dependency.dep, $(wildcard dependancy.dep))
include dependency.dep
endif
