#include <string.h>
#include <memutils.h>

int strlen( const char* str )
{
	int i,j;
	for(i=0; ; i++ )
		if( str[i] == '\0' )
			return i;
}

void reverse_string( char *str )
{
	int length;
	int i;
	char tmp;

	length = strlen(str);
	for( i=0 ; i<length/2; i++ )
	{
		tmp = str[i];
		str[i] = str[ length -1 -i ];
		str[ length -1 -i ] = tmp;
	}
}

int stoi( const char* str )
{
	int ret = 0;
	int tmp;
	unsigned char negative = 0;
	unsigned char hex = 0;

	if(*str == '-' )
	{
		str++;
		negative = 1;
	}

	if( memcmp( str, "0x", 2 ) == 0 )
	{
		hex = 1;
		str+=2;
	}
	
	if( hex )
	{
		while( *str!='\0' )
		{
			tmp = ( *str>='A' && *str<='F')? *str-'A'+10:
				  ( *str>='a' && *str<='f')? *str-'a'+10:
				  ( *str-'0' );
			if( tmp<0 || tmp >15)
				return ret;
			ret = ret*16 + tmp;
			str++;
		}
	}
	else
	{
		while( *str!='\0' )
		{
			if( *str < '0' || *str >'9' )
				return ret;
			ret = ret*10 + *str - '0';
			str++;
		}
	}
	return negative? -ret:ret;
}


 /* returns the # of digits */
int itos( int val, char* str, unsigned char hex )
{
	int tmp;
	char* pos;
	int i,j;

	i=1;
	pos = str;
	if( val < 0 )
	{
		tmp = -val;
		val = -val;
		*pos++ = '-';
	}
	else
		tmp = val;
	if( hex )
	{
		*pos++ = '0';
		*pos++ = 'x';
		while( tmp >= 16 )
		{
			tmp /= 16;
			i *= 16;
		}
		while(i)
		{
			tmp = val/i;
			tmp = (tmp>9)? tmp-10+'A':tmp+'0';
			*pos++ = tmp;
			val = val%i; i/=16;
		}
	}
	else
	{
		while(tmp >= 10)
		{
			tmp /= 10;
			i*=10;
		}

		while(i)
		{
			*pos++ = val/i+'0';
			val = val%i; i/=10;
		}
	}
	return (int)(pos-str);
}

int dqtos( unsigned long val, char* str )
{
	unsigned long tmp;
	int i,j;

	i=1;
	if( val < 0 )
	{
		tmp = -val;
		*str++ = '-';
	}
	else
		tmp = val;
	
	while(tmp >= 10)
	{
		tmp /= 10;
		i*=10;
	}

	while(val)
	{
		*str++ = val/i+'0';
		val = val%i; i/=10;
	}
	return j;
}


int hltos( long val, char* str )
{
	long tmp;
	int i,j;

	i = 1;
	if( val < 0 )
	{
		tmp = -val;
		*str++ = '-';
	}
	else
		tmp = val;
	
	while(tmp)
	{
		tmp /= 16;
		i *= 16;
	}

	while(val)
	{
		tmp = (val/i);
		tmp = (tmp>9)? tmp-10+'A':tmp;
		
		*str++ = tmp;
		val = val%i; i/=10;
	}
	return j;
}
	
int hqtos( unsigned long val, char* str )
{
	unsigned long tmp;
	int i,j;

	i = 1;
	if( val < 0 )
	{
		tmp = -val;
		*str++ = '-';
	}
	else
		tmp = val;
	
	while(tmp)
	{
		tmp /= 16;
		i *= 16;
	}

	while(val)
	{
		tmp = (val/i);
		tmp = (tmp>9)? tmp-10+'A':tmp;
		
		*str++ = tmp;
		val = val%i; i/=10;
	}
	return j;
}	
		
int sprintf( char* buf, const char* fstring, ... )
{
	va_list ap;
	int ret;

	va_start(ap, fstring);
	ret = vsprintf( buf, fstring, ap );
	va_end( ap );

	return ret;
}

int vsprintf( char* buf, const char* fstring, va_list ap )
{
	unsigned long i,j;
	int buf_index = 0;
	int format_length, copy_length;
	char* copy_string;
	unsigned long value;
	
	format_length = strlen(fstring);

	for(i=0;i<format_length; i++ )
	{
		if( fstring[i] == '%' )
		{
			switch( fstring[++i] )
			{
				case 's':
					copy_string = (char* )( va_arg(ap, char*) );
					copy_length = strlen(copy_string);
					memcpy( buf+buf_index, copy_string, copy_length );
					buf_index += copy_length;
					break;
				case 'c':
					buf[ buf_index ] = (char) ( va_arg(ap, int) ); //strange here why int?
					buf_index++;
					break;
				case 'd':
				case 'i':
					buf_index += itos( (int)(va_arg(ap,int) ), buf+buf_index, 0 );
					break;
				case 'x':
				case 'X':
					buf_index += itos( (int)(va_arg(ap,int) ), buf+buf_index, 1 );
					break;
				case 'q':
					buf_index += itos( (long)(va_arg(ap,long) ), buf+buf_index, 0 );
					break;
				default:
					buf[buf_index++] = fstring[i];
					break;
			}
		}
		else
			buf[ buf_index++ ] = fstring[i];
	}
	buf[buf_index] = '\0';
	return buf_index;
}




