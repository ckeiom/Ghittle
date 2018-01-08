#include <interrupt.h>
#include <pic.h>
#include <keyboard.h>
#include <console.h>
#include <memutils.h>
#include <task.h>
#include <desc.h>
#include <timer.h>
#include <hdd.h>

void exception_handler( int num, unsigned long e_code )
{
	char buf[3] = {0, };

	buf[0] = '0' + num/10;
	buf[1] = '0' + num%10;

	while(1);
}

void interrupt_handler( int num )
{
	char buf[] = "[INT:  , ]";
	static int int_cnt = 0;

	buf[5] = '0'+num/10;
	buf[6] = '0'+num%10;

	buf[8] = '0'+int_cnt++;

	eoi_PIC( num - PIC_IRQ_START_VECTOR );

}

void keyboard_handler( int num )
{
	char buf[] = "[INT:  , ]";
	static int int_cnt = 0;
	char tmp;

	buf[5] = '0'+num/10;
	buf[6] = '0'+num%10;
	buf[8] = '0'+int_cnt++;

	if( is_obuffer_full() )
	{
		tmp = get_keycode();
		put_scan_code(tmp);
	}

	eoi_PIC( num - PIC_IRQ_START_VECTOR );
}

/*
 * [ timer_handler( int num ) ]
 *
 * Everytime timer get invoked,
 * scheduler decrease its CPU time
 * When promised time expired, try to switch another task in readylist
 */
void timer_handler( int num )
{
	char buf[] = "[INT:  , ]";
	static int count = 0;

	static int periodic = 0;
	buf[5] = '0' + num/10;
	buf[6] = '0' + num%10;
	buf[8] = '0' + count;

	count = (count+1)%10;

	eoi_PIC( num - PIC_IRQ_START_VECTOR );

	tick++;

	dec_cputime();

	if( is_cputime_expired() )
	{
		schedule_in_interrupt();
	}
}

void hdd_handler( int num )
{
	char buf[] = "[INT:  , ]";
	static int hdd_intr_count = 0;
	unsigned char temp;

	buf[5] = '0' + num / 10;
	buf[6] = '0' + num % 10;
	buf[8] = '0' + hdd_intr_count;
	hdd_intr_count = ( hdd_intr_count + 1 ) % 10;
	print_string_xy( 10, 0, buf );

	if( num - PIC_IRQ_START_VECTOR == 14 )
		set_hdd_intr_flag( 1, 1 );
	else
		set_hdd_intr_flag( 0, 1 );

	eoi_PIC( num - PIC_IRQ_START_VECTOR );
}

unsigned char set_interrupt_flag( unsigned char enable_interrupt )
{
	unsigned long rflags;

	rflags = read_rflags();

	if( enable_interrupt )
		enable_interrupt();
	else
		disable_interrupt();

	if(rflags & 0x0200)
		return 1;
	return 0;
}
