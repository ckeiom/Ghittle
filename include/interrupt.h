#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

void exception_handler( int num, unsigned long e_code );
void interrupt_handler( int num );
void timer_handler( int num );
void keyboard_handler( int num );
void hdd_handler( int num );

unsigned char set_interrupt_flag( unsigned char enable_interrupt );

#define enable_interrupt() asm __volatile__ ("sti")
#define disable_interrupt() asm __volatile__ ("cli")

#endif
