#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#pragma pack(push, 1)
struct mbr
{
	unsigned char bcode[430];
	unsigned int magic;
	unsigned int reserved_sectors;
	unsigned int metadata_sectors;
	unsigned int total_blocks;
	unsigned char reserved[16 * 4];
	unsigned char bl_sign[2];
};

struct dentry
{
	char name[24];
	unsigned int size;
	unsigned int block_index;
};

#pragma pack(pop)
static long alloc_block(unsigned int* metadata, unsigned int size, unsigned int prev)
{
	unsigned int i;
	unsigned int* pos = metadata;

	for(i = 0; i < (size / sizeof(unsigned int)); i++)
	{
		if(*pos == 0)
		{
			if(prev != 0)
				*(metadata + prev) = i;
			*pos = 0xFFFFFFFF;
			return i;
		}
		pos++;
	}
	return -1;
}

static int update_dentry(struct dentry* dentry, 
						 char* name, unsigned int size,
						 unsigned int block_index)
{
	struct dentry* pos = dentry;
	char* name_pos = name;
	int i;
	long length = strlen(name);

	for(i = 0; i < length; i++)
	{
		if(*name_pos == '/')
			name = name_pos + 1;
		name_pos++;
	}
	
	while((unsigned long)pos < (unsigned long)dentry + 4096)
	{
		if(pos->block_index == 0)
		{
			strcpy(pos->name, name);
			pos->size = size;
			pos->block_index = block_index;
			return 0;
		}
		pos++;
	}
	return -1;
}

int main(int argc, char** argv)
{
	struct mbr mbr;
	int img_fd, source_fd;
	long pos_meta, pos_data;
	unsigned int* metadata;
	int i;
	long total_bytes, written_bytes, to_write;
	char block_buf[4096];
	char dentry_buf[4096];
	unsigned int block_index;


	if(argc < 2)
		goto err_out;

	if((img_fd = open("filesys.img",O_RDWR)) < 0)
	{
		printf("filesys.img does not exist\n");
		goto err_out;
	}

	if(read(img_fd, &mbr, sizeof(struct mbr)) != sizeof(struct mbr))
		goto err_out;

	if(mbr.magic != 0x7E38CF10)
	{
		printf("Format MBR first, you may do it by just booting with [filesys.img] as hda\n");
		goto err_out;
	}

	pos_meta = lseek(img_fd, mbr.reserved_sectors + 512, SEEK_SET);

	if(pos_meta < 0)
		goto err_out;

	metadata = (unsigned int*)malloc(mbr.metadata_sectors * 512);

	if(!metadata)
		goto err_out;

	if(mbr.metadata_sectors * 512 !=
	   read(img_fd, metadata, mbr.metadata_sectors * 512))
		goto err_out;

	pos_data = pos_meta + mbr.metadata_sectors * 512;

	if(read(img_fd, dentry_buf, 4096) != 4096)
		goto err_out;

	for(i = 1; i < argc; i++)
	{
		source_fd = open(argv[i], O_RDONLY);
		if(source_fd < 0)
			goto err_out;
		written_bytes = to_write = 0;
		total_bytes = lseek(source_fd, 0, SEEK_END);
		lseek(source_fd, 0, SEEK_SET);
		block_index = 0;

		while(written_bytes < total_bytes)
		{
			to_write = total_bytes - written_bytes;
			if(to_write > 4096)
				to_write = 4096;
			block_index = alloc_block(metadata, mbr.metadata_sectors * 512, 
									  block_index);
			if(block_index < 0)
				goto err_out;

			if(!written_bytes)
			{
				if(update_dentry((struct dentry *)dentry_buf, argv[i], 
							     total_bytes, block_index) < 0)
					goto err_out;
			}

			lseek(img_fd, pos_data + block_index * 4096, SEEK_SET);
			
	//printf("%d %d %d\n", to_write, block_index, read(source_fd, block_buf, to_write));
			if(to_write != read(source_fd, block_buf, to_write))
				goto err_out;

			if(to_write != write(img_fd, block_buf, to_write))
				goto err_out;

			written_bytes += to_write;
		}
		close(source_fd);
	}

	lseek(img_fd, pos_meta, SEEK_SET);
	if((mbr.metadata_sectors * 512) != 
	   write(img_fd, metadata, mbr.metadata_sectors * 512))
		goto err_out;

	lseek(img_fd, pos_data, SEEK_SET);

	if(write(img_fd, dentry_buf, 4096) != 4096)
		goto err_out;

	close(img_fd);

	return 0;

err_out:
	printf("An error detected\n");
	return -1;
}



