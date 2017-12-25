#include <types.h>
#include <desc.h>
#include <isr.h>


static void set_gdt_entry8( struct gdt_entry8* ent, unsigned int addr, unsigned int limit, 
							unsigned char flags_4_7, unsigned char flags_0_3, unsigned char type )
{
	ent->limit_0_15 = limit&0xFFFF;
	ent->addr_0_15 = addr & 0xFFFF;
	ent->addr_16_23 = ( addr>>16)&0xFF;
	ent->type_flags_0_3 = flags_0_3 | type;
	ent->limit_16_19_flag_4_7 = ( (limit>>16)&0xFF) | flags_4_7;
	ent->addr_24_31 = ( addr >>24 ) & 0xFF;
}

static void set_gdt_entry16( struct gdt_entry16* ent, unsigned long addr, unsigned int limit, 
							 unsigned char flags_4_7, unsigned char flags_0_3, unsigned char type)
{
	ent->limit_0_15 = limit&0xFFFF;
	ent->addr_0_15 = addr & 0xFFFF;
	ent->addr_16_23 = ( addr>>16)&0xFF;
	ent->type_flags_0_3 = flags_0_3 | type;
	ent->limit_16_19_flag_4_7 = ( (limit>>16)&0xFF) | flags_4_7;
	ent->addr_24_31 = ( addr>>24 ) & 0xFF;
	ent->addr_32_63 = addr>>32;
	ent->res = 0;
}

static void set_idt_entry( struct idt_entry* ent, void* handler, unsigned short selector,
						   unsigned char ist, unsigned char flags, unsigned char type )
{
	ent->addr_0_15 = (unsigned long) handler & 0xFFFF;
	ent->selector = selector;
	ent->ist = ist&0x3;
	ent->type_flags = type | flags;
	ent->addr_16_31 = ( (unsigned long)handler >> 16 ) & 0xFFFF;
	ent->addr_32_63 = (unsigned long)handler >>32;
	ent->res = 0;
}

static void init_TSS( struct tss_data* tss)
{
	memset(tss, 0, sizeof(struct tss_data) );
	tss->ist[0] = 0x800000;
	tss->iomap_addr = 0xFFFF;
}

void dummy_handler(void)
{
	// print_string( 0, 15, "[[[ Dummy handler ]]]" );
}

void init_GDT_TSS(void)
{
	GDTR* gdtr;
	struct gdt_entry8* entry;
	struct tss_data* tss;


	gdtr = (GDTR *)GDTR_START_ADDR;
	entry = (struct gdt_entry8*) (GDTR_START_ADDR+sizeof(GDTR));
	gdtr->limit = GDT_TABLE_SIZE -1;
	gdtr->addr = (unsigned long)entry;
	
	tss = ( struct tss_data* )((unsigned long)entry + GDT_TABLE_SIZE );

	
	set_gdt_entry8( &(entry[0] ),0,0,0,0,0 );
	set_gdt_entry8( &(entry[1] ),0,0xFFFF,GDT_FLAGS_UPPER_CODE,GDT_FLAGS_LOWER_KERNEL_CODE, GDT_TYPE_CODE );
	set_gdt_entry8( &(entry[2] ),0,0xFFFF,GDT_FLAGS_UPPER_DATA,GDT_FLAGS_LOWER_KERNEL_DATA, GDT_TYPE_DATA );
	set_gdt_entry16( (struct gdt_entry16* ) &(entry[3]), (unsigned long)tss, sizeof(struct tss_data) -1, 
					  GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS );

	init_TSS(tss);

}

void init_IDT(void)
{
	IDTR *idtr;
	struct idt_entry* entry;
	int i;


	idtr = (IDTR* )IDTR_START_ADDR;
	entry = (struct idt_entry* )( IDTR_START_ADDR + sizeof(IDTR) );
	idtr->addr = (unsigned long)entry;
	idtr->limit = IDT_TABLE_SIZE - 1;

	set_idt_entry(&(entry[0]), ISR_div0, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[1]), ISR_debug, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[2]), ISR_nmi, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[3]), ISR_breakpoint, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[4]), ISR_overflow, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[5]), ISR_boundrange, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[6]), ISR_invopcode, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[7]), ISR_nodev, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[8]), ISR_df, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[9]), ISR_copoverrun, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[10]), ISR_invtss, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[11]), ISR_noseg, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[12]), ISR_sf, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[13]), ISR_gp, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[14]), ISR_pf, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[15]), ISR_r15, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[16]), ISR_fpu, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[17]), ISR_align, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[18]), ISR_machine, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[19]), ISR_simd, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	
	for(i=20; i<32; i++)
		set_idt_entry(&(entry[i]), ISR_exception, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	

	/* PIC controller interrupts */
	set_idt_entry(&(entry[32]), ISR_timer, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[33]), ISR_keyboard, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[34]), ISR_slave, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[35]), ISR_serial2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[36]), ISR_serial1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[37]), ISR_parallel2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[38]), ISR_floppy, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[39]), ISR_parallel1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[40]), ISR_rtc, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[41]), ISR_res, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[42]), ISR_nouse1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[43]), ISR_nouse2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[44]), ISR_mouse, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[45]), ISR_cop, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[46]), ISR_hdd1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	set_idt_entry(&(entry[47]), ISR_hdd2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	
	for(i=48; i<IDT_MAX_ENTRY_CNT; i++)
		set_idt_entry(&(entry[i]), ISR_interrupt, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	
	
}
