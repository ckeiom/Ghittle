#include <types.h>
#include <asmutils.h>
#include <keyboard.h>
#include <queue.h>
#include <sync.h>

static struct queue key_queue;
static struct key_data key_buffer[KEY_MAX_QUEUE_CNT];
static struct key_manager key_manager = {0, };
static struct key_map_entry key_map_entry[KEY_MAPPING_TABLE_COUNT] = 
{
    /*  0   */  {   KEY_NONE        ,   KEY_NONE        },
    /*  1   */  {   KEY_ESC         ,   KEY_ESC         },
    /*  2   */  {   '1'             ,   '!'             },
    /*  3   */  {   '2'             ,   '@'             },
    /*  4   */  {   '3'             ,   '#'             },
    /*  5   */  {   '4'             ,   '$'             },
    /*  6   */  {   '5'             ,   '%'             },
    /*  7   */  {   '6'             ,   '^'             },
    /*  8   */  {   '7'             ,   '&'             },
    /*  9   */  {   '8'             ,   '*'             },
    /*  10  */  {   '9'             ,   '('             },
    /*  11  */  {   '0'             ,   ')'             },
    /*  12  */  {   '-'             ,   '_'             },
    /*  13  */  {   '='             ,   '+'             },
    /*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
    /*  15  */  {   KEY_TAB         ,   KEY_TAB         },
    /*  16  */  {   'q'             ,   'Q'             },
    /*  17  */  {   'w'             ,   'W'             },
    /*  18  */  {   'e'             ,   'E'             },
    /*  19  */  {   'r'             ,   'R'             },
    /*  20  */  {   't'             ,   'T'             },
    /*  21  */  {   'y'             ,   'Y'             },
    /*  22  */  {   'u'             ,   'U'             },
    /*  23  */  {   'i'             ,   'I'             },
    /*  24  */  {   'o'             ,   'O'             },
    /*  25  */  {   'p'             ,   'P'             },
    /*  26  */  {   '['             ,   '{'             },
    /*  27  */  {   ']'             ,   '}'             },
    /*  28  */  {   '\n'            ,   '\n'            },
    /*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
    /*  30  */  {   'a'             ,   'A'             },
    /*  31  */  {   's'             ,   'S'             },
    /*  32  */  {   'd'             ,   'D'             },
    /*  33  */  {   'f'             ,   'F'             },
    /*  34  */  {   'g'             ,   'G'             },
    /*  35  */  {   'h'             ,   'H'             },
    /*  36  */  {   'j'             ,   'J'             },
    /*  37  */  {   'k'             ,   'K'             },
    /*  38  */  {   'l'             ,   'L'             },
    /*  39  */  {   ';'             ,   ':'             },
    /*  40  */  {   '\''            ,   '\"'            },
    /*  41  */  {   '`'             ,   '~'             },
    /*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
    /*  43  */  {   '\\'            ,   '|'             },
    /*  44  */  {   'z'             ,   'Z'             },
    /*  45  */  {   'x'             ,   'X'             },
    /*  46  */  {   'c'             ,   'C'             },
    /*  47  */  {   'v'             ,   'V'             },
    /*  48  */  {   'b'             ,   'B'             },
    /*  49  */  {   'n'             ,   'N'             },
    /*  50  */  {   'm'             ,   'M'             },
    /*  51  */  {   ','             ,   '<'             },
    /*  52  */  {   '.'             ,   '>'             },
    /*  53  */  {   '/'             ,   '?'             },
    /*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
    /*  55  */  {   '*'             ,   '*'             },
    /*  56  */  {   KEY_LALT        ,   KEY_LALT        },
    /*  57  */  {   ' '             ,   ' '             },
    /*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
    /*  59  */  {   KEY_F1          ,   KEY_F1          },
    /*  60  */  {   KEY_F2          ,   KEY_F2          },
    /*  61  */  {   KEY_F3          ,   KEY_F3          },
    /*  62  */  {   KEY_F4          ,   KEY_F4          },
    /*  63  */  {   KEY_F5          ,   KEY_F5          },
    /*  64  */  {   KEY_F6          ,   KEY_F6          },
    /*  65  */  {   KEY_F7          ,   KEY_F7          },
    /*  66  */  {   KEY_F8          ,   KEY_F8          },
    /*  67  */  {   KEY_F9          ,   KEY_F9          },
    /*  68  */  {   KEY_F10         ,   KEY_F10         },
    /*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
    /*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },

    /*  71  */  {   KEY_HOME        ,   '7'             },
    /*  72  */  {   KEY_UP          ,   '8'             },
    /*  73  */  {   KEY_PAGEUP      ,   '9'             },
    /*  74  */  {   '-'             ,   '-'             },
    /*  75  */  {   KEY_LEFT        ,   '4'             },
    /*  76  */  {   KEY_CENTER      ,   '5'             },
    /*  77  */  {   KEY_RIGHT       ,   '6'             },
    /*  78  */  {   '+'             ,   '+'             },
    /*  79  */  {   KEY_END         ,   '1'             },
    /*  80  */  {   KEY_DOWN        ,   '2'             },
    /*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
    /*  82  */  {   KEY_INS         ,   '0'             },
    /*  83  */  {   KEY_DEL         ,   '.'             },
    /*  84  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  85  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  86  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  87  */  {   KEY_F11         ,   KEY_F11         },
    /*  88  */  {   KEY_F12         ,   KEY_F12         }
};

