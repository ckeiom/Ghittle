#ifndef __KMEM_H__
#define __KMEM_H__

#define KMEM_ADDR	0x1000000
#define KMEM_SIZE	0xFF000000
#define KMEM_PAGE_SIZE	0x1000
#define KMEM_NUM_PAGES	(KMEM_SIZE / KMEM_PAGE_SIZE)

void init_kmem(void);
void* alloc_kmem(void);

#endif
