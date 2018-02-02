/* This file is initially written by J. Hyun on 20180129 */

#include <kmem.h>
#include <bitmap.h>
#include <console.h>
#include <page.h>

/* kmem stands for kernel dynamic memory pool */

static char kmem_map[(KMEM_NUM_PAGES-1)/8];
static struct bitmap kmem_bitmap;

void init_kmem(void)
{
	init_bitmap(&kmem_bitmap, KMEM_NUM_PAGES, kmem_map);
}

/* 
 * alloc_kmem(void) allocates only one page (4K) for now
 * no slab.. just a page! 
 */
void* alloc_kmem(void)
{
	unsigned long loc;
	void* phys;
	void* virt;

	phys = alloc_pages(1);

	printk("phys: %x\n",phys);
	for(loc = 0; loc < KMEM_NUM_PAGES; loc++)
	{
		if(bitmap_test_set(&kmem_bitmap, loc) == 0)
			break;
	}

	if(loc >= KMEM_NUM_PAGES)
	{
		printk("No more kmem to allocate\n");
		return 0;
	}
	virt = (void*)(KMEM_ADDR + KMEM_PAGE_SIZE * loc);
	setup_page(PTE_DEFAULT, (unsigned long)phys, (unsigned long)virt);
	
	return virt;
}