/* 
 * [ unsigned char wait_for_ack(void) ]
 * wait for keyboard ACK and in case scan code is in the buffer,
 * put it keyboard queue
 * returns 1 when received ACK, or 0
 */

unsigned char wait_for_ack(void)
{
	int i,j;
	unsigned char data;
	unsigned char res = 0;

	for( i=0; i<100;i++ )
	{
		for( j=0; j<0xFFFF; j++)
		{
			if( is_obuffer_full() )
				break;
		}
		data = in_b(0x60);

		/* ACK: 0xFA */
		if( data == 0xFA ) 
		{
			res = 1;
			break;
		}
		else
			put_scan_code(data);
	}
	return res;
}

/* 
 * [ unsigned char put_scan_code(unsigned char scan_code) ]
 * convert scan_code to ascii code and put both in key_queue
 */
unsigned char put_scan_code(unsigned char scan_code)
{
	struct key_data data;
	unsigned char res = 0;
	unsigned char prev_interrupt;

	data.scan_code = scan_code;

	if( code_to_ascii( scan_code, &(data.ascii_code), &(data.flags) ) )
	{
		prev_interrupt = lock_system();
		/* putting stack data here???? */
		res = put_queue( &key_queue, &data );

		unlock_system(prev_interrupt);
	}
	return res;
}

/* 
 * [ unsigned char get_key_data(struct key_data* data) ]
 * get key_data from key_queue
 */
unsigned char get_key_data( struct key_data *data )
{
	unsigned char res;
	unsigned char prev_interrupt;

	if( is_queue_empty(&key_queue) )
		return 0;
	prev_interrupt = lock_system();
	res = get_queue( &key_queue, data );

	unlock_system(prev_interrupt);
	return res;
}


static unsigned char is_alphabet( unsigned char code )
{
	if ( ( 'a'<=key_map_entry[code].normal ) &&
		 ( key_map_entry[code].normal <='z' ) )
		return 1;
	return 0;
}

static unsigned char is_number_or_symbol( unsigned char code )
{
	if ( ( is_alphabet(code) == 0 ) &&
		 ( 2 <= code ) && ( code <= 53 ) )
		return 1;
	return 0;
}

static unsigned char is_numpad( unsigned char code )
{
	if( ( 71<=code ) && (code <=83 ) )
		return 1;
	return 0;
}

static unsigned char is_tobe_combined( unsigned char code )
{
	unsigned char combined;

	code = code & 0x7F;

	if( is_alphabet(code) )
	{
		if ( key_manager.shift_down ^ key_manager.caps_on )
			combined = 1;
		else
			combined = 0;
	}
	else if ( is_number_or_symbol( code ) )
	{
		if ( key_manager.shift_down )
			combined = 1;
		else
			combined = 0;
	}
	else if ( is_numpad(code) )
	{
		if( key_manager.num_on )
			combined = 1;
		else
			combined = 0;
	}

	return combined;
}

void update_keystate( unsigned char code )
{
	unsigned char down;
	unsigned char LED_changed;

	if( code & 0x80 )
	{
		down = 0;
		code = code & 0x7F;
	}
	else
		down = 1;

	if( ( code == 42 ) || ( code == 54 ) )
		key_manager.shift_down = down;
	else if ( ( code == 58 ) && ( down == 1 ) )
	{
		key_manager.caps_on ^= 1;
		LED_changed = 1;
	}
	else if ( ( code == 69 ) && ( down == 1 ) )
	{
		key_manager.num_on ^=1;
		LED_changed = 1;
	}
	else if ( ( code == 70 ) && ( down == 1 ) )
	{
		key_manager.scroll_on ^= 1;
		LED_changed = 1;
	}

	if ( LED_changed )
		change_led(key_manager.caps_on, key_manager.num_on, key_manager.scroll_on);
}

