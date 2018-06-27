#ifndef __KMEM_H__
#define __KMEM_H__

#define KMEM_ADDR	(unsigned long)0x1000000
#define KMEM_SIZE	(unsigned long)0x1000000
#define KMEM_PAGE_SIZE	(unsigned long)0x1000
#define KMEM_NUM_PAGES	(KMEM_SIZE / KMEM_PAGE_SIZE)

#define KMEM_SHARD_SIZE			16
#define KMEM_SHARD_PAGES		8
#define KMEM_NUM_SHARD		(KMEM_SHARD_PAGES * KMEM_PAGE_SIZE / \
							 KMEM_SHARD_SIZE)
#define kfree(addr)		__kfree((unsigned long)addr, sizeof(typeof(addr)))

void init_kmem(void);
void* kpage_alloc(int num);
void* kmalloc(int size);
void kpage_free(unsigned long addr, int num);
void __kfree(unsigned long addr, unsigned int size);

#endif
