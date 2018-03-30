#include <filesys.h>
#include <dir.h>
#include <string.h>
#include <kmem.h>
#include <file.h>
#include <block.h>

int file_create(const char *name, struct dentry *dentry, int *dentry_index)
{
	unsigned int block;
	
	block = filesys_alloc_block(FS_LAST_BLOCK);
	if(block == FS_LAST_BLOCK)
		return -1; 

	*dentry_index = find_free_dentry();
	if(*dentry_index < 0)
	{
		set_mte(block, FS_FREE_BLOCK);
		return -1;
	}

	strcpy(dentry->name, (char*)name);
	
	dentry->block_index = block;
	dentry->size = 0;

	if(set_de(*dentry_index, dentry) < 0)
	{
		set_mte(block, FS_FREE_BLOCK);
		return -1;
	}

	return 0;
}

struct file *file_open(const char *name, const char *mode)
{
	struct dentry dentry;
	int dentry_offset;
	int filename_length;
	unsigned int second_block;
	struct file *file;

	filename_length = strlen(name);
	if((filename_length > (sizeof(dentry.name) - 1 )) || (!filename_length))
		return 0;

	dentry_offset = find_dentry(name, &dentry);
	if(dentry_offset < 0)
	{
		if(mode[0] == 'r')
			return 0;

		if(file_create(name, &dentry, &dentry_offset) < 0)
			return 0;
	}
	else if(mode[0] == 'w')
	{
		if(get_mte(dentry.block_index, &second_block) < 0)
			return 0;

		if(set_mte(dentry.block_index, FS_LAST_BLOCK) < 0)
			return 0;

		/* Open with 'w' option eliminates all blocks of the file except the first one */
		if(free_blocks_all(second_block) < 0)
			return 0;

		dentry.size = 0;
		if(set_de(dentry_offset, &dentry) < 0)
			return 0;
	}

	file = kmalloc(sizeof(struct file));
	file->type = FS_TYPE_FILE;
	file->dentry_offset = dentry_offset;
	file->size = dentry.size;
	file->block_index = dentry.block_index;
	file->curr_block_index = dentry.block_index;
	file->prev_block_index = dentry.block_index;
	file->curr_offset = 0;

	if(mode[0] == 'a')
		file_seek(file, 0, FS_SEEK_END);

	return file;
}

int file_close(struct file* file)
{
	if(!file || (file->type != FS_TYPE_FILE))
		return -1;

	kfree(file);
	return 0;
}

unsigned int file_read(void *buf, unsigned int size, struct file *file)
{
	unsigned int total_bytes;
	unsigned int read_bytes = 0;
	unsigned int offset;
	unsigned int copy_size;
	unsigned int next_block;
	char rbuf[FS_BLOCK_SIZE];

	if((file == 0) || (file->type != FS_TYPE_FILE))
		return 0;

	if((file->curr_offset >= file->size) || (file->curr_block_index == FS_LAST_BLOCK))
		return 0;

	total_bytes = MIN(size, file->size - file->curr_offset);

	while(read_bytes < total_bytes)
	{
		if(block_read(file->curr_block_index, rbuf) < 0) 
			break;

		offset = file->curr_offset % FS_BLOCK_SIZE;

		copy_size = MIN(FS_BLOCK_SIZE - offset, total_bytes - read_bytes);
		memcpy((char *)buf + read_bytes, rbuf + offset, copy_size);

		read_bytes += copy_size;
		file->curr_offset += copy_size;

		if(!(file->curr_offset % FS_BLOCK_SIZE))
		{
			if(get_mte(file->curr_block_index, &next_block) < 0)
				break;
			
			file->prev_block_index = file->curr_block_index;
			file->curr_block_index = next_block;
		}
	}
	return read_bytes;
}

unsigned int file_write(const void *buf, unsigned int size, struct file *file)
{
	unsigned int total_bytes;
	unsigned int written_bytes = 0;
	unsigned int offset;
	unsigned int copy_size;
	unsigned int new_block;
	unsigned int next_block;
	char write_buf[FS_BLOCK_SIZE];
	struct dentry dentry;

	if(!file || (file->type != FS_TYPE_FILE))
		return 0;

	total_bytes = size;

	while(written_bytes < total_bytes)
	{
		if(file->curr_block_index == FS_LAST_BLOCK)
		{
			new_block = filesys_alloc_block(file->curr_block_index);
			
			if(new_block == FS_LAST_BLOCK)
				break;

			file->curr_block_index = new_block;

			memset(write_buf, 0, FS_BLOCK_SIZE);
		}
		else if(((file->curr_offset % FS_BLOCK_SIZE)) ||
		        ((total_bytes - written_bytes) < FS_BLOCK_SIZE))
			if(block_read(file->curr_block_index, write_buf) < 0)
				break;

		offset = file->curr_offset % FS_BLOCK_SIZE;
		copy_size = MIN(FS_BLOCK_SIZE - offset, total_bytes - written_bytes);
		memcpy(write_buf + offset, (char *)buf + written_bytes, copy_size);

		if(block_write(file->curr_block_index, write_buf) < 0)
			break;

		written_bytes += copy_size;
		file->curr_offset += copy_size;

		if(!(file->curr_offset % FS_BLOCK_SIZE))
		{
			if(get_mte(file->curr_block_index, &next_block) < 0)
				break;

			file->prev_block_index = file->curr_block_index;
			file->curr_block_index = next_block;
		}
	}

	if(file->size < file->curr_offset)
	{
		if(get_de(file->dentry_offset, &dentry) < 0)
		{
			printk("[Fatal]Increased file size is not updated properly\n");
			return 0;
		}

		dentry.size = file->curr_offset;

		if(set_de(file->dentry_offset, &dentry) < 0)
		{
			printk("[Fatal]Increased file size is not updated properly\n");
			return 0;
		}	
		file->size = file->curr_offset;
	}
	return written_bytes;
}