unsigned char code_to_ascii( unsigned char code, unsigned char* ascii, unsigned char* flags )
{
	if( key_manager.skip_count > 0 )
	{
		key_manager.skip_count--;
		return 0;
	}

	if ( code == 0xE1 ) // pause
	{
		*ascii = KEY_PAUSE;
		*flags = KEY_FLAGS_DOWN;
		key_manager.skip_count = KEY_SKIPCOUNT_FOR_PAUSE;
		return 1;
	}
	else if ( code == 0xE0 ) // extended
	{
		key_manager.ext_code = 1;
		return 0;
	}

	if( is_tobe_combined(code) )
		*ascii = key_map_entry[code&0x7F].combined;
	else
		*ascii = key_map_entry[code&0x7F].normal;

	if( key_manager.ext_code )
	{
		*flags = KEY_FLAGS_EXTENDEDKEY;
		key_manager.ext_code = 0;
	}
	else
		*flags = 0;

	if ( (code & 0x80) == 0 )
		*flags |= KEY_FLAGS_DOWN;

	update_keystate( code );
	return 1;
}


static void wait_ibuffer_empty()
{
	int i;
	for(i=0;i<0xFFFF;i++)
	{
		if( is_ibuffer_full() == 0 )
			return;
	}
}
static void wait_ibuffer_full()
{
	int i;
	for(i=0;i<0xFFFF;i++)
	{
		if( is_ibuffer_full() == 1 )
			return;
	}
}
static void wait_obuffer_full()
{
	int i;
	for(i=0;i<0xFFFF;i++)
	{
		if( is_ibuffer_full() == 1 )
			return;
	}
}
static void wait_obuffer_empty()
{
	int i;
	for(i=0;i<0xFFFF;i++)
	{
		if( is_obuffer_full() == 0 )
			return;
	}
}


/*
 * [ unsigned char init_keyboard(void) ]
 * initialize keyboard device
 * returns 1 when initialized properly or 0
 */
unsigned char init_keyboard(void)
{
	int i,j;
	unsigned char prev_interrupt;
	unsigned char res;

	init_queue( &key_queue, key_buffer, KEY_MAX_QUEUE_CNT, sizeof(struct key_data) );
	prev_interrupt = set_interrupt_flag(0);

	out_b(0x64, 0xAE);
	wait_ibuffer_empty();
	out_b(0x60, 0xF4);

	res = wait_for_ack();

	set_interrupt_flag( prev_interrupt );
	change_led(0, 0, 0);
	return res;
	
}

unsigned char get_keycode(void)
{
	while(is_obuffer_full() == 0);
	return in_b(0x60);
}

/*
 * [ unsigned char change_led(unsigned char caps, unsigned char num, unsigned char scroll) ]
 * change led state(caps lock, scroll lock, num lock)
 * returns 1 when successfully changed state, or 0
 */
unsigned char change_led(unsigned char caps, unsigned char num, unsigned char scroll)
{
	int i,j;
	unsigned char prev_interrupt;
	unsigned char res;
	unsigned char data;

	prev_interrupt = set_interrupt_flag(0);

	wait_ibuffer_empty();
	out_b(0x60, 0xED );

	wait_ibuffer_empty();

	res = wait_for_ack();

	if( !res )
	{
		set_interrupt_flag(prev_interrupt);
		return 0;
	}

	out_b(0x60, (caps<<2) | (num<<1) | scroll );

	wait_ibuffer_empty();

	res = wait_for_ack();
	set_interrupt_flag(prev_interrupt);
	return res;
}

void enable_a20gate(void)
{
	unsigned char data;
	int i;

	out_b(0x64, 0xD0 );

	wait_obuffer_full();
	data = in_b(0x60);
	data |= 0x01;

	wait_ibuffer_empty();
	out_b(0x64, 0xD1 );
	out_b(0x60, data);
}

void reboot(void)
{
	int i;

	wait_ibuffer_empty();
	out_b(0x64, 0xD1 );
	out_b(0x60, 0x0 );

	while(1);
}

