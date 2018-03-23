/* This file is initially written by J. Hyun on 20180129 */

#include <kmem.h>
#include <bitmap.h>
#include <console.h>
#include <page.h>

/* kmem stands for kernel dynamic memory pool */

static char kmem_map[(KMEM_NUM_PAGES-1)/8];
static char kmem_shard_map[(KMEM_NUM_SHARD-1)/8];
static struct bitmap kmem_bitmap;
static struct bitmap kmem_shard_bitmap;

static char* kmem_shard_pool;

void init_kmem(void)
{
	init_bitmap(&kmem_bitmap, KMEM_NUM_PAGES, kmem_map);
	init_bitmap(&kmem_shard_bitmap, KMEM_NUM_SHARD, kmem_shard_map);
	kmem_shard_pool = alloc_kpage(KMEM_SHARD_PAGES);
}

/* 
 * alloc_kmem(void) allocates only one page (4K) for now
 * no slab.. just a page! 
 * -> Now we have 16byte KMEM_SHARDs!
 * shards are 16byte aligned, allocated through kmalloc(int size)
 */
void* alloc_kpage(int num)
{
	int loc;
	
	loc = bitmap_find_free(&kmem_bitmap, num);

	if(loc < 0)
		return 0;

	bitmap_set(&kmem_bitmap, loc, num);

	return (void *)(KMEM_ADDR + KMEM_PAGE_SIZE * loc);
}

void free_kpage(unsigned long addr, int num)
{
	int loc;

	loc = (int)((addr - KMEM_ADDR) / KMEM_PAGE_SIZE);
	bitmap_clear(&kmem_bitmap, loc, num);
}

void* kmalloc(int size)
{
	int loc;
	int num;

	num = (size + KMEM_SHARD_SIZE - 1) / KMEM_SHARD_SIZE;

	loc = bitmap_find_free(&kmem_shard_bitmap, num); 

	if(loc < 0)
		return 0;

	bitmap_set(&kmem_shard_bitmap, loc, num);

	return (void *)(kmem_shard_pool + KMEM_SHARD_SIZE * loc);
}

void __kfree(unsigned long addr, unsigned int size)
{
	int loc;
	unsigned int num;

	loc = (int)((addr - (unsigned long)kmem_shard_pool) / KMEM_SHARD_SIZE);
	num = (size + KMEM_SHARD_SIZE - 1) / KMEM_SHARD_SIZE;
	bitmap_clear(&kmem_shard_bitmap, loc, num);
}
	

