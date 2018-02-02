/* This file is initially written by J. Hyun on 20180119 */

#include <page.h>
#include <bitmap.h>
#include <console.h>

static char map[(NUM_PHYS_PAGES-1)/8];
static struct bitmap phy_page_bitmap;

void init_page_pool(void)
{
	/* Kernel pool range is 0x100 0000 ~ 0x1 0000 0000
	   User pool range is 0x1 0000 0000 ~ 0x10 0000 0000
	   Create a bitmap as a specifier while allocating a page in page pool 
	   bitmap.c in utils/ bitmap.h include */
	init_bitmap(&phy_page_bitmap, NUM_PHYS_PAGES, map);
}


/* 
 * [alloc_pages(int num)] 
 * allocates 'num' continueous physical pages
 * current implementation allocates only one page whatever 'num' is
 * through first fit allocator
 */

void* alloc_pages(int num)
{
	unsigned long loc;

	for(loc = 0; loc < NUM_PHYS_PAGES; loc++)
	{
		if(bitmap_test_set(&phy_page_bitmap, loc) == 0)
			break;
	}
	if(loc >= NUM_PHYS_PAGES)
	{
		printk("No more pages to allocate\n");
		return 0;
	}

	return (void *)(PAGE_PHYS_POOL_ADDR + DPAGE_SIZE * loc);
}

/* Link physical address accquired from alloc_pages() to virtual address, 
   that is, modification in page table */

int setup_page(int flags, unsigned long phys, unsigned long virt)
{
}
