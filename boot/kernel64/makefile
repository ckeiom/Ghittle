NASM64 = nasm -f elf64
GCC64 = gcc -c -m64 -I$(GHITTLE_HOME)/include/ -ffreestanding
LD64 = ld -melf_x86_64 -T ../elf_x86_64.x -nostdlib -e main -Ttext 0x200000
OBJCOPY64 = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJ_DIR = obj

ENTRY_POINT_SRC = ../entry.s
ENTRY_POINT_OBJ = entry.o

CSRC_FILES = $(wildcard ../*.c)
ASRC_FILES = $(wildcard ../*.asm)

COBJ_FILES = $(notdir $(patsubst %.c, %.o, $(CSRC_FILES)))
AOBJ_FILES = $(notdir $(patsubst %.asm,%.o,$(ASRC_FILES)))

OTHER_OBJ_FILES = $(GHITTLE_HOME)/utils/obj/UTILS.o
OTHER_OBJ_FILES += $(GHITTLE_HOME)/device/obj/DEVICE.o
OTHER_OBJ_FILES += $(GHITTLE_HOME)/task/obj/TASK.o
OTHER_OBJ_FILES += $(GHITTLE_HOME)/filesys/obj/FILESYS.o

all: prepare kernel64.bin

prepare: 
	mkdir -p $(OBJ_DIR)

dep:
	make -C $(OBJ_DIR) -f ../makefile internal_dep

internal_build: dep
	make -C $(OBJ_DIR) -f ../makefile kernel64.elf

kernel64.bin: internal_build
	$(OBJCOPY64) $(OBJ_DIR)/kernel64.elf $@


clean:
	rm -f *.bin
	rm -f $(OBJ_DIR)/*.*

$(ENTRY_POINT_OBJ): $(ENTRY_POINT_SRC)
	$(NASM64) -o $@ $<

%.o: ../%.c
	$(GCC64) -c $<

%.o: ../%.asm
	$(NASM64) -o $@ $<

internal_dep:
	$(GCC64) -MM $(CSRC_FILES) > dependency.dep

kernel64.elf: $(ENTRY_POINT_OBJ) $(COBJ_FILES) $(AOBJ_FILES) $(OTHER_OBJ_FILES)
	$(LD64) -o $@ $^

ifeq (dependency.dep, $(wildcard dependancy.dep))
include dependency.dep
endif
