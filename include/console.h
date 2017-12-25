#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define CONSOLE_BG_BLACK	0x00
#define CONSOLE_BG_BLUE		0x10
#define CONSOLE_BG_GREEN	0x20
#define CONSOLE_BG_CYAN		0x30
#define CONSOLE_BG_RED		0x40
#define CONSOLE_BG_MAGENTA	0x50
#define CONSOLE_BG_BROWN	0x60
#define CONSOLE_BG_WHITE	0x70
#define CONSOLE_BG_BLINK	0x80

#define CONSOLE_FG_DBLACK	0x00
#define CONSOLE_FG_DBLUE	0x01
#define CONSOLE_FG_DGREEN	0x02
#define CONSOLE_FG_DCYAN	0x03
#define CONSOLE_FG_DRED		0x04
#define CONSOLE_FG_DMAGENTA	0x05
#define CONSOLE_FG_DBROWN	0x06
#define CONSOLE_FG_DWHITE	0x07
#define CONSOLE_FG_BBLACK	0x08
#define CONSOLE_FG_BBLUE	0x09
#define CONSOLE_FG_BGREEN	0x0A
#define CONSOLE_FG_BCYAN	0x0B
#define CONSOLE_FG_BRED		0x0C
#define CONSOLE_FG_BMAGENTA	0x0D
#define CONSOLE_FG_BBROWN	0x0E
#define CONSOLE_FG_BWHITE	0x0F

#define CONSOLE_DEFAULT_COLOR	( CONSOLE_BG_BLACK | CONSOLE_FG_BWHITE )
#define CONSOLE_PBOOT_COLOR		( CONSOLE_BG_CYAN | CONSOLE_FG_DBLACK )

#define CONSOLE_WIDTH	80
#define CONSOLE_HEIGHT	25

#define CONSOLE_TAB_SIZE	8

#define VGA_PORT_INDEX	0x3D4
#define VGA_PORT_DATA	0x3D5
#define VGA_INDEX_UPPER_CURSOR	0x0E
#define VGA_INDEX_LOWER_CURSOR	0x0F

#define VIDEO_MEM_START	0xB8000

#pragma pack(push, 1)

struct console_buffer
{
	unsigned char ch;
	unsigned char attr;
};

#pragma pack(pop)

#pragma pack( push, 1 )

struct console
{
	int current_offset;
};

#pragma pack( pop )

void init_console( int x, int y );
void set_cursor( int x, int y );
void get_cursor( int* x, int* y );
void printk( const char* fstring, ... );
int console_print_string( const char* buf );
void print_string_xy( int x, int y, const char* str );
void clear_screen( void );
unsigned char getch(void);

#endif
