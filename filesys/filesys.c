/* This file is initally written by Hyunsub Song on 2017.12.26 */
#include <filesys.h>
#include <hdd.h>
#include <block.h>
#include <console.h>
static struct filesys filesys;

int init_filesys(void)
{
	memset(&filesys, 0, sizeof(filesys));
	init_mutex(&filesys.mutex);

	if(init_hdd() < 0)
		return -1;

	if(mount() < 0)
		return -1;


	return 0;
}

int mount(void)
{
	struct mbr mbr;

	lock(&filesys.mutex);

	if(read_hdd_sector(HDD_PRIMARY, HDD_MASTER, 0, 1, (char*)&mbr) < 0)
		goto err_out;

	if(mbr.magic != FS_MAGIC)
	{
		if(format() < 0)
			goto err_out;
	}
	filesys.mounted = 1;
	filesys.reserved_sectors = mbr.reserved_sectors;
	filesys.metadata_addr = mbr.reserved_sectors + 1;
	filesys.metadata_sectors = mbr.metadata_sectors;
	filesys.data_addr = mbr.reserved_sectors + mbr.metadata_sectors + 1;
	filesys.total_blocks = mbr.total_blocks;
	filesys.last_allocated_index = 0;	
	block_base = filesys.data_addr;
	unlock(&filesys.mutex);
	return 0;

err_out:
	unlock(&filesys.mutex);
	return -1;
}

int format(void)
{
	struct hdd_info hdd_info;
	struct mbr mbr;
	char buf[HDD_SECTOR_SIZE];
	unsigned int total_sectors;
	unsigned int total_blocks;
	unsigned int metadata_sectors, data_sectors;
	unsigned int i;

	lock(&filesys.mutex);

	if(read_hdd_info(HDD_PRIMARY, HDD_MASTER, &hdd_info) < 0)
		goto err_out;
	
	total_sectors = hdd_info.total_sector;
	total_blocks = total_sectors / FS_SECTORS_PER_BLOCK;

	metadata_sectors = (total_blocks + 127) / 128;
	data_sectors = total_sectors - metadata_sectors - 1;
	total_blocks = data_sectors / FS_SECTORS_PER_BLOCK;
	metadata_sectors = (total_blocks + 127) / 128;

	if(read_hdd_sector(HDD_PRIMARY, HDD_MASTER, 0, 1, (char*)&mbr) < 0)
		goto err_out;

	memset(mbr.part, 0, sizeof(mbr.part));
	mbr.magic = FS_MAGIC;
	mbr.reserved_sectors = 0;
	mbr.metadata_sectors = metadata_sectors;
	mbr.total_blocks = total_blocks;

	if(write_hdd_sector(HDD_PRIMARY, HDD_MASTER, 0, 1, (char*)&mbr) < 0)
		goto err_out;

	memset(buf, 0, sizeof(buf));

	/* Following "+ FS_SECTORS_PER_BLOCK" initiates root directory clean */ 
	for(i = 0; i < (metadata_sectors + FS_SECTORS_PER_BLOCK); i++)
	{
		if(i == 0)
			((unsigned int *)buf)[0] = FS_LAST_BLOCK;
		else	
			((unsigned int *)buf)[0] = FS_FREE_BLOCK;

		if (write_hdd_sector(HDD_PRIMARY, HDD_MASTER, i + 1, 1, buf) < 0)
			goto err_out;
	}

	unlock(&filesys.mutex);
	return 0;

err_out:
	unlock(&filesys.mutex);
	return -1;
}


static int read_metadata_sector(unsigned int offset, unsigned char* buf)
{
	return read_hdd_sector(HDD_PRIMARY, HDD_MASTER, 
						   offset + filesys.metadata_addr, 1, buf);
}

static int write_metadata_sector(unsigned int offset, unsigned char* buf)
{
	return write_hdd_sector(HDD_PRIMARY, HDD_MASTER,
			                offset + filesys.metadata_addr, 1, buf);
}

unsigned int filesys_alloc_block(unsigned int prev_index)
{
	unsigned int buf[HDD_SECTOR_SIZE];
	unsigned int last_index;
	unsigned int current_sector;
	unsigned int i, j;

	lock(&filesys.mutex);

	if(!filesys.mounted)
		goto err_out;

	last_index = (filesys.last_allocated_index + 1) % filesys.total_blocks;
	current_sector = (last_index / FS_MTE_PER_SECTOR);

	for(i = 0; i < filesys.metadata_sectors; i++)
	{
		if(read_metadata_sector(current_sector, (char*)buf) < 0)
			goto err_out;

		for(j = 0; j < FS_MTE_PER_SECTOR; j++)
		{
			if(buf[j] == FS_FREE_BLOCK)
			{
				filesys.last_allocated_index = current_sector * 
											   FS_MTE_PER_SECTOR + j;
				set_mte(filesys.last_allocated_index, FS_LAST_BLOCK);
				if(prev_index != FS_LAST_BLOCK)
					set_mte(prev_index, filesys.last_allocated_index);


				unlock(&filesys.mutex);
				return filesys.last_allocated_index;
			}
		}

		current_sector = (current_sector + 1) % filesys.metadata_sectors;
	}

err_out:
	unlock(&filesys.mutex);
	return FS_LAST_BLOCK;
}

int set_mte(unsigned int block_index, unsigned int data)
{
	unsigned int sector_offset;
	char buf[HDD_SECTOR_SIZE];

	sector_offset = block_index / FS_MTE_PER_SECTOR;

	if(read_metadata_sector(sector_offset, buf) < 0)
		return -1;

	((unsigned int*)buf)[block_index % FS_MTE_PER_SECTOR] = data;
	
	if(write_metadata_sector(sector_offset, buf) < 0)
		return -1;

	return 0;
}

int get_mte(unsigned int block_index, unsigned int *data)
{
	unsigned int sector_offset;
	char buf[HDD_SECTOR_SIZE];

	sector_offset = block_index / FS_MTE_PER_SECTOR;

	if(sector_offset > filesys.metadata_sectors)
		return -1;

	if(read_metadata_sector(sector_offset, buf) < 0)
		return -1;

	*data = ((unsigned int *)buf)[block_index % FS_MTE_PER_SECTOR];

	return 0;
}

/* 
 * Free whole block chain from given block_index 
 * Should avoid passing index in the middle of chain, only the first index 
 */
int free_blocks_all(unsigned int block_index)
{
	unsigned int curr_block;
	unsigned int next_block;

	curr_block = block_index;

	while(curr_block != FS_LAST_BLOCK)
	{
		if(get_mte(curr_block, &next_block) < 0)
			return -1;

		if(set_mte(curr_block, FS_FREE_BLOCK) < 0)
			return -1;

		curr_block = next_block;
	}
	return 0;
}

#include <file.h>
int filesys_test(void)
{
	struct file* file;
	char buf[4096] = "abcd";
	int res;

	file = file_open("a.txt","r");
	file_read(buf, 4096, file);
	printk("%s\n", buf);
	file_close(file);

	file = file_open("b.txt","r");
	file_read(buf, 4096, file);
	printk("%s\n", buf);
	file_close(file);
	file = file_open("c.txt","r");
	file_read(buf, 4096, file);
	printk("%s\n", buf);
	file_close(file);
	file = file_open("large.txt","r");
	file_read(buf, 4096, file);
	printk("%s\n", buf);
	file_close(file);
	printk("done\n");
	return 0;
}










