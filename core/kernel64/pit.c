#include <pit.h>
#include <asmutils.h>
#include <dio.h>

void init_pit(unsigned short count, unsigned char periodic)
{
	/* Stop counter and set freqency of counter0 */
	out_b(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);

	if(periodic)
		out_b(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
	
	/* set initial counter value of counter0 */
	out_b(PIT_PORT_COUNTER0, count);
	out_b(PIT_PORT_COUNTER0, count >> 8);
}

unsigned short read_counter0(void)
{
	unsigned char high, low;

	/* ready to read the value of conter0 */
	out_b(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

	low = in_b(PIT_PORT_COUNTER0);
	high = in_b(PIT_PORT_COUNTER0);

	return ((high << 8) | low);
}

void wait_pit(unsigned short count)
{
	unsigned short last_count;
	unsigned short current_count;

	init_pit(0, 1);
	last_count = read_counter0();

	do
		current_count = read_counter0();
	while(((last_count - current_count) & 0xFFFF) < count);

}
