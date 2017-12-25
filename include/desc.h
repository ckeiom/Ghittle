/* GDT dsescriptor bits */

#define GDT_TYPE_CODE	0x0A
#define GDT_TYPE_DATA	0x02
#define GDT_TYPE_TSS	0x09

#define GDT_FLAGS_LOWER_S	0x10
#define GDT_FLAGS_LOWER_DPL0	0x00
#define GDT_FLAGS_LOWER_DPL1	0x20
#define GDT_FLAGS_LOWER_DPL2	0x40
#define GDT_FLAGS_LOWER_DPL3	0x60
#define GDT_FLAGS_LOWER_P		0x80

#define GDT_FLAGS_UPPER_L		0x20
#define GDT_FLAGS_UPPER_DB		0x40
#define GDT_FLAGS_UPPER_G		0x80

/* Pre-defined GDT descriptor options */
#define GDT_FLAGS_LOWER_KERNEL_CODE ( GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | \
							GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_KERNEL_DATA ( GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | \
							GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_TSS ( GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_USER_CODE ( GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | \
							GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_USER_DATA ( GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | \
							GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P )

#define GDT_FLAGS_UPPER_CODE ( GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L )
#define GDT_FLAGS_UPPER_DATA ( GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L )
#define GDT_FLAGS_UPPER_TSS ( GDT_FLAGS_UPPER_G )

/* 
 * [ GDT entry ]
 *
 * |---------------|
 * |  TSS SEGMENT  |  < entry 3
 * |---------------|
 * | DATA SEGMENT  |  < entry 2
 * |---------------| 
 * | CODE SEGMENT  |  < entry 1
 * |---------------|  
 * |     NULL      |  < entry 0
 * |---------------|  < 0x142000
 */

#define GDT_KERNEL_CODE_SEGMENT 0x08  	/* first entry of gdt */
#define GDT_KERNEL_DATA_SEGMENT 0x10	/* second entry of gdt */
#define GDT_TSS_SEGMENT 0x18			/* third entry of gdt */
#define GDTR_START_ADDR 0x142000
#define GDT_MAX_ENTRY8_CNT 3			/* NULL, CODE, DATA segment */
#define GDT_MAX_ENTRY16_CNT 1			/* TSS segment */

#define GDT_TABLE_SIZE ( (sizeof( struct gdt_entry8 )*GDT_MAX_ENTRY8_CNT) + \
						 (sizeof( struct gdt_entry16 )*GDT_MAX_ENTRY16_CNT ) )
#define TSS_SEGMENT_SIZE (sizeof( struct tss_data ) )

#define IDT_TYPE_INTERRUPT	0x0E
#define IDT_TYPE_TRAP		0x0F
#define IDT_FLAGS_DPL0		0x00
#define IDT_FLAGS_DPL1		0x20
#define IDT_FLAGS_DPL2		0x40
#define IDT_FLAGS_DPL3		0x60
#define IDT_FLAGS_P			0x80
#define IDT_FLAGS_IST0		0
#define IDT_FLAGS_IST1		1

#define IDT_FLAGS_KERNEL	(IDT_FLAGS_DPL0|IDT_FLAGS_P)
#define IDT_FLAGS_USER		(IDT_FLAGS_DPL3|IDT_FLAGS_P)

#define IDT_MAX_ENTRY_CNT	100
#define IDTR_START_ADDR		(GDTR_START_ADDR + sizeof(GDTR) + \
							 GDT_TABLE_SIZE + TSS_SEGMENT_SIZE )

#define IDT_START_ADDR		( IDTR_START_ADDR + sizeof(IDTR) )
#define IDT_TABLE_SIZE		(IDT_MAX_ENTRY_CNT * sizeof( struct idt_entry ) )

/*  
 * Interrupt Stack Table
 * When interrupt occurs, set stack to IST address 
 */
#define IST_START_ADDR		0x700000
#define IST_SIZE			0x100000









#pragma pack (push,1)

typedef struct desc_table_reg
{
	unsigned short limit;
	unsigned long addr;
	unsigned short pad1;
	unsigned int pad2;
}GDTR, IDTR;

/* for CODE, DATA segment descriptor */
struct gdt_entry8
{
	unsigned short limit_0_15;
	unsigned short addr_0_15;
	unsigned char addr_16_23;
	unsigned char type_flags_0_3;
	unsigned char limit_16_19_flag_4_7;
	unsigned char addr_24_31;
};

/* for TSS segment descriptor */
struct gdt_entry16
{
	unsigned short limit_0_15;
	unsigned short addr_0_15;
	unsigned char addr_16_23;
	unsigned char type_flags_0_3;
	unsigned char limit_16_19_flag_4_7;
	unsigned char addr_24_31;
	unsigned int addr_32_63;
	unsigned int res;
};

struct tss_data
{
	unsigned int res1;
	unsigned long rsp[3];
	unsigned long res2;
	unsigned long ist[7];
	unsigned long res3;
	unsigned short res4;
	unsigned short iomap_addr;
};

struct idt_entry
{
	unsigned short addr_0_15;
	unsigned short selector;
	unsigned char ist;
	unsigned char type_flags;
	unsigned short addr_16_31;
	unsigned int addr_32_63;
	unsigned int res;
};

#pragma pack ( pop )


void init_GDT_TSS(void);
void init_IDT(void);
void dummy_handler(void);
