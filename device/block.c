#include <block.h>
#include <hdd.h>
#include <filesys.h>

int block_read(unsigned int offset, unsigned char* buf)
{
	return read_hdd_sector(HDD_PRIMARY, HDD_MASTER, 
			               (offset * FS_SECTORS_PER_BLOCK) + block_base,
			               FS_SECTORS_PER_BLOCK, buf);
}

int block_write(unsigned int offset, unsigned char* buf)
{
	return write_hdd_sector(HDD_PRIMARY, HDD_MASTER,
			                (offset * FS_SECTORS_PER_BLOCK) + block_base,
							FS_SECTORS_PER_BLOCK, buf);
}

int block_write_zero(unsigned int offset)
{
	char buf[FS_BLOCK_SIZE] = {0, };
	return write_hdd_sector(HDD_PRIMARY, HDD_MASTER,
							(offset * FS_SECTORS_PER_BLOCK) + block_base,
							FS_SECTORS_PER_BLOCK, buf);
}
