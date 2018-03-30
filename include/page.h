#ifndef __PAGE_H__
#define __PAGE_H__

#include <types.h>
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
#define PTE_MASK		0x7FFFFFFFFFFFF000


#define PD_PRESENT		0x1
#define PD_WRITABLE		0x2
#define PD_USER			0x4
#define PD_DIRECT		0x8  /* Write through cache policy, default: write back*/
#define PD_CACHEABLE	0x10
#define PD_ACCESSED		0x20
/* 0x40, 0x80, 0x100 are reserved */
/* 0x200, 0x400, 0x800 are available */

#define PD_DEFAULT	( PD_PRESENT | PD_WRITABLE )
#define PD_MASK			((u64)0x7FFFFFFFFFFFF000)

/* These only cover Static kernel memory */
#define PGD_START		0x100000 /* Only 1 PGD entry */
#define PMD_START		0x101000 /* Only 1 PMD entries */
#define PD_START		0x102000 /* 8 PTEs */

#define NUM_PGD_ENTRY	1
#define NUM_PMD_ENTRY	1
#define NUM_PD_ENTRY	16

#define PAGETABLE_SIZE	0x1000
#define SPAGE_SIZE 		0x200000 /* Static Page size: 2MB */
#define DPAGE_SIZE		0x1000
#define DPAGE_MASK		0xFFFFFFFFFFFFF000
#define PAGE_MAXENTRY_COUNT		512

#define PAGE_PHYS_ADDR	0x2000000
#define PAGE_PHYS_SIZE	0xE000000
#define NUM_PHYS_PAGES	(PAGE_PHYS_SIZE / DPAGE_SIZE)


/* 
   Virtual memory
   0x0			~ 0x100 0000: 	Static kernel area
   0x100 0000	~ 0x1000 0000: 	Dynamic kernel area 
   0x1000 0000	~ 				User area

   Physical memory
   0x0			~ 0x100 0000:	Static memory (Kernel)
   0x100 0000	~				Dynamic memory
 */

#define PGD_OFFSET(addr)	(((u64)addr >> 39) & 0x1FF)
#define PUD_OFFSET(addr)	(((u64)addr >> 30) & 0x1FF)
#define PMD_OFFSET(addr)	(((u64)addr >> 21) & 0x1FF)
#define PD_OFFSET(addr)		(((u64)addr >> 12) & 0x1FF)

#pragma pack(push, 1)
struct pte
{
	unsigned int low;
	unsigned int high;
};
#pragma pack(pop)

#pragma pack(push, 1)
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
			 u64 addr,
			 unsigned short flags);
void set_pd(struct pd* d,
			u64 addr,
			unsigned short flags);

void* alloc_pages(int num);
void init_page_pool(void);

struct pte* get_pte(int alloc, struct pd* pgd, u64 addr);

u64 get_pud_addr(u64 pgd, u64 addr);
u64 get_pmd_addr(u64 pud, u64 addr);
u64 get_pte_addr(u64 pmd, u64 addr);

static inline struct pd* get_pud(struct pd* pgd, u64 addr)
{
	return pgd + PGD_OFFSET(addr);
}
static inline struct pd* get_pmd(struct pd* pud, u64 addr)
{
	return pud + PUD_OFFSET(addr);
}
static inline struct pd* get_pd(struct pd* pmd, u64 addr)
{
	return pmd + PMD_OFFSET(addr);
}
static inline u64 pd_addr(struct pd* pd)
{
	u64* addr = (u64*)pd;
	return (*addr & PD_MASK);
}
static inline int pd_present(struct pd* pd)
{
	return (*((u64 *)pd) & PD_PRESENT)? 1:0;
}
static inline int pte_present(struct pte* pte)
{
	return (*((u64 *)pte) & PTE_PRESENT)? 1:0;
}
#endif /* __PAGE_H__ */
