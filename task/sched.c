/* This file is initially written by J. Hyun on 2017.12.21 */

#include <sched.h>
#include <timer.h>
#include <memutils.h>
#include <desc.h>
#include <sync.h>

struct scheduler scheduler;

/*
 * [ void init_scheduler( void ) ]
 *
 * Ready to schedule
 * allocate running task to set current booting task in the taskpool
 */
 
void init_scheduler(void)
{
	init_taskpool();
	init_mutex(&scheduler.lock);
	init_list_header(&scheduler.readylist);
	init_list_header(&scheduler.waitlist);
	
	scheduler.running = alloc_task();
	scheduler.num_running = 0;
	scheduler.idle_time = 0;
	scheduler.cpu_load = 0;
}

/* Not preferrable for multi core systems */
void set_running_task(struct task* task)
{
	lock(&scheduler.lock);
	scheduler.running = task;
	unlock(&scheduler.lock);
}

struct task* get_running_task(void)
{
	struct task* running;
	running = scheduler.running;
	return running;
}

struct task* get_next_task(void)
{
	struct task* next = 0;
	int task_count, i, j;

	if(list_empty(&scheduler.readylist))
		return scheduler.running;

	lock(&scheduler.lock);
	next = list_entry(list_pop(&scheduler.readylist), struct task, link);
	unlock(&scheduler.lock);
	
	return next;
}

unsigned char add_task_to_readylist(struct task* task)
{
	lock(&scheduler.lock);
	list_add_tail(&scheduler.readylist, &(task->link));
	unlock(&scheduler.lock);
	return 1;
}

struct task* remove_task_from_readylist(unsigned long id)
{
	struct task* target;
	struct list* pos;

	if(id >= TASK_MAX)
		return 0;

	target = (struct task* )taskpool.addr + id;
	lock(&scheduler.lock);
	list_remove(&scheduler.readylist, &target->link);
	unlock(&scheduler.lock);
	return target;
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
	
	prev_flag = lock_system();
	next = get_next_task();
	
	if( !next )
	{
		unlock_system(prev_flag);
		return;
	}

	running = scheduler.running;
	scheduler.running = next;
	
	if((running->flags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
		scheduler.idle_time += SCHED_CPUTIME - scheduler.cputime;

	if( running->flags & TASK_FLAGS_ENDTASK )
	{
		list_add_tail(&scheduler.waitlist, &running->link);
		context_switch( 0, &(next->context) );
	}
	else
	{
		add_task_to_readylist(running);
		context_switch(&running->context, &next->context);
	}
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
int schedule_in_interrupt(void)
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
	context = (char* )IST_START_ADDR + IST_SIZE - sizeof(struct task_context);
	running = scheduler.running;
	scheduler.running = next;

	if( (running->flags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
		scheduler.idle_time += SCHED_CPUTIME;

	if( running->flags & TASK_FLAGS_ENDTASK )
		list_add_tail(&scheduler.waitlist, &running->link);
	else
	{
		memcpy(&running->context, context, sizeof(struct task_context));
		add_task_to_readylist(running);
	}
	
	unlock_system(prev_flag);
	memcpy(context, &(next->context), sizeof(struct task_context));

	scheduler.cputime = SCHED_CPUTIME;
	return 0;
}

void dec_cputime(void)
{
	if( scheduler.cputime > 0)
		scheduler.cputime--;
}


unsigned char is_cputime_expired(void)
{
	return (scheduler.cputime<=0)? 1:0;
}


void stop_tasks(const char* buf)
{
	unsigned char prev_flags;
	int i;
	prev_flags = set_interrupt_flag(0);
	if(!list_empty(&scheduler.readylist))
		list_add_tail(&scheduler.waitlist, list_pop(&scheduler.readylist));
	set_interrupt_flag(prev_flags);
}

void resume_tasks(const char* buf)
{
	unsigned char prev_flags;
	unsigned char prio;
	struct task* task;
	prev_flags = set_interrupt_flag(0);
	while(!list_empty(&scheduler.waitlist))
	{
		task = list_entry(list_pop(&scheduler.waitlist), struct task, link);
		list_add_tail(&scheduler.readylist, &task->link);
	}
	set_interrupt_flag(prev_flags);
	schedule();
}
	
unsigned char end_task(unsigned long id)
{
	struct task* target;
	unsigned char prio;
	unsigned char prev_flag;

	prev_flag = lock_system();
	target = scheduler.running;
	if(target->id == id)
	{
		target->flags |= TASK_FLAGS_ENDTASK;
		unlock_system(prev_flag);
		schedule();

		while(1); // never to come here again
	}
	else
	{
		target = remove_task_from_readylist(id);
		target->flags |= TASK_FLAGS_ENDTASK;
		list_add_tail(&scheduler.waitlist, &target->link);
	}
	return 1;
}

void exit_task(void)
{
	end_task(scheduler.running->id);
}


unsigned long get_cpu_load(void)
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

		if(scheduler.waitlist.items >= 0)
		{
			while(1)
			{
				prev_flag = lock_system();
				if( list_empty(&scheduler.waitlist))
				{
					unlock_system(prev_flag);
					break;
				}
				task = list_entry(list_pop(&scheduler.waitlist), struct task, link);
				tid = task->id;
				free_task(task->id);
				unlock_system(prev_flag);
			}
		}
		schedule();
	}
}

void halt(void)
{
	if(scheduler.cpu_load < 40)
	{
		asm __volatile__("hlt");
		asm __volatile__("hlt");
		asm __volatile__("hlt");
	}
	else if(scheduler.cpu_load < 80)
	{
		asm __volatile__("hlt");
		asm __volatile__("hlt");
	}
	else if(scheduler.cpu_load < 95)
	{
		asm __volatile__("hlt");
	}
}

