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

/* These only cover Static kernel memory */
#define PGD_START		0x100000 /* Only 1 PGD entry */
#define PMD_START		0x101000 /* Only 1 PMD entries */
#define PD_START		0x102000 /* 8 PTEs */

#define NUM_PGD_ENTRY	1
#define NUM_PMD_ENTRY	1
#define NUM_PD_ENTRY	8

#define PAGETABLE_SIZE	0x1000
#define SPAGE_SIZE 		0x200000 /* Static Page size: 2MB */
#define DPAGE_SIZE		0x1000
#define PAGE_MAXENTRY_COUNT		512


/* 
   Virtual memory
   0x0			~ 0x100 0000: 	Static kernel area
   0x100 0000	~ 0x1000 0000: 	Dynamic kernel area 
   0x1000 0000	~ 				User area

   Physical memory
   0x0			~ 0x100 0000:	Static memory (Kernel)
   0x100 0000	~				Dynamic memory
 */
#define PAGE_KERN_POOL_ADDR		0x1000000
#define PAGE_KERN_POOL_SIZE		0xFF000000
#define PAGE_USER_POOL_ADDR		0x100000000
#define PAGE_USER_POOL_SIZE		0xF00000000

#define PAGE_PHYS_POOL_ADDR	0x1000000
#define PAGE_PHYS_POOL_SIZE 0xFFF000000
#define NUM_PHYS_PAGES	(PAGE_PHYS_POOL_SIZE / DPAGE_SIZE)

#define PGD_OFFSET(addr)	((addr >> 39) & 0x1FF)
#define PUD_OFFSET(addr)	((addr >> 30) & 0x1FF)
#define PMD_OFFSET(addr)	((addr >> 21) & 0x1FF)
#define PD_OFFSET(addr)		((addr >> 12) & 0x1FF)

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
			 unsigned long addr,
			 unsigned short flags);
void set_pd(struct pd* d,
			unsigned long addr,
			unsigned short flags);

void* alloc_pages(int num);
int setup_page(int flags, unsigned long phys, unsigned long virt);
void init_page_pool(void);

unsigned long get_pud_addr(unsigned long pgd, unsigned long addr);
unsigned long get_pmd_addr(unsigned long pud, unsigned long addr);
unsigned long get_pte_addr(unsigned long pmd, unsigned long addr);
#endif /* __PAGE_H__ */
