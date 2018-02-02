#include <hdd.h>

static struct hdd_mgr h_mgr;

unsigned char init_hdd( void )
{
	init_mutex( &( h_mgr.mut ) );

	h_mgr.p_intr = 0;
	h_mgr.s_intr = 0;
	
	out_b( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
	out_b( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
	
	if( read_hdd_info( 1, 1, &( h_mgr.h_info ) ) == 0 )
	{
		h_mgr.detected = 0;
		h_mgr.write = 0;
		
		return 0;
	}

	h_mgr.detected = 1;
	if ( memcmp( h_mgr.h_info.model, "QEMU", 4 ) == 0 )
		h_mgr.write = 1;
	else
		h_mgr.write = 0;

	return 1;
}

static unsigned char read_hdd_stat( unsigned char p )
{
	if( p == 1 )
		return in_b( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS );

	return in_b( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS );
}

static unsigned char wait_hdd_nobusy( unsigned char p )
{
	unsigned long start_tcount;
	unsigned char stat;

	start_tcount = tick;

	while( ( tick - start_tcount ) <= HDD_WAITTIME )
	{
		stat = read_hdd_stat( p );

		if( ( stat & HDD_STATUS_BUSY ) != HDD_STATUS_BUSY )
			return 1;
		
		/* On implementing multi-tasking version, 
		   mdelay() changes to schedule() */
		mdelay( 1 ); 
	}

	return 0;
}

unsigned char read_hdd_info( unsigned char p,
							 unsigned char m,
							 struct hdd_info *h_info)
{
	unsigned short port_base;
	unsigned long last_tcount;
	unsigned char stat;
	unsigned char dflag;
	int i;
	unsigned short temp;
	unsigned char wait_result;

	if( p == 1 )
		port_base = HDD_PORT_PRIMARYBASE;
	else
		port_base = HDD_PORT_SECONDARYBASE;

	lock( &( h_mgr.mut ) );
	
	if( wait_hdd_nobusy( p ) == 0 )
	{
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	if( m == 1 )
		dflag = HDD_DRIVEANDHEAD_LBA;
	else
		dflag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

	out_b( port_base + HDD_PORT_INDEX_DRIVEANDHEAD, dflag );

	if( wait_hdd_ready( p ) == 0 )
	{
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	set_hdd_intr_flag( p, 0 );

	out_b( port_base + HDD_PORT_INDEX_COMMAND,
			HDD_COMMAND_IDENTIFY );
	wait_result = wait_hdd_intr( p );
	stat = read_hdd_stat( p );
	if( ( wait_result == 0 ) ||
			( ( stat & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR ) )
	{
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	/* Read one sector */
	for( i = 0; i < 512 / 2; i++ )
		( ( unsigned short * ) h_info )[i] =
			in_w( port_base + HDD_PORT_INDEX_DATA );

	swap_byte_in_word( h_info->model, sizeof( h_info->model ) / 2 );
	swap_byte_in_word( h_info->serial, sizeof( h_info->serial ) / 2 );

	unlock( &( h_mgr.mut ) );

	return 1;
}

static void swap_byte_in_word( unsigned short *data, int wcount )
{
	int i;
	unsigned short temp;

	for( i = 0; i < wcount; i++ )
	{
		temp = data[i];
		data[i] = ( temp >> 8 ) | ( temp << 8 );
	}
}

static unsigned char wait_hdd_ready( unsigned char p )
{
	unsigned long start_tcount;
	unsigned char stat;

	start_tcount = tick;

	while( ( tick - start_tcount ) <= HDD_WAITTIME)
	{

		stat = read_hdd_stat( p );
		if( ( stat & HDD_STATUS_READY ) == HDD_STATUS_READY )
			return 1;
		
		/* On implementing multi-tasking version, 
		   mdelay() changes to schedule() */
		mdelay( 1 );
	}
	
	return 0;
}

static unsigned char wait_hdd_intr( unsigned char p )
{
	unsigned long tcount;

	tcount = tick;

	while( ( tick - tcount ) <= HDD_WAITTIME )
	{
		if( ( p == 1 ) && ( h_mgr.p_intr == 1 ) )
			return 1;
		else if( ( p == 0 ) && ( h_mgr.s_intr == 1 ) )
			return 1;
	}

	return 0;
}

void set_hdd_intr_flag( unsigned char p, unsigned char flag )
{
	if( p == 1 )
		h_mgr.p_intr = flag;
	else
		h_mgr.s_intr = flag;
}

int read_hdd_sector( unsigned char p, unsigned char m,
					 unsigned int lba, int sector_count,
					 char *buf)
{
	unsigned short port_base;
	int i, j;
	unsigned char dflag;
	unsigned char stat;
	long rcount = 0;
	unsigned char wait_result;
	
	if( ( h_mgr.detected == 0 ) ||
		( sector_count <= 0 ) || ( 256 < sector_count ) ||
		( ( lba + sector_count ) >= h_mgr.h_info.total_sector ) )
		return 0;
	
	if( p == 1 )
		port_base = HDD_PORT_PRIMARYBASE;
	else
		port_base = HDD_PORT_SECONDARYBASE;

	lock( &( h_mgr.mut ) );

	if( wait_hdd_nobusy( p ) == 0 )
	{
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	/* Set data register */
	out_b( port_base + HDD_PORT_INDEX_SECTORCOUNT, sector_count );
	out_b( port_base + HDD_PORT_INDEX_SECTORNUMBER, lba );
	out_b( port_base + HDD_PORT_INDEX_CYLINDERLSB, lba >> 8 );
	out_b( port_base + HDD_PORT_INDEX_CYLINDERMSB, lba >> 16 );
				
	if( m == 1 )
		dflag = HDD_DRIVEANDHEAD_LBA;
	else
		dflag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

	out_b( port_base + HDD_PORT_INDEX_DRIVEANDHEAD,
			dflag | ( ( lba >> 24 ) & 0x0F ) );

	if( wait_hdd_ready( p ) == 0 )
	{
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	set_hdd_intr_flag( p, 0 );
	out_b( port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ );

	/* Receive data after waiting interrupt */
	for( i = 0; i < sector_count; i++ )
	{
		stat = read_hdd_stat( p );
		if( ( stat & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
		{
			printk( "Error Occur\n" );
			unlock( &( h_mgr.mut ) );
			return i;
		}

		if( ( stat & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
		{
			wait_result = wait_hdd_intr( p );
			set_hdd_intr_flag( p, 0 );
			if( wait_result == 0 )
			{
				printk( "Interrult Not Occur\n" );
				unlock( &( h_mgr.mut ) );
				return 0;
			}
		}
		
		for( j = 0; j < 512 / 2; j++ )
			( ( unsigned short * ) buf )[rcount++]
				= in_w( port_base + HDD_PORT_INDEX_DATA );

	}

	unlock( &( h_mgr.mut ) );

	return i;
}

int write_hdd_sector( unsigned char p, unsigned char m,
		              unsigned int lba, int sector_count,
					  char *buf)
{
	unsigned short port_base;
	unsigned short temp;
	int i, j;
	unsigned char dflag;
	unsigned char stat;
	long rcount = 0;
	unsigned char wait_result;

	if( ( h_mgr.write == 0 ) ||
		( sector_count <= 0 ) || ( 256 < sector_count ) ||
		( ( lba + sector_count ) >= h_mgr.h_info.total_sector ) )
		return 0;

	if( p == 1 )
		port_base = HDD_PORT_PRIMARYBASE;
	else
		port_base = HDD_PORT_SECONDARYBASE;

	lock( &( h_mgr.mut ) );
	
	if( wait_hdd_nobusy( p ) == 0 ) {
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	out_b( port_base + HDD_PORT_INDEX_SECTORCOUNT, sector_count );
	out_b( port_base + HDD_PORT_INDEX_SECTORNUMBER, lba );
	out_b( port_base + HDD_PORT_INDEX_CYLINDERLSB, lba >> 8 );
	out_b( port_base + HDD_PORT_INDEX_CYLINDERMSB, lba >> 16 );

	if( m == 1 )
		dflag = HDD_DRIVEANDHEAD_LBA;
	else
		dflag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

	out_b( port_base + HDD_PORT_INDEX_DRIVEANDHEAD,
			dflag | ( ( lba >> 24 ) & 0x0F ) );

	if ( wait_hdd_ready ( p ) == 0 )
	{
		unlock( &( h_mgr.mut ) );
		return 0;
	}

	set_hdd_intr_flag( p, 0 );
	out_b( port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE );

	while( 1 )
	{
		stat = read_hdd_stat( p );
		if( ( stat & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
		{
			unlock( &( h_mgr.mut ) );
			return 0;
		}

		if( ( stat & HDD_STATUS_DATAREQUEST ) == HDD_STATUS_DATAREQUEST )
			break;

		mdelay( 1 );
	}

	for( i = 0; i < sector_count; i++ )
	{
		set_hdd_intr_flag( p, 0 );
		for( j = 0; j < 512 / 2; j++ )
			out_w( port_base + HDD_PORT_INDEX_DATA,
					( ( unsigned short * ) buf )[rcount++] );

		stat = read_hdd_stat( p );
		if( ( stat & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
		{
			unlock( &( h_mgr.mut ) );
			return i;
		}

		if( ( stat & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
		{
			wait_result = wait_hdd_intr( p );
			set_hdd_intr_flag( p, 0 );
			if( wait_result == 0 )
			{
				unlock( &( h_mgr.mut ) );
				return 0;
			}
		}
	}

	unlock( &( h_mgr.mut ) );
	
	return i;
}


