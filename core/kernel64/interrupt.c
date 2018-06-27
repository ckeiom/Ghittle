/* This file is initially written by J. Hyun on 20171123 */

#include <interrupt.h>
#include <pic.h>
#include <keyboard.h>
#include <console.h>
#include <memutils.h>
#include <task.h>
#include <sched.h>
#include <desc.h>
#include <timer.h>
#include <hdd.h>

void exception_handler(int num, unsigned long e_code)
{
	print_string_xy(50, 24, "Exception");
	while(1);
}

void interrupt_handler(int num)
{
	print_string_xy(40, 24, "Interrupt");
	eoi_PIC(num - PIC_IRQ_START_VECTOR);
}

void keyboard_handler(int num)
{
	char tmp;
	print_string_xy(30, 24, "Keyboard");
	if(is_obuffer_full())
	{
		tmp = get_keycode();
		put_scan_code(tmp);
	}
	eoi_PIC(num - PIC_IRQ_START_VECTOR);
}

/*
 * [ timer_handler(int num) ]
 *
 * Everytime timer get invoked (default set is 1ms),
 * scheduler decrease its CPU time
 * When promised time expired, try to switch another task in readylist
 */
void timer_handler(int num)
{
	eoi_PIC(num - PIC_IRQ_START_VECTOR);
	tick++;

	dec_cputime();

	if(is_cputime_expired())
		schedule_in_interrupt();
}

/* Hyunsub added hdd_handler on 20180108 */
void hdd_handler(int num)
{
	if(num - PIC_IRQ_START_VECTOR == 14)
		set_hdd_interrupt_state(HDD_PRIMARY, HDD_INTERRUPT);
	else
		set_hdd_interrupt_state(HDD_SECONDARY, HDD_INTERRUPT);

	eoi_PIC(num - PIC_IRQ_START_VECTOR);
}

unsigned char set_interrupt_flag(unsigned char enable_interrupt)
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
