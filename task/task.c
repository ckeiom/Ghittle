/* This file is initially written by J. Hyun on 2017.12.21 */

#include <task.h>
#include <memutils.h>
#include <sync.h>
#include <sched.h>
#include <desc.h>

struct taskpool taskpool;

/* 
 * [ void init_taskpool( void ) ]
 *
 * All tasks belong to global taskpool 
 * which locate each tasks and manage them
 */
void init_taskpool( void )
{
	int i;
	memset(&taskpool, 0, sizeof(struct taskpool));
	taskpool.addr = (void *)TASK_POOL_ADDR;
	memset((void* )TASK_POOL_ADDR, 0, sizeof(struct task) * TASK_MAX);

	init_mutex(&taskpool.lock);
	/*
	for( i = 0; i < TASK_MAX; i++)
		taskpool.start_addr[i].id = i;
	taskpool.max_count = TASK_MAX_COUNT;
	*/
}

/*
 * [ struct task* alloc_task( void ) ]
 *
 * Allocate a part of taskpool to task to be created
 * lower 32 bits of id stand for the index of taskpool
 * [ alloc_count(idnetifier, 32bits) | index of pool(32bits) ]
 */
struct task* alloc_task(void)
{
	struct task* t;
	int i;

	if(taskpool.num_task >= TASK_MAX)
		return 0;

	t = (struct task* )taskpool.addr;

	for(i = 0; i < TASK_MAX; i++)
	{
		if(!t->enabled)
			break;
		t++;
	}
	if(i == TASK_MAX)
		return 0;

	t->id = i;
	t->enabled = 1;
	taskpool.num_task++;

	return t;
}

/*
 * [ void free_task(unsigned long id)
 * 
 * Free allocated task
 */
void free_task(unsigned long id)
{
	struct task* t;
	int i;
	
	t = (struct task* )taskpool.addr;

	memset(&t[id], 0, sizeof(struct task));
	taskpool.num_task--;
}

/*
 * [ struct task* create_task(unsigned long flags, unsigned long entry) ]
 *
 * Allocate task from taskpool and seeks for its stack addr
 * stack and taskpool are linearly indexed
 * Also adds created task to readylist of scheduler
 */
struct task* create_task(unsigned long flags, unsigned long entry)
{
	struct task* t;
	void* stack;
	unsigned char prev_flag;

	lock(&taskpool.lock);
	t = alloc_task();
	unlock(&taskpool.lock);

	if(!t)
		return 0;

	stack = (void* )(STACK_POOL_ADDR + (STACK_SIZE * t->id));
	setup_task(t, flags, entry, stack, STACK_SIZE);

	lock(&scheduler.lock);
	add_task_to_readylist(t);
	unlock(&scheduler.lock);

	return t;
}

/*
 * [ void setup_task( ... ) ]
 * 
 * Set registers of task 
 * especially BP, SP, IP and CS, DS... selectors
 */
void setup_task( struct task* task, unsigned long flags, unsigned long entry,
				 void* stack, unsigned long stack_size )
{
	memset(task->context.reg, 0, sizeof(struct task_context));

	task->context.reg[CONTEXT_OFFSET_RSP] = (unsigned long)stack + stack_size;
	task->context.reg[CONTEXT_OFFSET_RBP] = (unsigned long)stack + stack_size;

	task->context.reg[CONTEXT_OFFSET_CS] = GDT_KERNEL_CODE_SEGMENT;
	task->context.reg[CONTEXT_OFFSET_DS] = GDT_KERNEL_DATA_SEGMENT;
	task->context.reg[CONTEXT_OFFSET_ES] = GDT_KERNEL_DATA_SEGMENT;
	task->context.reg[CONTEXT_OFFSET_FS] = GDT_KERNEL_DATA_SEGMENT;
	task->context.reg[CONTEXT_OFFSET_GS] = GDT_KERNEL_DATA_SEGMENT;
	task->context.reg[CONTEXT_OFFSET_SS] = GDT_KERNEL_DATA_SEGMENT;
	
	task->context.reg[CONTEXT_OFFSET_RIP] = entry;

	/* set IF flag on ( make interruptible ) */
	task->context.reg[CONTEXT_OFFSET_RFLAGS] |= 0x200;

	task->stack = stack;
	task->stack_size = stack_size;
	task->flags = flags;
}

