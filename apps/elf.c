#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "elf.h"
int main()
{
	int fd;
	int i;
	int read_byte;
	struct elf_header header;

	fd = open("./div0", O_RDONLY);

	if(fd < 0)
		return -1;

	read_byte = read(fd, &header, sizeof(struct elf_header));

	if(read_byte != sizeof(struct elf_header))
		return -1;

	printf("%c",header.e_ident[0]);
	printf("%c",header.e_ident[1]);
	printf("%c",header.e_ident[2]);
	printf("%c (2, 64bit)\n",header.e_ident[3]);
	printf("class: %x\n",header.e_ident[4]);
	printf("encoding (1, LSB): %x\n",header.e_ident[5]);
	printf("version (1): %x\n",header.e_ident[6]);
	printf("OS: %x (0, UNIX)\n",header.e_ident[7]);
	printf("type(2, executable): %x\n",header.e_type);
	printf("machine(3e): %x\n", header.e_machine);
	printf("version(1): %x\n", header.e_version);
	printf("entry: %lx\n", header.e_entry);
	printf("program header table offset: %lx\n", header.e_phoff);
	printf("section table offset: %lx\n", header.e_shoff);
	printf("e_flags: %x\n", header.e_flags);
	printf("header size: %x(40)\n", header.e_ehsize);
	printf("program header entry size: %x\n", header.e_phentsize);
	printf("# of program header entry: %x\n", header.e_phnum);
	printf("section header entry size: %x\n", header.e_shentsize);
	printf("# of section header entry: %x\n", header.e_shnum);
	printf("section header entry string index: %x\n", header.e_shstrndx);
	return 0;
}


