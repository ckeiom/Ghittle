#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL 0

#define KERN64_MEM_START 	0x100000
#define KERN64_MEM_END		0x600000

#define NUM_BOOT_SECTOR		1
#define NUM_KERNEL_SECTOR	*((unsigned short* )0x7C05)
#define PBOOT_FLAG		*((unsigned char* )0x7DFC)

#define offsetof( type, member ) \
	( (long)&((type* )0)->member )

#define container_of( ptr, type, member) ( \
		{ const typeof( ((type *)0)->member ) *__mptr = (ptr); \
		(type *)( (char *)__mptr - offsetof(type, member) );} )

#endif /*__TYPES_H__*/