int file_fill_zero(struct file *file, unsigned int from, unsigned int to)
{
	char buf[FS_BLOCK_SIZE] = {0, };
	unsigned int total_bytes = to - from;
	unsigned int written_bytes = 0;
	unsigned int to_copy;

	if(!file)
		return 0;

	/* buf = ( unsigned char * ) KMALLOC( FS_CLUSTERSIZE ); */
	/* unsigned char t2[FS_CLUSTERSIZE]; */

	while(written_bytes < total_bytes)
	{
		if(file->curr_offset % FS_BLOCK_SIZE)
			to_copy = FS_BLOCK_SIZE - (file->curr_offset % FS_BLOCK_SIZE);
		else
			to_copy = FS_BLOCK_SIZE;
		
		if(file_write(buf, to_copy, file) != to_copy)
			return 0;

		written_bytes += to_copy;
	}
	/* KFREE( buf ); */
	
	return written_bytes;
}

int file_seek(struct file *file, int offset, int origin)
{
	unsigned int new_offset;
	unsigned int new_block_offset;
	unsigned int curr_block_offset;
	unsigned int last_block_offset;
	unsigned int moves;
	unsigned int i;
	unsigned int block_index;
	unsigned int prev_block_index;
	unsigned int curr_block_index;

	if(!file || (file->type != FS_TYPE_FILE))
		return -1;

	switch(origin)
	{
		case FS_SEEK_SET:
			if(offset < 0)
				new_offset = 0;
			else
				new_offset = offset;
			break;
		case FS_SEEK_CUR:
			if((offset < 0) && (file->curr_offset <= (unsigned int)-offset))
				new_offset = 0;
			else
				new_offset = file->curr_offset + offset;
			break;
		case FS_SEEK_END:
			if((offset < 0) && (file->size <= (unsigned int)-offset))
				new_offset = 0;
			else
				new_offset = file->size + offset;
			break;
	}

	last_block_offset = file->size / FS_BLOCK_SIZE;
	new_block_offset = new_offset / FS_BLOCK_SIZE;
	curr_block_offset = file->curr_offset / FS_BLOCK_SIZE;

	if(last_block_offset < new_block_offset)
	{
		moves = last_block_offset - curr_block_offset;
		block_index = file->curr_block_index;
	}
	else if(curr_block_offset <= new_block_offset)
	{
		moves = new_block_offset - curr_block_offset;
		block_index = file->curr_block_index;
	}
	else
	{
		moves = new_block_offset;
		block_index = file->block_index;
	}

	curr_block_index = block_index;

	for(i = 0; i < moves; i++)
	{
		prev_block_index = curr_block_index;

		if(get_mte(prev_block_index, &curr_block_index) < 0)
			return -1;
	}

	if(moves > 0)
	{
		file->prev_block_index = prev_block_index;
		file->curr_block_index = curr_block_index;
	}
	else if(block_index == file->block_index)
	{
		file->prev_block_index = file->block_index;
		file->curr_block_index = file->block_index;
	}

	if(last_block_offset < new_block_offset)
	{
		file->curr_offset = file->size;
		if(file_fill_zero(file, file->curr_offset, new_offset) < 0)
			return -1;	
	}

	file->curr_offset = new_offset;

	return 0;
}

int file_is_open(const struct dentry *dentry)
{
	return 1;
}

int file_remove(const char *name)
{
	struct dentry dentry;
	int dentry_offset;
	int filename_length;

	filename_length = strlen(name);
	if((filename_length > (sizeof(dentry.name) - 1)) || !filename_length)
		return -1;

	dentry_offset = find_dentry(name, &dentry);

	if(dentry_offset < 0)
		return -1;

	if(free_blocks_all(dentry.block_index) < 0)
		return -1;

	memset(&dentry, 0, sizeof(struct dentry));
	if(set_de(dentry_offset, &dentry) < 0)
		return -1;

	return 0;
}


