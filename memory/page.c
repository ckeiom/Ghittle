#include <page.h>

void init_page_pool(void)
{
	/* Kernel pool range is 0x800000 ~ 0x10000000
	   User pool range is 0x10000000 ~ 0x1000000000 
	   Create a bitmap as a specifier while allocating a page in page pool 
	   bitmap.c in utils/ bitmap.h include */

}


/* mode = 0 for kernel, higher for user */

void* alloc_pages(int mode, int num)
{
	/* Kernel pool range is 0x800000 ~ 0x10000000
	   User pool range is 0x10000000 ~ 0x1000000000 */
}

/* Link virtual page address to physical address, 
   that is, modification in page table */

int setup_page(int mode, unsigned long virt, unsigned long phys)
{
	/* You may regard pgtable base address is 0x100000 for now */
}
