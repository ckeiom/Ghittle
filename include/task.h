#ifndef __TASK_H__
#define __TASK_H__

#include <list.h>
#include <sync.h>

#define TASK_REGISTERS 24
#define TASK_REGISTER_SIZE 8

#define CONTEXT_OFFSET_GS	0
#define CONTEXT_OFFSET_FS	1
#define CONTEXT_OFFSET_ES	2
#define CONTEXT_OFFSET_DS	3
#define CONTEXT_OFFSET_R15	4
#define CONTEXT_OFFSET_R14	5
#define CONTEXT_OFFSET_R13	6
#define CONTEXT_OFFSET_R12	7
#define CONTEXT_OFFSET_R11	8
#define CONTEXT_OFFSET_R10	9
#define CONTEXT_OFFSET_R9	10
#define CONTEXT_OFFSET_R8	11
#define CONTEXT_OFFSET_RSI	12
#define CONTEXT_OFFSET_RDI	13
#define CONTEXT_OFFSET_RDX	14
#define CONTEXT_OFFSET_RCX	15
#define CONTEXT_OFFSET_RBX	16
#define CONTEXT_OFFSET_RAX	17
#define CONTEXT_OFFSET_RBP	18
#define CONTEXT_OFFSET_RIP	19
#define CONTEXT_OFFSET_CS	20
#define CONTEXT_OFFSET_RFLAGS	21
#define CONTEXT_OFFSET_RSP	22
#define CONTEXT_OFFSET_SS	23

#define TASK_POOL_ADDR	0x800000
#define TASK_MAX	1024

#define STACK_POOL_ADDR (TASK_POOL_ADDR + sizeof(struct task) * TASK_MAX)
#define STACK_SIZE	8192

#define TASK_INVALID_ID	0xFFFFFFFFFFFFFFFF

#define TASK_FLAGS_ENDTASK	0x8000000000000000
#define TASK_FLAGS_IDLE		0x0800000000000000

#define TASK_OFFSET(x)	((x)&0xFFFFFFFF)


#pragma pack(push,1)

struct task_context
{
	unsigned long reg[TASK_REGISTERS];
};

struct task
{
	struct task_context context;
	int id;
	int enabled;
	unsigned long flags;
	
	struct list link;
	void* stack;
	unsigned long stack_size;
};

struct taskpool
{
	void* addr;
	int num_task;
	struct mutex lock;
};

extern struct taskpool taskpool;
#pragma pack(pop)

void init_taskpool(void);
struct task* alloc_task(void);
void free_task(unsigned long id);
struct task* create_task( unsigned long flags, unsigned long entry );

void setup_task( struct task* task, unsigned long flags, unsigned long entry,
				 void* stack, unsigned long stack_size );

void load_gdtr(unsigned long addr);
void load_tss(unsigned short addr);
void load_idtr(unsigned long addr);
unsigned long read_tsc(void);
unsigned long read_rflags(void);

void task_test(void);
#endif
