#include <task.h>
#include <sched.h>
#include <desc.h>
#include <memutils.h>
#include <sync.h>
#include <timer.h>
#include <list.h>

static struct scheduler scheduler;
static struct taskpool_manager taskpool_manager;

/* 
 * [ void init_taskpool( void ) ]
 *
 * All tasks belong to global taskpool 
 * which locate each tasks and manage them
 */
void init_taskpool( void )
{
	int i;
	memset( &(taskpool_manager), 0, sizeof( struct taskpool_manager ) );
	taskpool_manager.start_addr = (struct task*)TASK_POOL_ADDR;
	memset( (void* )TASK_POOL_ADDR, 0, sizeof(struct task)*TASK_MAX_COUNT );

	for( i=0; i<TASK_MAX_COUNT ; i++)
		taskpool_manager.start_addr[i].id = i;
	taskpool_manager.max_count = TASK_MAX_COUNT;
	taskpool_manager.alloc_count = 1;
}

/*
 * [ struct task* alloc_task( void ) ]
 *
 * Allocate a part of taskpool to task to be created
 * lower 32 bits of id stand for the index of taskpool
 * [ alloc_count(idnetifier, 32bits) | index of pool(32bits) ]
 */
struct task* alloc_task( void )
{
	struct task* t;
	int i;

	if( taskpool_manager.use_count == taskpool_manager.max_count )
		return 0;

	for( i=0; i<taskpool_manager.max_count; i++ )
	{
		if( ( taskpool_manager.start_addr[i].id >> 32 ) == 0 )
		{
			t = &(taskpool_manager.start_addr[i] );
			break;
		}
	}

	t->id = ( (unsigned long) taskpool_manager.alloc_count << 32 )| i;
	taskpool_manager.use_count++;
	taskpool_manager.alloc_count++;

	/* alloc_count may reach out 0xFFFFFF..... */
	if( taskpool_manager.alloc_count == 0 )
		taskpool_manager.alloc_count = 1;
	return t;
}

/*
 * [ void free_task( unsigned long id )
 * 
 * Free allocated task
 */
void free_task( unsigned long id )
{
	int i;
	i = TASK_OFFSET(id);
	memset( &(taskpool_manager.start_addr[i].context), 0, sizeof(struct task_context) );
	taskpool_manager.start_addr[i].id = i;
	taskpool_manager.use_count--;
}

/*
 * [ struct task* create_task( unsigned long flags, unsigned long entry ) ]
 *
 * Allocate task from taskpool and seeks for its stack addr
 * stack and taskpool are linearly indexed
 * Also adds created task to readylist of scheduler
 */
struct task* create_task( unsigned long flags, unsigned long entry )
{
	struct task* t;
	void* stack;
	unsigned char prev_flag;

	prev_flag = lock_system();
	t = alloc_task();
	unlock_system( prev_flag );

	if( !t )
		return 0;

	//stack = (void* )(STACK_POOL_ADDR + ( STACK_SIZE * ( t->id & 0xFFFFFFFF ) ) );
	stack = (void *) (STACK_POOL_ADDR + ( STACK_SIZE * TASK_OFFSET( t->id ) ) );
	setup_task( t, flags, entry, stack, STACK_SIZE);

	prev_flag = lock_system();
	add_task_to_readylist( t );
	unlock_system( prev_flag );

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
	memset( task->context.reg, 0, sizeof( struct task_context ) );

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


/*
 * [ void init_scheduler( void ) ]
 *
 * Ready to schedule
 * allocate running task to set current booting task in the taskpool
 */
 
void init_scheduler( void )
{
	int i;

	init_taskpool();

	for( i=0; i<SCHED_MAX_READYLIST; i++ )
	{
		init_list_header( &(scheduler.readylist[i] ) );
		scheduler.exe_count[i] = 0;
	}
	//init_list_header( &(scheduler.readylist) );
	init_list_header( &(scheduler.waitlist) );
	scheduler.running = alloc_task();
	scheduler.running->flags = SCHED_PRIO_HIGHEST;
	scheduler.idle_time = 0;
	scheduler.cpu_load = 0;
	
}

/* Not preferrable for multi core systems */
void set_running_task( struct task* task )
{
	unsigned char prev_flag;

	prev_flag = lock_system();
	scheduler.running = task;
	unlock_system(prev_flag);
}

struct task* get_running_task( void )
{
	unsigned char prev_flag;
	struct task* running;

	prev_flag = lock_system();
	running = scheduler.running;
	unlock_system(prev_flag);
	return running;
}

struct task* get_next_task( void )
{
	struct task* target = 0;
	int task_count, i, j;

