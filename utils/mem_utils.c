#include <memutils.h>

void memset( void* dst, unsigned char data, int size)
{
	int i;
	for(i=0;i<size;i++)
		 ((char* )dst)[i] = data;
}

int memcpy( void *dst, const void* src, int size )
{
	int i;
	for(i=0;i<size;i++)
		( (char* )dst)[i] = ( (char* )src )[i];
	return size;
}

int memcmp( const void* dst, const void* src, int size )
{
	int i;
	char tmp;

	for(i=0;i<size;i++)
	{
		tmp = ( (char* )dst )[i] - ( (char*) src )[i];
		if( tmp!=0 )
			return (int)tmp;
	}
	return 0;
}
