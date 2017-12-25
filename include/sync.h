#ifndef __SYNC_H__
#define __SYNC_H__

#include <interrupt.h>

#pragma pack( push, 1 )
struct mutex
{
	volatile unsigned long tid;
	volatile unsigned int count;

	volatile unsigned char flag;
	unsigned char padding[3];
};

#pragma pack(pop)

static inline unsigned char lock_system( void )
{
	return set_interrupt_flag(0);
}
static inline void unlock_system( unsigned char flag )
{
	set_interrupt_flag( flag );
}

void init_mutex(struct mutex* m);
void lock(struct mutex* m);
void unlock(struct mutex* m);
unsigned char test_set( volatile unsigned char* dst, unsigned char comp, unsigned char src );

#endif