	for( j=0; j<2; j++)
	{
		for( i=0; i<SCHED_MAX_READYLIST; i++ )
		{
			task_count = scheduler.readylist[i].items;
			if( scheduler.exe_count[i] < task_count )
			{
				target = list_entry( list_pop( &(scheduler.readylist[i]) ), struct task, link  );
				scheduler.exe_count[i]++;
				break;
			}
			else
				scheduler.exe_count[i] = 0;
		}
		if(target != 0)
			break;
	}
	return target;
//	if( list_empty( &(scheduler.readylist) ) )
//		return NULL;
//	return list_entry( list_pop( &(scheduler.readylist) ), struct task, link );
}

unsigned char add_task_to_readylist( struct task* task )
{
	unsigned char prio;
	prio = SCHED_GET_PRIORITY( task->flags );
	if( prio >= SCHED_MAX_READYLIST )
		return 0;
	list_add_tail( &(scheduler.readylist[prio]), &(task->link)  );
	return 1;
	//list_add_tail( &(scheduler.readylist), &(task->link) );
}

struct task* remove_task_from_readylist( unsigned long id )
{
	struct task* target;
	unsigned char prio;
	struct list* pos;

	if( TASK_OFFSET(id) >= TASK_MAX_COUNT )
		return 0;
	target = &(taskpool_manager.start_addr[ TASK_OFFSET( id ) ] );
	
	if( (target->id & 0xFFFFFFFF) != id )
		return 0;
	
	prio = SCHED_GET_PRIORITY( target->flags );
	pos = &(scheduler.readylist[prio].head);

