#include <sync.h>
#include <interrupt.h>
#include <asmutils.h>
#include <sched.h>
#include <task.h>
void lock(struct mutex* m)
{
	if(test_set(&(m->flag), 0, 1) == 0)
	{
		/* already locked, did i do that? */
		if( m->tid == get_running_task()->id )
		{
			m->count++;
			return;
		}
		/* wait until unlocked */
		while(test_set(&(m->flag), 0, 1) == 0)
		{
			schedule();
		}
	}
	m->count = 1;
	m->tid = get_running_task()->id;
}

void unlock(struct mutex* m)
{
	/* already unlocked? or not my lock? */
	if((m->flag == 0 ) || (m->tid != get_running_task()->id))
		return;

	if( m->count > 1 )
	{
		m->count--;
		return;
	}

	m->tid = TASK_INVALID_ID;
	m->count = 0;
	m->flag = 0;
}

void init_mutex(struct mutex* m)
{
	m->flag = 0;
	m->count = 0;
	m->tid = TASK_INVALID_ID;
}
