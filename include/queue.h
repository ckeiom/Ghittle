#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "types.h"

#pragma pack( push, 1 )

#define QUEUE_LAST_PUT 1
#define QUEUE_LAST_GET 0

struct queue
{
	int data_size;
	int max_count;
	void* array;
	int put;
	int get;
	unsigned char last_op;
};

#pragma pack(pop)

void init_queue( struct queue* q, void* buf, int max_count, int data_size);
unsigned char put_queue( struct queue* q, const void* data );
unsigned char get_queue( struct queue* q, void* data );

static inline int is_queue_full( const struct queue* q )
{
	return ( ( q->get == q->put ) && ( q->last_op == QUEUE_LAST_PUT ) )? 1:0;
}
static inline int is_queue_empty( const struct queue* q )
{
	return ( ( q->get == q->put ) && ( q->last_op == QUEUE_LAST_GET ) )? 1:0;
}


#endif