	do
	{
		target = list_entry( pos, struct task, link );
		if( (target->id&0xFFFFFFFF) == id )
		{
			list_del(pos);
			scheduler.readylist[prio].items--;
			return target;
		}
		pos = pos->next;
	}while( pos != &(scheduler.readylist[prio].head) );
	return 0;
}

unsigned char change_priority( unsigned long id, unsigned char prio )
{
	struct task* target;
	unsigned char prev_flag;

	if( prio > SCHED_MAX_READYLIST )
		return 0;

	prev_flag = lock_system();
	target = scheduler.running;

	if( target->id == id )
		SCHED_SET_PRIORITY(target->flags, prio );
	else
	{
		target = remove_task_from_readylist( id );
		if(target == 0)
		{
			target = get_task_in_pool( TASK_OFFSET(id) );
			if( target != NULL )
				SCHED_SET_PRIORITY( target->flags, prio );
		}
		else
		{
			SCHED_SET_PRIORITY( target->flags, prio );
			add_task_to_readylist( target );
		}
	}
	unlock_system(prev_flag);
	return 1;
}



/* 
 * [ void schedule( void ) ]
 *
 * Simple scheduler... 
 * prevent from interrupted while scheduling
 */
void schedule( void )
{
	struct task* running;
	struct task* next;
	unsigned char prev_flag;
	
	if( get_readytask_count() < 1 )
		return;

	prev_flag = lock_system();
	next = get_next_task();
	if( !next )
	{
		unlock_system(prev_flag);
		return;
	}
	running = scheduler.running;
	scheduler.running = next;
	
	if( (running->flags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
	{
		scheduler.idle_time += SCHED_CPUTIME - scheduler.cputime;
	}

	if( running->flags & TASK_FLAGS_ENDTASK )
	{
		list_add_tail( &(scheduler.waitlist), &(running->link) );
		context_switch( 0, &(next->context) );
	}
	else
	{
		add_task_to_readylist(running);
		context_switch( &(running->context), &(next->context) );
	}
	//add_task_to_readylist( running );
	//context_switch( &(running->context), &(next->context) );
	scheduler.cputime = SCHED_CPUTIME;
	unlock_system(prev_flag);
}

/*
 * [ int schedule_in_interrupt( void ) ]
 *
 * Called by timer_handler when promised CPU time is expired
 * Because context of current running task is stored when interrupt occured
 * We will replace its context with new task to be scheduled
 */
int schedule_in_interrupt( void )
{
	struct task* running;
	struct task* next;
	char* context;
	unsigned char prev_flag;

	prev_flag = lock_system();
	next = get_next_task();
	if( !next )
	{
		unlock_system(prev_flag);
		return -1;
	}
	context = (char* )IST_START_ADDR + IST_SIZE - sizeof( struct task_context );
	running = scheduler.running;
	scheduler.running = next;

	if( (running->flags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
		scheduler.idle_time += SCHED_CPUTIME;

	if( running->flags & TASK_FLAGS_ENDTASK )
		list_add_tail( &(scheduler.waitlist), &(running->link) );
	else
	{
		memcpy( &(running->context), context, sizeof( struct task_context ) );
		add_task_to_readylist( running );
	}
	
	unlock_system(prev_flag);
	memcpy( context, &(next->context), sizeof( struct task_context ) );

	scheduler.cputime = SCHED_CPUTIME;
	return 0;
}

void dec_cputime( void )
{
	if( scheduler.cputime> 0 )
		scheduler.cputime--;
}


unsigned char is_cputime_expired( void )
{
	return (scheduler.cputime<=0)? 1:0;
}


void stop_tasks( const char* buf )
{
	unsigned char prev_flags;
	int i;
	prev_flags = set_interrupt_flag(0);
	for( i=0; i<SCHED_MAX_READYLIST; i++ )
	{
		while( !list_empty(&(scheduler.readylist[i]) ) )
			list_add_tail( &(scheduler.waitlist), list_pop( &(scheduler.readylist[i]) ) );
	}
	set_interrupt_flag(prev_flags);
}

void resume_tasks( const char* buf )
{
	unsigned char prev_flags;
	unsigned char prio;
	struct task* task;
	prev_flags = set_interrupt_flag(0);
	while( !list_empty(&(scheduler.waitlist) ) )
	{
		task = list_entry(list_pop(&scheduler.waitlist), struct task, link );
		prio = SCHED_GET_PRIORITY(task->flags);
		list_add_tail( &(scheduler.readylist[prio]), &(task->link) );
	}
	set_interrupt_flag(prev_flags);
	schedule();
}
	
unsigned char end_task( unsigned long id )
{
	struct task* target;
	unsigned char prio;
	unsigned char prev_flag;

	prev_flag = lock_system();
	target = scheduler.running;
	if( target->id == id )
	{
		target->flags |= TASK_FLAGS_ENDTASK;
		SCHED_SET_PRIORITY( target->flags, SCHED_PRIO_WAIT );
		unlock_system(prev_flag);
		schedule();

		while(1); // never to come here again
	}
	else
	{
		target = remove_task_from_readylist( id );
		if( target == NULL )
		{
			target = get_task_in_pool( TASK_OFFSET( id ) );
			if( target != NULL )
			{
				target->flags |= TASK_FLAGS_ENDTASK;
				SCHED_SET_PRIORITY( target->flags, SCHED_PRIO_WAIT );
			}
			unlock_system(prev_flag);
			return 1;
		}
		target->flags |= TASK_FLAGS_ENDTASK;
		SCHED_SET_PRIORITY( target->flags, SCHED_PRIO_WAIT);
		list_add_tail( &(scheduler.waitlist), &(target->link) );
	}
	return 1;
}

void exit_task(void)
{
	end_task( scheduler.running->id );
}

int get_readytask_count( void )
{
	int count = 0;
	int i;
	unsigned char prev_flag;

	prev_flag = lock_system();

	for( i=0; i<SCHED_MAX_READYLIST ; i++ )
		count += scheduler.readylist[i].items;
	
	unlock_system(prev_flag);
	return count;
}

int get_task_count( void )
{
	int count;
	unsigned char prev_flag;
	count = get_readytask_count();

	prev_flag = lock_system();
	count += scheduler.waitlist.items+1;
	unlock_system(prev_flag);
	return count;
}

struct task* get_task_in_pool( int offset )
{
	if( (offset< -1) && (offset > TASK_MAX_COUNT ) )
		return NULL;

	return &(taskpool_manager.start_addr[offset]);
}

unsigned char task_exist( unsigned long id )
{
	struct task* task;
	task = get_task_in_pool( TASK_OFFSET(id) );

	if( (task == 0) || (task->id != id ) )
		return 0;
	return 1;
}

unsigned long get_cpu_load( void )
{
	return scheduler.cpu_load;
}


void idle_task( void )
{
	struct task* task;
	unsigned long last_tick, last_idle_tick;
	unsigned long current_tick, current_idle_tick;
	unsigned char prev_flag;
	unsigned long tid;

	last_idle_tick = scheduler.idle_time;
	last_tick = tick;

	while(1)
	{
		current_tick = tick;
		current_idle_tick = scheduler.idle_time;

		if( current_tick - last_tick == 0 )
			scheduler.cpu_load = 0;
		else
		{
			scheduler.cpu_load = 100 - (current_idle_tick - last_idle_tick ) * 100 / (current_tick - last_tick );
		}

		last_tick = current_tick;
		last_idle_tick = current_idle_tick;

		halt();

		if( scheduler.waitlist.items >= 0 )
		{
			while(1)
			{
				prev_flag = lock_system();
				if( list_empty( &(scheduler.waitlist) ) )
				{
					unlock_system(prev_flag);
					break;
				}
				task = list_entry( list_pop( &(scheduler.waitlist)), struct task, link);
				tid = task->id;
				free_task( task->id);
				unlock_system(prev_flag);
			}
		}
		schedule();
	}
}

void halt(void)
{
	if( scheduler.cpu_load < 40 )
	{
		asm __volatile__("hlt");
		asm __volatile__("hlt");
		asm __volatile__("hlt");
	}
	else if( scheduler.cpu_load < 80 )
	{
		asm __volatile__("hlt");
		asm __volatile__("hlt");
	}
	else if( scheduler.cpu_load < 95 )
	{
		asm __volatile__("hlt");
	}
}

