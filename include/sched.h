#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <task.h>

#define SCHED_CPUTIME	5 /* ms */
#define SCHED_MAX_READYLIST	5

#define SCHED_PRIO_HIGHEST 	0
#define SCHED_PRIO_HIGH		1
#define SCHED_PRIO_MEDIUM	2
#define SCHED_PRIO_LOW		3
#define SCHED_PRIO_LOWEST	4
#define SCHED_PRIO_WAIT		0xFF

#define SCHED_GET_PRIORITY(x)		( (x)&0xFF)
#define SCHED_SET_PRIORITY(x, prio) 	( (x) = ( (x)&0xFFFFFFFFFFFFFF00 ) | (prio) )

#pragma pack(push,1)

struct scheduler
{
	struct task* running;
	int cputime;
	struct list_header readylist[SCHED_MAX_READYLIST];
	struct list_header waitlist;
	int exe_count[SCHED_MAX_READYLIST];
	unsigned long cpu_load;
	unsigned long idle_time;
};

#pragma pack(pop)

void init_scheduler(void);

void set_running_task( struct task* task );
struct task* get_running_task( void);
struct task* get_next_task(void);

unsigned char add_task_to_readylist( struct task* task );
struct task* remove_task_from_readylist( unsigned long id );

void schedule(void);
int schedule_in_interrupt(void);

void dec_cputime(void);
unsigned char is_cputime_expired(void);

unsigned char change_priority( unsigned long id, unsigned char prio );
unsigned char end_task( unsigned long id );
void exit_task( void );
int get_readytask_count( void );
unsigned long get_cpu_load( void );

void context_switch(struct task_context *prev, struct task_context *next);

#endif
