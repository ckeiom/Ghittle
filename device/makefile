NASM64 = nasm -f elf64
GCC64 = gcc -c -m64 -I$(GHITTLE_HOME)/include/ -ffreestanding
LD64 = ld -r

OBJ_DIR = obj

CSRC_FILES = $(wildcard ../*.c)
ASRC_FILES = $(wildcard ../*.asm)
COBJ_FILES = $(notdir $(patsubst %.c, %.o, $(CSRC_FILES)))
AOBJ_FILES = $(notdir $(patsubst %.asm, %.o, $(ASRC_FILES)))

all: prepare Device

prepare:
	mkdir -p $(OBJ_DIR)

%.o: ../%.c
	$(GCC64) -c $< -o $@

%.o: ../%.asm
	$(NASM64) -o $@ $<

dep: 
	make -C $(OBJ_DIR) -f ../makefile internal_dep

internal_dep:
	$(GCC64) -MM $(CSRC_FILES) > dependency.dep

internal_build: dep
	make -C $(OBJ_DIR) -f ../makefile DEVICE.o

DEVICE.o: $(COBJ_FILES) $(AOBJ_FILES)
	$(LD64) -o DEVICE.o $(COBJ_FILES) $(AOBJ_FILES)

Device: internal_build

clean:
	rm -rf $(OBJ_DIR)

ifeq (dependency.dep, $(wildcard dependency.dep))
include dependency.dep
endif
