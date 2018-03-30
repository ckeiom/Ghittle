#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <sync.h>
#include <hdd.h>
#include <memutils.h>
#include <string.h>
#include <task.h>

#define FS_MAGIC				0x7E38CF10
#define FS_SECTORS_PER_BLOCK	8
#define FS_LAST_BLOCK			0xFFFFFFFF
#define FS_FREE_BLOCK			0x00000000
#define FS_BLOCK_SIZE		(FS_SECTORS_PER_BLOCK * HDD_SECTOR_SIZE)

/* DE stands for Directory entry */
#define FS_DE_PER_BLOCK		(FS_BLOCK_SIZE / sizeof(struct dentry))

/* MTE stands for Metadata Table Entry in metadata region */
#define FS_MTE_SIZE	4
#define FS_MTE_PER_SECTOR	(HDD_SECTOR_SIZE / FS_MTE_SIZE)
#define FS_MAX_INODE		(TASK_MAX * 3)

#define FS_MAX_FILENAME_LENGTH	24

#define FS_TYPE_FREE			0
#define FS_TYPE_FILE			1
#define FS_TYPE_DIR				2

#define FS_SEEK_SET				0
#define FS_SEEK_CUR				1
#define FS_SEEK_END				2

#define fopen					open_file
#define fread					read_file
#define fwrite					write_file
#define	fseek					seek_file
#define fclose					close_file
#define remove					remove_file
#define opendir					open_dir
#define	readdir					read_dir
#define	rewinddir				rewind_dir
#define closedir				close_dir

#pragma pack( push, 1 )

struct partition
{
	unsigned char bflag;
	unsigned char start_chs_addr[3];
	unsigned char ptype;
	unsigned char end_chs_addr[3];
	unsigned int start_lba_addr;
	unsigned int sector_size;
};

struct mbr
{
	unsigned char bcode[430];
	unsigned int magic;
	unsigned int reserved_sectors;
	unsigned int metadata_sectors;
	unsigned int total_blocks;
	struct partition part[4];
	unsigned char bl_sign[2];
};


struct filesys
{
	unsigned char mounted;
	unsigned int reserved_sectors;		// in sector
	unsigned int metadata_addr;			// in sector
	unsigned int metadata_sectors;		// in sector
	unsigned int data_addr;				// in sector
	unsigned int total_blocks;			// in block
	unsigned int last_allocated_index;	// in block
	struct mutex mutex;
};

#pragma pack( pop )

int init_filesys(void);
int format(void);
int mount(void);
unsigned int filesys_alloc_block(unsigned int prev_index);
int free_blocks_all(unsigned int block_index);
int set_mte(unsigned int block_index, unsigned int data);
int get_mte(unsigned int block_index, unsigned int *data);
static int read_metadata_sector(unsigned int offset, unsigned char *buf);
static int write_metadata_sector(unsigned int offset, unsigned char *buf);

int filesys_test(void);

// do we really need below?
/*
static void *alloc_inode( void );
static void free_inode( struct inode *file );
unsigned char write_zero( struct inode *file, unsigned int count );
*/
#endif
