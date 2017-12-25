#ifndef __PAGE_H__
#define __PAGE_H__

#define PTE_PRESENT		0x1
#define PTE_WRITABLE	0x2
#define PTE_USER		0x4
#define PTE_DIRECT		0x8  /* Write through cache policy, default: write back*/
#define PTE_CACHEABLE	0x10
#define PTE_ACCESSED	0x20
#define PTE_DIRTY		0x40
#define PTE_PAE			0x80
#define PTE_GLOBAL		0x100 /* If set, no TLB replacement when CR3 changes */
/* 0x200, 0x400, 0x800 are available */
#define PTE_PAT		0x1000 /* Page Attribute Table Index */

#define PTE_DEFAULT	( PTE_PRESENT | PTE_WRITABLE )

#define PD_PRESENT		0x1
#define PD_WRITABLE		0x2
#define PD_USER			0x4
#define PD_DIRECT		0x8  /* Write through cache policy, default: write back*/
#define PD_CACHEABLE	0x10
#define PD_ACCESSED		0x20
/* 0x40, 0x80, 0x100 are reserved */
/* 0x200, 0x400, 0x800 are available */

#define PD_DEFAULT	( PD_PRESENT | PD_WRITABLE )

#define PGD_START		0x100000 /* Only 1 PGD entry */
#define PMD_START		0x101000 /* Only 64 PMD entries */
#define PD_START		0x102000 /* 64 * 512 PTE entries */

#define NUM_PGD_ENTRY	1
#define NUM_PMD_ENTRY	64
#define NUM_PD_ENTRY	512

#define PAGETABLE_SIZE	0x1000
#define PAGE_SIZE 	0x200000 /* Page size: 2MB */
#define PAGE_MAXENTRY_COUNT	0x200

#pragma pack(push,1)
struct pte
{
	unsigned int low;
	unsigned int high;
};
#pragma pack(pop)

#pragma pack(push,1)
struct pd
{
	unsigned int low;
	unsigned int high;
};
#pragma pack(pop)

int init_pagetable(void);
void set_pte_32(struct pte* e,
			    unsigned int addr_high,
				unsigned int addr_low,
				unsigned short flags);
void set_pd_32(struct pd* d,
			   unsigned int addr_high,
			   unsigned int addr_low,
			   unsigned short flags);

void set_pte(struct pte* e,
			 unsigned long addr,
			 unsigned short flags);
void set_pd(struct pd* d,
			unsigned long addr,
			unsigned short flags);

void* alloc_pages(int mode, int num);
int setup_page(int mode, unsigned long virt, unsigned long phys);
void init_page_pool(void);
#endif /* __PAGE_H__ */
