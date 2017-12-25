#ifndef __KERN32_H__
#define __KERN32_H__

void read_cpuid(unsigned int in_eax, 
				unsigned int *eax, 
				unsigned int *ebx, 
				unsigned int *ecx, 
				unsigned int *edx);
void switch_to_64bit(void);

#define PBOOT_FLAG	*((unsigned char* )0x7DFC )

#define KMEM_START 	0x100000
#define KMEM_SIZE	0x500000

#endif /*__TYPES_H__*/
