#include <timer.h>

volatile unsigned long tick;

void mdelay(unsigned long msec)
{
	unsigned long start_tick = tick;
	unsigned long cur_tick;
	
	while(1)
	{
		cur_tick = tick;
		if(cur_tick - start_tick > msec)
			break;
	}
}
