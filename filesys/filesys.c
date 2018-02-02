/* This file is initally written by Hyunsub Song on 2017.12.26 */
#include <filesys.h>

/* Temp global variables */
struct inode t1[FS_HANDLE_MAXCOUNT * sizeof( struct inode )];
unsigned char t2[FS_CLUSTERSIZE];
struct dir_entry t3[FS_CLUSTERSIZE];

static struct filesys_mgr fs_mgr;
static unsigned char temp_buf[FS_SECTORSPERCLUSTER * 512];

f_read_hdd_info p_read_hdd_info = 0;
f_read_hdd_sector p_read_hdd_sector = 0;
f_write_hdd_sector p_write_hdd_sector = 0;

unsigned char init_fs( void )
{
	memset( &fs_mgr, 0, sizeof( fs_mgr ) );
	init_mutex( &( fs_mgr.mut ) );

	if( init_hdd() == 1 )
	{
		p_read_hdd_info = read_hdd_info;
		p_read_hdd_sector = read_hdd_sector;
		p_write_hdd_sector = write_hdd_sector;
	}
	else
	{
		return 0;
	}

	if( mount() == 0 )
		return 0;

	/* Need to modify to no alloc version */ 
	/* fs_mgr.inode_pool = ( struct inode * ) KMALLOC(
			FS_HANDLE_MAXCOUNT * sizeof( struct inode ) ); */

	/* struct inode t1[FS_HANDLE_MAXCOUNT * sizeof( struct inode )]; */
	fs_mgr.inode_pool = ( struct inode * ) t1;

	if( fs_mgr.inode_pool == 0 )
	{
		fs_mgr.mounted = 0;
		return 0;
	}

	memset( fs_mgr.inode_pool, 0, FS_HANDLE_MAXCOUNT * sizeof( struct inode ) );
	/***************************************/

	return 1;
}

