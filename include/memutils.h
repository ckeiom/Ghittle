#ifndef __MEMUTILS_H__
#define __MEMUTILS_H__

#define MIN( x, y )			( ( ( x ) < ( y ) ? ( x ) : ( y ) ) )
#define MAX( x, y )			( ( ( x ) > ( y ) ? ( x ) : ( y ) ) )

void memset( void* dst, unsigned char data, int size );
int memcpy( void* dst, const void* src, int size);
int memcmp( const void* dst, const void* src, int size );

#endif
