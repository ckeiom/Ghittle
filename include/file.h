#ifndef __FILE_H__
#define __FILE_H__

#include <dir.h>
struct file
{
	unsigned char type;
	int dentry_offset;
	unsigned int size;
	unsigned int block_index;
	unsigned int curr_block_index;
	unsigned int prev_block_index;
	unsigned int curr_offset;
};


int file_create(const char *name, struct dentry *dentry, int *dentry_index);
struct file* file_open(const char *name, const char *mode);
int file_close(struct file *file);
unsigned int file_read(void *buf, unsigned int size, struct file *file);
unsigned int file_write(const void *buf, unsigned int size, struct file *file );
int file_seek(struct file *file, int offset, int origin);
int file_remove(const char *name);
int file_is_open(const struct dentry *dentry);

#endif
