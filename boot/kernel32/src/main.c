#include <kern32.h>
#include <page.h>
#include <console.h>

void print_string( int x, int y, const char *str );
int init_kern64_mem();
int validate_64bit_mode();
int copy_kern64();

void main(void)
{
	int err;

	print_string( 0, 3, "[[[ Kernel entry ]]]");
	print_string( 0, 4, "Initializing 64bit memory...");

	err = init_kern64_mem();
	
	if (err < 0)
		goto ERR_OUT;

	print_string( 0, 5, "Initializing 64bit pagetable...");

	err = init_pagetable();

	if (err < 0)
		goto ERR_OUT;

	print_string( 0, 6, "Validating 64bit mode...");
	err = validate_64bit_mode();
	
	if (err < 0)
		goto ERR_OUT;

	print_string( 0, 8, "Copying 64bit kernel image...");

	err = copy_kern64();

	if (err < 0)
		goto ERR_OUT;

	print_string( 0, 9, "Switching to 64bit mode...");
	switch_to_64bit();
ERR_OUT:
	
	print_string( 30, 20, "ERROR!!!");
	while(1);
}

void print_string( int x, int y, const char* str )
{
	struct console_buffer *scr = (struct console_buffer *) VEDIO_MEM_START;
	int i;

	scr += ( y*80) + x;
	for( i=0; str[i] !=0 ; i++)
		scr[i].ch = str[i];
}


/* 
 * Initialize memory for 64bits kernel area 
 * setting entire memory to be used by kernel zero
 */
int init_kern64_mem()
{
	unsigned int* pos;
	
	if(PBOOT_FLAG)
		return 0;
	
	pos = (unsigned int *)KMEM_START;

	while( (unsigned int)pos < (KMEM_START + KMEM_SIZE) )
	{
		*pos = 0;
		if ( *pos != 0 )
			return -1;
		pos++;
	}
	return 0;
}

int validate_64bit_mode()
{
	unsigned int eax, ebx, ecx, edx;
	char vendor_str[16]={0, };
	
	read_cpuid( 0x0, &eax, &ebx, &ecx, &edx );

	*((unsigned int*)vendor_str )= ebx;
	*((unsigned int*)vendor_str + 1 )= edx;
	*((unsigned int*)vendor_str + 2 )= ecx;

	print_string( 3, 7, "vendor:");
	print_string( 11, 7, vendor_str );

	read_cpuid( 0x80000001, &eax, &ebx, &ecx, &edx );

	if ( !( edx & (1<<29 ) ) )
		return -1;
	return 0;
}

int copy_kern64()
{
	unsigned short kern32_sec_cnt, total_sec_cnt;
	unsigned int* src, * dst;
	int i;

	if(PBOOT_FLAG)
		return 0;

	total_sec_cnt = *((unsigned short*)0x7C05);
	kern32_sec_cnt = *((unsigned short*)0x7C07);

	src = (unsigned int* )( 0x10000 + (kern32_sec_cnt*512) );
	dst = (unsigned int* )(KMEM_START + 0x100000);

	for( i=0;i<512*(total_sec_cnt - kern32_sec_cnt)/4;i++ )
		*dst++ = *src++;

}


int init_pagetable()
{
	struct pd* d;
	struct pte* e;
	int i;
	unsigned int map_addr, map_high_addr;

	if(PBOOT_FLAG)
		return 0;

	/* PGD table */
	d = (struct pd*)PGD_START;

	set_pd_32(d, 0, PMD_START, PD_DEFAULT);

	/* We have only one PGD entry */
	for (i=1; i<PAGE_MAXENTRY_COUNT; i++)
		set_pd_32( &(d[i]), 0, 0, 0);
	
	/* PMD table */

	d = (struct pd*)PMD_START;

	/* and we have 64 PMD entries */
	for(i=0; i<NUM_PMD_ENTRY; i++)
		set_pd_32( &(d[i]), 0, PD_START + (i*PAGETABLE_SIZE), PD_DEFAULT);

	for(i=NUM_PMD_ENTRY; i<PAGE_MAXENTRY_COUNT; i++)
		set_pd_32( &(d[i]), 0, 0, 0);

	/* PD table */
	e = (struct pte*) PD_START;
	map_addr = 0;
	map_high_addr = 0;
	for (i=0; i<NUM_PMD_ENTRY*NUM_PD_ENTRY; i++)
	{
		set_pte_32( &(e[i]), map_high_addr ,map_addr, PTE_DEFAULT | PTE_PAE);
		map_addr += PAGE_SIZE;
		if(map_addr == 0)
			map_high_addr++;
	}
	return 0;
}

void set_pte_32(struct pte* e, 
				unsigned int addr_high,
			    unsigned int addr_low,
	 	     	unsigned short flags)
{
	e->low = addr_low | flags;
	e->high = addr_high & 0xFF;
}

void set_pd_32(struct pd* d, 
			   unsigned int addr_high,
			   unsigned int addr_low,
	    	   unsigned short flags)
{
	d->low = addr_low | flags;
	d->high = addr_high & 0xFF;
}
