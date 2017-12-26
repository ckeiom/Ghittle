#include <console.h>
#include <desc.h>
#include <asmutils.h>
#include <pit.h>
#include <keyboard.h>
#include <pic.h>
#include <task.h>
#include <interrupt.h>
#include <sched.h>

void main(void)
{
	int x,y;
	int err;

	init_console(0,10);
	printk("Console initialized\n");

	get_cursor( &x, &y );

	printk("Initializing GDT...\n");
	init_GDT_TSS();

	load_gdtr( GDTR_START_ADDR );

	printk("Loading TSS segment...\n");
	load_tss( GDT_TSS_SEGMENT );

	printk("Initializing IDT...\n");
	init_IDT();
	load_idtr(IDTR_START_ADDR);
	
	printk("Initializing taskpool & scheduler\n");

	init_scheduler();
	init_pit( MS_TO_COUNT(1), 1 );
	err = init_keyboard();
	if( err < 0 )
		goto ERR_OUT;

	change_led(0, 0, 0);
	printk("Initializing PIC...\n");
	init_PIC();
	mask_PIC(0);
	enable_interrupt();

	PBOOT_FLAG = 1;

	create_task( SCHED_PRIO_LOWEST | TASK_FLAGS_IDLE, (unsigned long)idle_task);

	/* Include testing headers here */

	/* TEST BOARD */
	printk("TEST\n");


ERR_OUT:
	while(1);

}


