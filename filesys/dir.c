#include <dir.h>
#include <block.h>
#include <string.h>
#include <kmem.h>
#include <file.h>

int find_free_dentry(void)
{
	struct dentry *dentry;
	char buf[FS_BLOCK_SIZE];
	int i;

	if(block_read(0, buf) < 0)
		return -1;

	dentry = (struct dentry*)buf;
	
	for(i = 0; i < FS_DE_PER_BLOCK; i++)
		if( dentry[i].block_index == DIR_FREE)
			return i;
	return -1;
}

int set_de(int index, struct dentry *dentry)
{
	struct dentry *root_dir;
	char buf[FS_BLOCK_SIZE];

	if((index < 0) || (index >= FS_DE_PER_BLOCK))
		return -1;

	if(block_read(DIR_ROOT, buf) < 0)
		return -1;

	root_dir = (struct dentry *)buf;

	memcpy(root_dir + index, dentry, sizeof(struct dentry));

	if(block_write(0, buf) < 0)
		return -1;

	return 0;
}

int get_de(int index, struct dentry *dentry)
{
	struct dentry *root_dir;
	char buf[FS_BLOCK_SIZE];

	if((index < 0) || (index >= FS_DE_PER_BLOCK))
		return -1;

	if(block_read(DIR_ROOT, buf) < 0)
		return -1;

	root_dir = (struct dentry *)buf;
	memcpy(dentry, root_dir + index, sizeof(struct dentry));

	return 0;
}

int find_dentry(const char *name, struct dentry *dentry)
{
	struct dentry *root_dir;
	char buf[FS_BLOCK_SIZE];
	int i;
	int len;

	if(block_read(DIR_ROOT, buf) < 0)
		return -1;
	
	root_dir = (struct dentry *)buf;
	len = strlen(name);
	
	for(i = 0; i < FS_DE_PER_BLOCK; i++)
	{
		if(strcmp(root_dir[i].name, (char*)name) == 0)
		{
			memcpy(dentry, root_dir + i, sizeof(struct dentry));
			return i;
		}
	}
	return -1;
}

struct file* dir_open(const char *name)
{
	struct file *dir = 0;
	char buf[FS_BLOCK_SIZE];
	struct dentry *dentry;
	int i;

	if(block_read(DIR_ROOT, buf) < 0)
		return 0;
	dentry = (struct dentry*)buf;

	for(i = 0; i < FS_DE_PER_BLOCK; i++)
	{
		if(strcmp(dentry[i].name, (char*)name) == 0)
		{
			dir = (struct file*)kmalloc(sizeof(struct file));
			if(!dir)
				return 0;
			dir->type = FS_TYPE_DIR;
			dir->dentry_offset = i;
			dir->size = dentry[i].size;
			dir->block_index = dentry[i].block_index;
			dir->curr_block_index = dentry[i].block_index;
			dir->prev_block_index = dentry[i].block_index;
			dir->curr_offset = 0;				
		}
	}	
	return dir;
}

#if 0
struct dentry* dir_read(struct file *dir)
{
	struct dir *d_handle;
	struct dentry *dentry;

	if(!dir || (dir->type != FS_TYPE_DIR))
		return 0;

	if((dir->current_offset < 0) || (dir->current_offset >= FS_BLOCK_SIZE))
		return 0;

	dentry = d_handle->dir_buf;
	while(dir->current_offset < FS_BLOCK_SIZE)
	{
		if( dentry[d_handle->current_offset].start_cluster_index != 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return &( dentry[d_handle->current_offset++] );
		}

		d_handle->current_offset++;
	}

	unlock( &( fs_mgr.mut ) );
	return 0;
}

void rewind_dir( struct inode *dir)
{
	struct dir *d_handle;

	if( ( dir == 0 ) || ( dir->type != FS_TYPE_DIR ) )
		return ;

	d_handle = &( dir->d_handle );

	lock( &( fs_mgr.mut ) );

	d_handle->current_offset = 0;

	unlock( &( fs_mgr.mut ) );
}

int close_dir( struct inode *dir )
{
	struct dir *d_handle;

	if( ( dir == 0 ) || ( dir->type != FS_TYPE_DIR ) )
		return -1;

	d_handle = &( dir->d_handle );

	lock( &( fs_mgr.mut ) );

	/* KFREE( d_handle->dir_buf ); */

	free_inode( dir );

	unlock( &( fs_mgr.mut ) );

	return 0;
}

static unsigned char update_dentry( struct file *f_handle )
{
	struct dir_entry dentry;

	if( ( f_handle == 0 ) ||
			( get_dentry_data( f_handle->dentry_offset, &dentry ) == 0 ) )
		return 0;

	dentry.fsize = f_handle->fsize;
	dentry.start_cluster_index = f_handle->start_cluster_index;

	if( set_dentry_data( f_handle->dentry_offset, &dentry ) == 0 )
		return 0;

	return 1;
}

#endif

