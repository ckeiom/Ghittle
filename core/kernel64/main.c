/* This file is initially written by J. Hyun Kim on 2017.12.24 */

#include <console.h>
#include <desc.h>
#include <asmutils.h>
#include <pit.h>
#include <keyboard.h>
#include <pic.h>
#include <task.h>
#include <interrupt.h>
#include <sched.h>
#include <hdd.h>
#include <filesys.h>
#include <page.h>
#include <kmem.h>

void main(void)
{
	int x,y;
	int err;

	init_console(0, 10);
	printk("Console initialized\n");

	printk("Initializing GDT...\n");
	init_GDT_TSS();

	load_gdtr(GDTR_START_ADDR);

	printk("Loading TSS segment...\n");
	load_tss(GDT_TSS_SEGMENT);

	printk("Initializing IDT...\n");
	init_IDT();
	load_idtr(IDTR_START_ADDR);
	
	printk("Initializing taskpool & scheduler\n");

	init_scheduler();
	init_pit( MS_TO_COUNT(1), 1 );

	err = init_keyboard();
	if(err < 0)
		goto ERR_OUT;

	printk("Initializing PIC...\n");
	init_PIC();
	mask_PIC(0);
	enable_interrupt();

	err = init_fs();
	if(err < 0)
		goto ERR_OUT;

	PBOOT_FLAG = 1;

	init_page_pool();
	init_kmem();
	printk("Boot finished\n");


	char *km = alloc_kmem();
	printk("KM: %x\n", km);
	*km = 3;

	printk("ll\n");
	idle_task();
ERR_OUT:
	printk("PANIC\n");
	while(1);

}


