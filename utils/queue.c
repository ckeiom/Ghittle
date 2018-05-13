#include <queue.h>
#include <memutils.h>

void init_queue(struct queue* q, void * buf, int max_count, int data_size)
{
	q->max_count = max_count;
	q->data_size = data_size;
	q->array = buf;
	q->put = q->get = 0;
	q->last_op = QUEUE_LAST_GET;
}

unsigned char put_queue(struct queue* q, const void* data)
{
	if (is_queue_full(q))
		return 0;
	memcpy((char*)q->array + (q->data_size*q->put), data, q->data_size);
	q->put = (q->put + 1)%q->max_count;
	q->last_op = QUEUE_LAST_PUT;
	return 1;
}

unsigned char get_queue(struct queue* q, void* data)
{
	if(is_queue_empty(q))
		return 0;

	memcpy(data, (char* )q->array + (q->data_size*q->get), q->data_size);
	q->get = (q->get + 1) % q->max_count;
	q->last_op = QUEUE_LAST_GET;
	return 1;
}
