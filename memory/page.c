/* This file is initially written by J. Hyun on 20180119 */

#include <page.h>
#include <bitmap.h>
#include <console.h>
#include <kmem.h>

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

void* page_alloc_phys(int num)
{
	u64 loc;

	loc = bitmap_find_free(&phy_page_bitmap, num);
	if(loc < 0)
	{
		printk("No more pages to allocate\n");
		return 0;
	}

	return (void *)(PAGE_PHYS_ADDR + DPAGE_SIZE * loc);
}

struct pte* get_pte(struct pd* pgd, u64 addr)
{
	struct pd *pud, *pmd, *pd;
	struct pte *pte;
	void* new_page;

	pud = get_pud(pgd, addr);

	if(!pd_present(pud))
		return 0;

	pud = (struct pd*)pd_addr(pud);
	pmd = get_pmd(pud, addr);

	if(!pd_present(pmd))
		return 0;

	pmd = (struct pd*)pd_addr(pmd);
	pd = get_pd(pmd, addr);

	if(!pd_present(pd))
		return 0;

	pd = (struct pd*)pd_addr(pd);
	pte = (struct pte*)(pd + PD_OFFSET(addr));

	if(!pte_present(pte))
		return 0;

	return pte;
}

void set_pd(struct pd* d, u64 addr, unsigned short flags)
{
	*((u64 *)d) = (addr & PD_MASK) | flags;
}

void set_pte(struct pte* e, u64 addr, unsigned short flags)
{
	*((u64 *)e) = (addr & PTE_MASK) | flags;
}

void page_setup(u64 phys_addr, u64 virt_addr, struct pd* pgd, short flags)
{
	struct pd *pud, *pmd, *pd;
	struct pte *pte;
	void *new_page;

	pud = get_pud(pgd, virt_addr);

	if(!pd_present(pud))
	{
		new_page = kpage_alloc(1);
		set_pd(pud, (u64)new_page, PD_DEFAULT);
	}

	pud = (struct pd*)pd_addr(pud);
	pmd = get_pmd(pud, virt_addr);

	if(!pd_present(pmd))
	{
		new_page = kpage_alloc(1);
		set_pd(pmd, (u64)new_page, PD_DEFAULT);
	}

	pmd = (struct pd*)pd_addr(pmd);
	pd = get_pd(pmd, virt_addr);

	if(!pd_present(pd))
	{
		new_page = kpage_alloc(1);
		set_pd(pd, (u64)new_page, PD_DEFAULT);
	}

	pd = (struct pd*)pd_addr(pd);
	pte = (struct pte*)(pd + PD_OFFSET(virt_addr));

	if(!pte_present(pte))
	{
		new_page = page_alloc_phys(1);
		set_pte(pte, (u64)new_page, flags);
	}
}
