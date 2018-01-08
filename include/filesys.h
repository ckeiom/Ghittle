#ifndef __FILESYS_H__
#define __FILESYS_H__

#include <sync.h>
#include <hdd.h>
#include <memutils.h>
#include <string.h>
#include <task.h>

#define FS_SIGN					0x7E38CF10
#define FS_SECTORSPERCLUSTER	8
#define FS_LASTCLUSTER			0xFFFFFFFF
#define FS_FREECLUSTER			0x00
#define FS_MAXDIRENTRYCOUNT		( ( FS_SECTORSPERCLUSTER * 512 ) / \
		sizeof( struct dir_entry ) )
#define FS_CLUSTERSIZE			( FS_SECTORSPERCLUSTER * 512 )

#define FS_HANDLE_MAXCOUNT		( TASK_MAX_COUNT * 3 )

#define FS_MAXFILENAMELEN		24

#define FS_TYPE_FREE			0
#define FS_TYPE_FILE			1
#define FS_TYPE_DIR				2

#define FS_SEEK_SET				0
#define FS_SEEK_CUR				1
#define FS_SEEK_END				2

/* Functions related on control of HDD */
typedef unsigned char ( *f_read_hdd_info ) ( unsigned char p, unsigned char m,
		struct hdd_info *h_info );
typedef int ( *f_read_hdd_sector ) ( unsigned char p, unsigned char m,
		unsigned int lba, int sector_count, char *buf );
typedef int ( *f_write_hdd_sector ) ( unsigned char p, unsigned char m,
		unsigned int lba, int sector_count, char *buf );

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

#define SEEK_SET				FS_SEEK_SET
#define SEEK_CUR				FS_SEEK_CUR
#define SEEK_END				FS_SEEK_END

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
	unsigned int sign;
	unsigned int reserved_sector_count;
	unsigned int cl_sector_count;
	unsigned int total_cluster_count;
	struct partition part[4];
	unsigned char bl_sign[2];
};

struct dir_entry
{
	char fname[FS_MAXFILENAMELEN];
	unsigned int fsize;
	unsigned int start_cluster_index;
};

struct file
{
	int dentry_offset;
	unsigned int fsize;
	unsigned int start_cluster_index;
	unsigned int current_cluster_index;
	unsigned int previous_cluster_index;
	unsigned int current_offset;
};

struct dir
{
	struct dir_entry *dir_buf;
	int current_offset;
};

struct inode
{
	unsigned char type;
	union
	{
		struct file f_handle;
		struct dir d_handle;
	};
};

#pragma pack( pop )

struct filesys_mgr
{
	unsigned char mounted;
	unsigned int reserved_sector_count;
	unsigned int cla_start_addr;
	unsigned int cla_size;
	unsigned int da_start_addr;
	unsigned int total_cluster_count;
	unsigned int cl_sector_offset;
	struct mutex mut;
	struct inode *inode_pool;
};

unsigned char init_fs( void );
unsigned char format( void );
unsigned char mount( void );
unsigned char get_hdd_info( struct hdd_info *h_info );

static unsigned char read_cl_table( unsigned int offset, unsigned char *buf );
static unsigned char write_cl_table( unsigned int offset, unsigned char *buf );
static unsigned char read_cluster( unsigned int offset, unsigned char *buf ); 
static unsigned char write_cluster( unsigned int offset, unsigned char *buf ); 
static unsigned int find_free_cluster( void );
static unsigned char set_cl_data( unsigned int cluster_index, unsigned int data );
static unsigned char get_cl_data( unsigned int cluster_index, unsigned int *data );
static int find_free_dentry( void );
static unsigned char set_dentry_data( int index, struct dir_entry *dentry );
static unsigned char get_dentry_data( int index, struct dir_entry *dentry );
static int find_dentry( const char *fname, struct dir_entry *dentry );
void get_fs_info( struct filesys_mgr *mgr );

struct inode *open_file( const char *fname, const char *mode );
unsigned int read_file( void *buf, unsigned int size, unsigned int count,
		struct inode *file );
unsigned int write_file( const void *buf, unsigned int size, unsigned int count,
		struct inode *file );
int seek_file( struct inode *file, int offset, int origin );
int close_file( struct inode *file );
int remove_file( const char *fname );
struct inode *open_dir( const char *dname );
struct dir_entry *read_dir( struct inode *dir );
void rewind_dir( struct inode *dir );
int close_dir( struct inode *dir );
unsigned char write_zero( struct inode *file, unsigned int count );
unsigned char is_file_opened( const struct dir_entry *dentry );

static void *alloc_inode( void );
static void free_inode( struct inode *file );
static unsigned char create_file( const char *fname, struct dir_entry *dentry,
		int *dentry_index );
static unsigned char free_cluster_until_end( unsigned int cluster_index );
static unsigned char update_dentry( struct file *f_handle);

#endif
