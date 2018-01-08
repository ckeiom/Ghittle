#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define BYTEOFSECTOR 512

int adjust_sector_size( int fd, int size );
void write_kern_info( int fd, int t_sec_count, int kern32_sec_count );
int copyfile( int s_fd, int d_fd );

int main(int argc, char** argv )
{
	int s_fd, d_fd;
	int bootloader_size;
	int kern32_sector_count;
	int kern64_sector_count;
	int filesys_sector_count;
	int source_size;

	if( argc < 5 )
		exit(-1);

	if( ( d_fd = open("disk.img",O_RDWR|O_CREAT|O_TRUNC,
					S_IREAD|S_IWRITE ) ) < 0 )
		exit(-1);

	printf("Copying bootloader to image file...\n");
	
	if( ( s_fd = open( argv[1],O_RDONLY) ) < 0 )
		exit(-1);

	source_size = copyfile( s_fd, d_fd );
	close(s_fd);

	bootloader_size = adjust_sector_size( d_fd, source_size );

	printf("   %s size: [%d], %d sectors\n",argv[1],source_size,bootloader_size);

	printf("Copying kern32 to image file...\n");

	if( ( s_fd = open( argv[2],O_RDONLY) ) < 0 )
		exit(-1);
	
	source_size = copyfile( s_fd, d_fd );
	close(s_fd);

	kern32_sector_count = adjust_sector_size( d_fd, source_size );

	printf("   %s size: [%d], %d sectors\n",argv[2],source_size,kern32_sector_count);

	printf("Copying kern64 to image file...\n");

	if( ( s_fd = open( argv[3],O_RDONLY) ) < 0 )
		exit(-1);
	
	source_size = copyfile( s_fd, d_fd );
	close(s_fd);

	kern64_sector_count = adjust_sector_size( d_fd, source_size );

	printf("   %s size: [%d], %d sectors\n",argv[3],source_size,kern64_sector_count);

	printf("Copying filesys image to image file...\n");

	if((s_fd = open(argv[4], O_RDONLY) ) < 0)
		exit(-1);

	source_size = copyfile( s_fd, d_fd );
	close(s_fd);

	filesys_sector_count = adjust_sector_size(d_fd, source_size);
	
	printf("   %s size: [%d], %d sectors\n",argv[4],source_size,filesys_sector_count);
	
	printf("Updating image file...\n");

	write_kern_info( d_fd, kern32_sector_count + kern64_sector_count, kern32_sector_count );

	printf("   complete!\n");

	close( d_fd );

	return 0;
}


int adjust_sector_size( int fd, int size )
{
	int i;
	int to_fill;
	char c;
	int sec_cnt;

	to_fill = size % BYTEOFSECTOR;
	c = 0x00;

	if ( to_fill != 0 )
	{
		to_fill = BYTEOFSECTOR - to_fill;
		printf("   file size: %d, to fill %d bytes\n",size,to_fill );
		for(i=0;i<to_fill;i++)
			write(fd, &c, 1 );
	}
	else
		printf("   already aligned 512 bytes\n");

	/* temp */
	/*for(i=0;i<511*BYTEOFSECTOR;i++)
		write(fd, &c, 1);
	sec_cnt = ( size + to_fill + (511*BYTEOFSECTOR) ) / BYTEOFSECTOR;
*/
	sec_cnt = ( size + to_fill ) / BYTEOFSECTOR ;
	return sec_cnt;
}
void write_kern_info( int fd, int t_sec_count, int kern32_sec_count )
{
	unsigned short data;
	long pos;

	pos = lseek( fd, 5, SEEK_SET );
	if ( pos < 0 )
		exit(-1);

	read( fd, &data, 2 );
	printf("   default set sectors: %d\n",data);
	read( fd, &data, 2 );
	printf("   default set kern32 sectors: %d\n",data );

	pos = lseek( fd, 5, SEEK_SET );

	data = t_sec_count;
	write( fd, &data, 2 );

	data = kern32_sec_count;
	write( fd, &data, 2 );
}

int copyfile( int s_fd, int d_fd )
{
	int s_file_size;
	int r,w;
	char buf[BYTEOFSECTOR];

	s_file_size = 0;
	while(1)
	{
		r = read(s_fd, buf, sizeof(buf) );
		w = write(d_fd, buf, r );

		if ( r!=w )
			exit(-1);
		s_file_size += r;

		if( r != sizeof(buf) )
			break;
	}
	return s_file_size;
}
