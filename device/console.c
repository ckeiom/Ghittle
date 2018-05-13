#include <console.h>
#include <memutils.h>
#include <string.h>
#include <dio.h>
#include <stdarg.h>
#include <keyboard.h>
#include <sched.h>

struct console console = {0, };

void init_console( int x, int y )
{
	memset(&console, 0, sizeof(struct console));
	set_cursor(x, y);
}

void set_cursor( int x, int y )
{
	int offset;
	
	offset = y * CONSOLE_WIDTH + x;

	out_b(VGA_PORT_INDEX, VGA_INDEX_UPPER_CURSOR);
	out_b(VGA_PORT_DATA, offset >> 8);

	out_b(VGA_PORT_INDEX, VGA_INDEX_LOWER_CURSOR);
	out_b(VGA_PORT_DATA, offset & 0xFF);

	console.current_offset = offset;
}


void get_cursor(int* x, int* y)
{
	*x = console.current_offset % CONSOLE_WIDTH;
	*y = console.current_offset / CONSOLE_WIDTH;
}

void printk(const char* fstring, ...)
{
	va_list ap;
	char vc_buf[1024];
	int next_offset;

	va_start(ap, fstring);
	vsprintf(vc_buf, fstring, ap);
	va_end(ap);

	next_offset = console_print_string(vc_buf);
	set_cursor(next_offset % CONSOLE_WIDTH, next_offset / CONSOLE_WIDTH);
}


int console_print_string(const char* buf)
{
	struct console_buffer* cb = (struct console_buffer*)VIDEO_MEM_START;
	int i,j;
	int length;
	int offset;

	offset = console.current_offset;
	length = strlen(buf);

	for(i = 0; i < length; i++)
	{
		switch(buf[i])
		{
			case '\n':
				offset += (CONSOLE_WIDTH - (offset % CONSOLE_WIDTH));
				break;
			case '\t':
				offset += (CONSOLE_TAB_SIZE - (offset % CONSOLE_TAB_SIZE));
				break;
			default:
				cb[offset].ch = buf[i];
				offset++;
		}
		if(offset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH))
		{
			memcpy( (void *)VIDEO_MEM_START, 
					(void *)(VIDEO_MEM_START + CONSOLE_WIDTH * sizeof(struct console_buffer)),
					( CONSOLE_HEIGHT - 1 )*CONSOLE_WIDTH*sizeof(struct console_buffer));
			for(j = (CONSOLE_HEIGHT - 1)*(CONSOLE_WIDTH); j < (CONSOLE_HEIGHT * CONSOLE_WIDTH); j++)
			{
				cb[j].ch = ' ';
			}
			offset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		}
	}
	return offset;
}


void clear_screen(void)
{
	struct console_buffer *cb = (struct console_buffer*)VIDEO_MEM_START;
	int i;

	for( i=0; i<CONSOLE_WIDTH*CONSOLE_HEIGHT; i++ )
		cb[i].ch = ' ';
	set_cursor(0, 0);
}

unsigned char getch(void)
{
	struct key_data data;

	while(1)
	{
		while(get_key_data(&data) == 0)
			schedule();
		if(data.flags & KEY_FLAGS_DOWN)
			return data.ascii_code;
	}
}

void print_string_xy(int x, int y, const char* str)
{
	struct console_buffer *cb = (struct console_buffer* )VIDEO_MEM_START;
	int i;

	cb += (y * CONSOLE_WIDTH) + x;

	for(i = 0; str[i] != 0 ;i++)
		cb[i].ch = str[i];
}


