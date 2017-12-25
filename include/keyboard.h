#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <dio.h>

/* How many scan codes to be ignored to process PAUSE */
#define KEY_SKIPCOUNT_FOR_PAUSE       2

#define KEY_FLAGS_UP             0x00
#define KEY_FLAGS_DOWN           0x01
#define KEY_FLAGS_EXTENDEDKEY    0x02

#define KEY_MAPPING_TABLE_COUNT    89

#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08

#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8A
#define KEY_F5          0x8B
#define KEY_F6          0x8C
#define KEY_F7          0x8D
#define KEY_F8          0x8E
#define KEY_F9          0x8F
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9A
#define KEY_PAGEDOWN    0x9B
#define KEY_INS         0x9C
#define KEY_DEL         0x9D
#define KEY_F11         0x9E
#define KEY_F12         0x9F
#define KEY_PAUSE       0xA0

#define KEY_MAX_QUEUE_CNT	100

#pragma pack( push, 1 )
struct key_map_entry
{
	unsigned char normal;	
	unsigned char combined;	/* can be combined with Shift, Caps lock */
};

struct key_manager
{
	unsigned char shift_down;
	unsigned char caps_on;
	unsigned char num_on;
	unsigned char scroll_on;

	unsigned char ext_code;
	int skip_count;	/* for puase */
};

struct key_data
{
	unsigned char scan_code;
	unsigned char ascii_code;
	unsigned char flags;
};

#pragma pack( pop )

static inline unsigned char is_obuffer_full(void)
{
	return (in_b(0x64) & 0x01)? 1:0;
}

static inline unsigned char is_ibuffer_full(void)
{
	return (in_b(0x64) & 0x02)? 1:0;
}

unsigned char wait_for_ack(void);
unsigned char put_scan_code(unsigned char scan_code);
unsigned char get_key_data( struct key_data* data );

void update_keystate( unsigned char code );
unsigned char code_to_ascii( unsigned char code, unsigned char* ascii, unsigned char* flags );
unsigned char init_keyboard(void);
unsigned char get_keycode(void);
unsigned char change_led(unsigned char caps, unsigned char num, unsigned char scroll);
void enable_a20gate(void);
void reboot(void);
#endif