/* Low level function */
unsigned char mount( void )
{
	struct mbr *pmbr;

	lock( &( fs_mgr.mut ) );

	if( p_read_hdd_sector( 1, 1, 0, 1, temp_buf ) == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	pmbr = ( struct mbr * ) temp_buf;
	if( pmbr->sign != FS_SIGN )
	{
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	fs_mgr.mounted = 1;

	fs_mgr.reserved_sector_count = pmbr->reserved_sector_count;
	fs_mgr.cla_start_addr = pmbr->reserved_sector_count + 1;
	fs_mgr.cla_size = pmbr->cl_sector_count;
	fs_mgr.da_start_addr = pmbr->reserved_sector_count +
		pmbr->cl_sector_count + 1;
	fs_mgr.total_cluster_count = pmbr->total_cluster_count;

	unlock( &( fs_mgr.mut ) );
	return 1;
}

unsigned char format( void )
{
	struct hdd_info *h_info;
	struct mbr *pmbr;
	unsigned int total_sector_count, remain_sector_count;
	unsigned int max_cluster_count, cluster_count;
	unsigned int cl_sector_count;
	unsigned int i;

	lock( &( fs_mgr.mut ) );

	h_info = ( struct hdd_info * ) temp_buf;
	if( p_read_hdd_info( 1, 1, h_info ) == 0 )
	{
		unlock( &( fs_mgr.mut) );
		return 0;
	}
	total_sector_count = h_info->total_sector;

	max_cluster_count = total_sector_count / FS_SECTORSPERCLUSTER;

	cl_sector_count = ( max_cluster_count + 127 ) / 128;

	remain_sector_count = total_sector_count - cl_sector_count - 1;
	cluster_count = remain_sector_count / FS_SECTORSPERCLUSTER;

	cl_sector_count = ( cluster_count + 127 ) / 128;

	if( p_read_hdd_sector( 1, 1, 0, 1, temp_buf ) == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	pmbr = ( struct mbr * ) temp_buf;
	memset( pmbr->part, 0, sizeof(pmbr->part ) );
	pmbr->sign = FS_SIGN;
	pmbr->reserved_sector_count = 0;
	pmbr->cl_sector_count = cl_sector_count;
	pmbr->total_cluster_count = cluster_count;

	if( p_write_hdd_sector( 1, 1, 0, 1, temp_buf ) == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	memset( temp_buf, 0, 512 );
	for( i = 0; i < ( cl_sector_count + FS_SECTORSPERCLUSTER );
			i++ )
	{
		if( i == 0 )
			( ( unsigned int * ) ( temp_buf ) )[0] = FS_LASTCLUSTER;
		else	
			( ( unsigned int * ) ( temp_buf ) )[0] = FS_FREECLUSTER;

		if (p_write_hdd_sector( 1, 1, i + 1, 1, temp_buf ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}
	}

	unlock( &( fs_mgr.mut ) );
	return 1;
}

unsigned char get_hdd_info( struct hdd_info *h_info )
{
	unsigned char result;
	
	lock( &( fs_mgr.mut ) );
	result = p_read_hdd_info( 1, 1, h_info );
	unlock( &( fs_mgr.mut ) );

	return result;
}

static unsigned char read_cl_table( unsigned int offset, unsigned char *buf )
{
	return p_read_hdd_sector( 1, 1, offset + fs_mgr.cla_start_addr, 1, buf );
}

static unsigned char write_cl_table( unsigned int offset, unsigned char *buf )
{
	return p_write_hdd_sector( 1, 1, offset + fs_mgr.cla_start_addr, 1, buf );
}

static unsigned char read_cluster( unsigned int offset, unsigned char *buf )
{
	return p_read_hdd_sector( 1, 1, ( offset * FS_SECTORSPERCLUSTER ) +
			fs_mgr.da_start_addr, FS_SECTORSPERCLUSTER, buf );
}

static unsigned char write_cluster( unsigned int offset, unsigned char *buf )
{
	return p_write_hdd_sector( 1, 1, ( offset * FS_SECTORSPERCLUSTER ) +
			fs_mgr.da_start_addr, FS_SECTORSPERCLUSTER, buf );
}

static unsigned int find_free_cluster( void )
{
	unsigned int link_count;
	unsigned int last_sector_offset, current_sector_offset;
	unsigned int i, j;

	if( fs_mgr.mounted == 0 )
		return FS_LASTCLUSTER;

	last_sector_offset = fs_mgr.cl_sector_offset;

	for( i = 0; i < fs_mgr.cla_size; i++ )
	{
		if( ( last_sector_offset + i ) == ( fs_mgr.cla_size - 1 ) )
			link_count = fs_mgr.total_cluster_count % 128;
		else
			link_count = 128;

		current_sector_offset = ( last_sector_offset + i ) % fs_mgr.cla_size;

		if( read_cl_table( current_sector_offset, temp_buf ) == 0 )
			return FS_LASTCLUSTER;

		for( j = 0; j < link_count; j++ )
			if( ( ( unsigned int * ) temp_buf )[j] == FS_FREECLUSTER )
				break;
		
		if( j != link_count )
		{
			fs_mgr.cl_sector_offset = current_sector_offset;

			return ( current_sector_offset * 128 ) + j;
		}
	}

	return FS_LASTCLUSTER;
}

static unsigned char set_cl_data( unsigned int cluster_index, unsigned int data )
{
	unsigned int sector_offset;

	if( fs_mgr.mounted == 0 )
		return 0;

	sector_offset = cluster_index / 128;

	if( read_cl_table( sector_offset, temp_buf ) == 0 )
		return 0;

	( ( unsigned int * ) temp_buf ) [cluster_index % 128] = data;
	
	if( write_cl_table( sector_offset, temp_buf ) == 0 )
		return 0;

	return 1;
}

static unsigned char get_cl_data( unsigned int cluster_index, unsigned int *data )
{
	unsigned int sector_offset;

	if( fs_mgr.mounted == 0 )
		return 0;

	sector_offset = cluster_index / 128;

	if( sector_offset > fs_mgr.cla_size )
		return 0;

	if( read_cl_table( sector_offset, temp_buf ) == 0 )
		return 0;

	*data = ( ( unsigned int * ) temp_buf )[cluster_index % 128];

	return 1;
}

static int find_free_dentry( void )
{
	struct dir_entry *dentry;
	int i;

	if( fs_mgr.mounted == 0 )
		return -1;

	if( read_cluster( 0, temp_buf ) == 0 )
		return -1;

	dentry = ( struct dir_entry * ) temp_buf;
	
	for( i = 0; i < FS_MAXDIRENTRYCOUNT; i++ )
		if( dentry[i].start_cluster_index == 0 )
			return i;

	return -1;
}

static unsigned char set_dentry_data( int index, struct dir_entry *dentry )
{
	struct dir_entry *root_entry;

	if( ( fs_mgr.mounted == 0 ) || ( index < 0 ) || ( index >= FS_MAXDIRENTRYCOUNT ) )
		return 0;

	if( read_cluster( 0, temp_buf ) == 0 )
		return 0;

	root_entry = ( struct dir_entry * ) temp_buf;
	memcpy( root_entry + index, dentry, sizeof( struct dir_entry ) );

	if( write_cluster( 0, temp_buf ) == 0 )
		return 0;

	return 1;
}

static unsigned char get_dentry_data( int index, struct dir_entry *dentry )
{
	struct dir_entry *root_entry;

	if( ( fs_mgr.mounted == 0 ) || ( index < 0 ) || ( index >= FS_MAXDIRENTRYCOUNT ) )
		return 0;

	if( read_cluster( 0, temp_buf ) == 0 )
		return 0;

	root_entry = ( struct dir_entry * ) temp_buf;
	memcpy( dentry, root_entry + index, sizeof( struct dir_entry ) );

	return 1;
}

static int find_dentry( const char *fname, struct dir_entry *dentry )
{
	struct dir_entry *root_entry;
	int i;
	int len;

	if( fs_mgr.mounted == 0 )
		return -1;

	if( read_cluster( 0, temp_buf ) == 0 )
		return -1;

	len = strlen( fname );
	root_entry = ( struct dir_entry * ) temp_buf;
	for( i = 0; i < FS_MAXDIRENTRYCOUNT; i++ )
	{
		if( memcmp( root_entry[i].fname, fname, len ) == 0 )
		{
			memcpy( dentry, root_entry + i, sizeof( struct dir_entry ) );
			return i;
		}
	}

	return -1;
}

void get_fs_info( struct filesys_mgr *mgr )
{
	memcpy( mgr, &fs_mgr, sizeof( fs_mgr ) );
}


static void *alloc_inode( void )
{
	int i;
	struct inode *file;
	
	file = fs_mgr.inode_pool;
	for( i = 0; i < FS_HANDLE_MAXCOUNT; i++ )
	{
		if( file->type == FS_TYPE_FREE )
		{
			file->type = FS_TYPE_FILE;
			return file;
		}
		file++;
	}

	return 0;
}

static void free_inode( struct inode *file )
{
	memset( file, 0, sizeof( struct inode ) );
	file->type = FS_TYPE_FREE;
}

static unsigned char create_file( const char *fname, struct dir_entry *dentry,
		int *dentry_index )
{
	unsigned int cluster;
	cluster = find_free_cluster();
	if( ( cluster == FS_LASTCLUSTER ) ||
			( set_cl_data( cluster, FS_LASTCLUSTER ) == 0 ) )
		return 0;

	*dentry_index = find_free_dentry();
	if( *dentry_index == -1 )
	{
		set_cl_data( cluster, FS_FREECLUSTER );
		return 0;
	}

	memcpy( dentry->fname, fname, strlen( fname ) + 1 );
	dentry->start_cluster_index = cluster;
	dentry->fsize = 0;

	if( set_dentry_data( *dentry_index, dentry ) == 0 )
	{
		set_cl_data( cluster, FS_FREECLUSTER );
		return 0;
	}

	return 1;
}

static unsigned char free_cluster_until_end( unsigned int cluster_index )
{
	unsigned int current_cluster_index;
	unsigned int next_cluster_index;

	current_cluster_index = cluster_index;

	while( current_cluster_index != FS_LASTCLUSTER )
	{
		if( get_cl_data( current_cluster_index, &next_cluster_index ) == 0 )
			return 0;

		if( set_cl_data( current_cluster_index, FS_FREECLUSTER ) == 0 )
			return 0;

		current_cluster_index = next_cluster_index;
	}

	return 1;
}

struct inode *open_file( const char *fname, const char *mode )
{
	struct dir_entry dentry;
	int dentry_offset;
	int fname_len;
	unsigned int second_cluster;
	struct inode *file;

	fname_len = strlen( fname );
	if( ( fname_len > ( sizeof( dentry.fname ) - 1 ) ) ||
		( fname_len == 0 ) )
		return 0;

	lock( &( fs_mgr.mut ) );

	dentry_offset = find_dentry( fname, &dentry );
	if( dentry_offset == -1 )
	{
		if( mode[0] == 'r' )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}

		if( create_file( fname, &dentry, &dentry_offset ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}
	}
	else if( mode[0] == 'w' )
	{
		if( get_cl_data( dentry.start_cluster_index, &second_cluster ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}

		if( set_cl_data( dentry.start_cluster_index, FS_LASTCLUSTER ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}

		if( free_cluster_until_end( second_cluster ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}

		dentry.fsize = 0;
		if( set_dentry_data( dentry_offset, &dentry ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return 0;
		}
	}

	file = alloc_inode();
	if( file == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	file->type = FS_TYPE_FILE;
	file->f_handle.dentry_offset = dentry_offset;
	file->f_handle.fsize = dentry.fsize;
	file->f_handle.start_cluster_index = dentry.start_cluster_index;
	file->f_handle.current_cluster_index = dentry.start_cluster_index;
	file->f_handle.previous_cluster_index = dentry.start_cluster_index;
	file->f_handle.current_offset = 0;

	if( mode[0] == 'a' )
		seek_file( file, 0, FS_SEEK_END );

	unlock( &( fs_mgr.mut ) );

	return file;
}

unsigned int read_file( void *buf, unsigned int size, unsigned int count,
		struct inode *file )
{
	unsigned int total_count;
	unsigned int read_count;
	unsigned int offset_cluster;
	unsigned int copy_size;
	struct file *f_handle;
	unsigned int next_cluster_index;

	if( ( file == 0 ) || ( file->type != FS_TYPE_FILE ) )
		return 0;

	f_handle = &( file->f_handle );

	if( ( f_handle->current_offset == f_handle->fsize ) ||
			( f_handle->current_cluster_index == FS_LASTCLUSTER ) )
		return 0;

	total_count = MIN( size * count, f_handle->fsize - f_handle->current_offset );

	lock( &( fs_mgr.mut ) );

	read_count = 0;
	while( read_count != total_count )
	{
		if( read_cluster( f_handle->current_cluster_index, temp_buf ) == 0 ) 
			break;

		offset_cluster = f_handle->current_offset % FS_CLUSTERSIZE;

		copy_size = MIN( FS_CLUSTERSIZE - offset_cluster, total_count - read_count );
		memcpy( ( char * ) buf + read_count, temp_buf + offset_cluster, copy_size );

		read_count += copy_size;
		f_handle->current_offset += copy_size;

		if( ( f_handle->current_offset % FS_CLUSTERSIZE ) == 0 )
		{
			if( get_cl_data( f_handle->current_cluster_index,
						&next_cluster_index ) == 0 )
				break;
			
			f_handle->previous_cluster_index = f_handle->current_cluster_index;
			f_handle->current_cluster_index = next_cluster_index;
		}
	}

	unlock( &( fs_mgr.mut ) );

	return read_count;
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

unsigned int write_file( const void *buf, unsigned int size, unsigned int count,
		struct inode *file )
{
	unsigned int write_count;
	unsigned int total_count;
	unsigned int offset_cluster;
	unsigned int copy_size;
	unsigned int alloc_cluster_index;
	unsigned int next_cluster_index;
	struct file *f_handle;

	if( ( file == 0 ) || ( file->type != FS_TYPE_FILE ) )
		return 0;

	f_handle = &( file->f_handle );

	total_count = size * count;

	lock( &( fs_mgr.mut ) );

	write_count = 0;
	while( write_count != total_count )
	{
		if( f_handle->current_cluster_index == FS_LASTCLUSTER )
		{
			alloc_cluster_index = find_free_cluster();
			if( alloc_cluster_index == FS_LASTCLUSTER )
				break;

			if( set_cl_data( alloc_cluster_index, FS_LASTCLUSTER ) == 0 )
				break;

			if( set_cl_data( f_handle->previous_cluster_index,
						alloc_cluster_index ) == 0 )
			{
				set_cl_data( alloc_cluster_index, FS_FREECLUSTER );
				break;
			}

			f_handle->current_cluster_index = alloc_cluster_index;

			memset( temp_buf, 0, FS_LASTCLUSTER );
		}
		else if( ( ( f_handle->current_offset % FS_CLUSTERSIZE ) != 0 ) ||
				( ( total_count - write_count ) < FS_CLUSTERSIZE ) )
			if( read_cluster( f_handle->current_cluster_index, temp_buf ) == 0 )
				break;

		offset_cluster = f_handle->current_offset % FS_CLUSTERSIZE;
		copy_size = MIN( FS_CLUSTERSIZE - offset_cluster, total_count - write_count );
		memcpy( temp_buf + offset_cluster, ( char * ) buf + write_count, copy_size );

		if( write_cluster( f_handle->current_cluster_index, temp_buf ) == 0 )
			break;

		write_count += copy_size;
		f_handle->current_offset += copy_size;

		if( ( f_handle->current_offset % FS_CLUSTERSIZE ) == 0 )
		{
			if( get_cl_data( f_handle->current_cluster_index,
						&next_cluster_index ) == 0 )
				break;

			f_handle->previous_cluster_index = f_handle->current_cluster_index;
			f_handle->current_cluster_index = next_cluster_index;
		}
	}

	if( f_handle->fsize < f_handle->current_offset )
	{
		f_handle->fsize = f_handle->current_offset;
		update_dentry( f_handle );
	}

	unlock( &( fs_mgr.mut ) );

	return write_count;
}

unsigned char write_zero( struct inode *file, unsigned int count )
{
	unsigned char *buf;
	unsigned int remain_count;
	unsigned int write_count;

	if( file == 0 )
		return 0;

	/* buf = ( unsigned char * ) KMALLOC( FS_CLUSTERSIZE ); */
	/* unsigned char t2[FS_CLUSTERSIZE]; */
	buf = ( unsigned char * ) t2;


	if( buf == 0 )
		return 0;

	memset( buf, 0, FS_CLUSTERSIZE );
	remain_count = count;

	while( remain_count != 0 )
	{
		write_count = MIN( remain_count, FS_CLUSTERSIZE );
		if( write_file( buf, 1, write_count, file ) != write_count )
		{
			/* KFREE( buf ); */
			return 0;
		}
		remain_count -= write_count;
	}
	/* KFREE( buf ); */
	
	return 1;
}

int seek_file( struct inode *file, int offset, int origin )
{
	unsigned int real_offset;
	unsigned int cluster_offset_move;
	unsigned int current_cluster_offset;
	unsigned int last_cluster_offset;
	unsigned int move_count;
	unsigned int i;
	unsigned int start_cluster_index;
	unsigned int previous_cluster_index;
	unsigned int current_cluster_index;
	struct file *f_handle;

	if( ( file == 0 ) || ( file->type != FS_TYPE_FILE ) )
		return 0;

	f_handle = &( file->f_handle );

	switch( origin )
	{
		case FS_SEEK_SET:
			if( offset <= 0 )
				real_offset = 0;
			else
				real_offset = offset;
			break;
		case FS_SEEK_CUR:
			if( ( offset < 0 ) && ( f_handle->current_offset <= ( unsigned int ) -offset ) )
				real_offset = 0;
			else
				real_offset = f_handle->current_offset + offset;
			break;
		case FS_SEEK_END:
			if( ( offset < 0 ) && ( f_handle->fsize <= ( unsigned int ) -offset ) )
				real_offset = 0;
			else
				real_offset = f_handle->fsize + offset;
			break;
	}

	last_cluster_offset = f_handle->fsize / FS_CLUSTERSIZE;
	cluster_offset_move = real_offset / FS_CLUSTERSIZE;
	current_cluster_offset = f_handle->current_offset / FS_CLUSTERSIZE;

	if( last_cluster_offset < cluster_offset_move )
	{
		move_count = last_cluster_offset - current_cluster_offset;
		start_cluster_index = f_handle->current_cluster_index;
	}
	else if( current_cluster_offset <= cluster_offset_move )
	{
		move_count = cluster_offset_move - current_cluster_offset;
		start_cluster_index = f_handle->current_cluster_index;
	}
	else
	{
		move_count = cluster_offset_move;
		start_cluster_index = f_handle->start_cluster_index;
	}

	lock( &( fs_mgr.mut ) );

	current_cluster_index = start_cluster_index;

	for( i = 0; i < move_count; i++ )
	{
		previous_cluster_index = current_cluster_index;

		if( get_cl_data( previous_cluster_index, &current_cluster_index ) == 0 )
		{
			unlock( &( fs_mgr.mut ) );
			return -1;
		}
	}

	if( move_count > 0 )
	{
		f_handle->previous_cluster_index = previous_cluster_index;
		f_handle->current_cluster_index = current_cluster_index;
	}
	else if( start_cluster_index == f_handle->start_cluster_index )
	{
		f_handle->previous_cluster_index = f_handle->start_cluster_index;
		f_handle->current_cluster_index = f_handle->start_cluster_index;
	}

	if( last_cluster_offset < cluster_offset_move )
	{
		f_handle->current_offset = f_handle->fsize;
		unlock( &( fs_mgr.mut ) );

		if( write_zero( file, real_offset - f_handle->fsize ) == 0 )
			return 0;
	}

	f_handle->current_offset = real_offset;

	unlock( &( fs_mgr.mut ) );

	return 0;
}

int close_file( struct inode *file )
{
	if( ( file == 0 ) || ( file->type != FS_TYPE_FILE ) )
		return -1;

	free_inode( file );
	
	return 0;
}

unsigned char is_file_opened( const struct dir_entry *dentry )
{
	int i;
	struct inode *file;

	file = fs_mgr.inode_pool;
	for( i = 0; i < FS_HANDLE_MAXCOUNT; i++ )
		if( ( file[i].type == FS_TYPE_FILE ) &&
		    ( file[i].f_handle.start_cluster_index == dentry->start_cluster_index ) )
			return 1;

	return 0;
}

int remove_file( const char *fname )
{
	struct dir_entry dentry;
	int dentry_offset;
	int fname_len;

	fname_len = strlen( fname );
	if( ( fname_len > ( sizeof( dentry.fname ) - 1 ) ) ||
			(fname_len == 0 ) )
		return 0;

	lock( &( fs_mgr.mut ) );

	dentry_offset = find_dentry( fname, &dentry );
	if( dentry_offset == -1 )
	{
		unlock( &( fs_mgr.mut ) );
		return -1;
	}

	if( is_file_opened( &dentry ) == 1 )
	{
		unlock( &( fs_mgr.mut ) );
		return -1;
	}

	if( free_cluster_until_end( dentry.start_cluster_index ) == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return -1;
	}

	memset( &dentry, 0, sizeof( dentry) );
	if( set_dentry_data( dentry_offset, &dentry ) == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return -1;
	}

	unlock( &( fs_mgr.mut ) );
	
	return 0;
}

struct inode *open_dir( const char *dname)
{
	struct inode *dir;
	struct dir_entry *dir_buf;

	lock( &( fs_mgr.mut ) );
	dir = alloc_inode();
	if( dir == 0 )
	{
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	/* dir_buf = ( struct dir_entry * ) KMALLOC( FS_CLUSTERSIZE ); */
	/* struct dir_entry t3[FS_CLUSTERSIZE]; */
	dir_buf = ( struct dir_entry * ) t3;

	if( dir == 0 )
	{
		free_inode( dir );
		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	if( read_cluster( 0, ( unsigned char * ) dir_buf ) == 0 )
	{
		free_inode( dir );
		/* KFREE( dir_buf ); */

		unlock( &( fs_mgr.mut ) );
		return 0;
	}

	dir->type = FS_TYPE_DIR;
	dir->d_handle.current_offset = 0;
	dir->d_handle.dir_buf = dir_buf;

	unlock( &( fs_mgr.mut ) );
	return dir;
}

struct dir_entry *read_dir( struct inode *dir )
{
	struct dir *d_handle;
	struct dir_entry *dentry;

	if( ( dir == 0 ) || ( dir->type != FS_TYPE_DIR ) )
		return 0;

	d_handle = &( dir->d_handle );

	if( ( d_handle->current_offset < 0 ) ||
			( d_handle->current_offset >= FS_MAXDIRENTRYCOUNT ) )
		return 0;

	lock( &( fs_mgr.mut ) );

	dentry = d_handle->dir_buf;
	while( d_handle->current_offset < FS_MAXDIRENTRYCOUNT )
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




