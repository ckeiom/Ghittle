#ifndef __DIR_H__
#define __DIR_H__

#include <filesys.h>
#define DIR_ROOT	0
#define DIR_FREE	0

struct dentry
{
	char name[FS_MAX_FILENAME_LENGTH];
	unsigned int size;
	unsigned int block_index;
};


int find_free_dentry(void);

/* de stands for directory entry */
int set_de(int index, struct dentry *dentry);
int get_de(int index, struct dentry *dentry);
int find_dentry(const char* name, struct dentry *dentry);
struct file* dir_open(const char *name);

#endif
