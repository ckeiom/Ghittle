#include <pic.h>
#include <asmutils.h>
#include <types.h>
#include <dio.h>

void init_PIC(void)
{
	out_b(PIC_MASTER_PORT1, 0x11);
	out_b(PIC_MASTER_PORT2, PIC_IRQ_START_VECTOR);
	out_b(PIC_MASTER_PORT2, 0x04);
	out_b(PIC_MASTER_PORT2, 0x01);

	out_b(PIC_SLAVE_PORT1, 0x11);
	out_b(PIC_SLAVE_PORT2, PIC_IRQ_START_VECTOR+8 );
	out_b(PIC_SLAVE_PORT2, 0x02);
	out_b(PIC_SLAVE_PORT2, 0x01);
}

void mask_PIC(unsigned short mask)
{
	out_b(PIC_MASTER_PORT2, (unsigned char)mask );
	out_b(PIC_SLAVE_PORT2, (unsigned char)(mask>>8) );
}

void eoi_PIC(int num)
{
	out_b(PIC_MASTER_PORT1, 0x20);
	if( num >=8 )
		out_b(PIC_SLAVE_PORT1, 0x20);
}
