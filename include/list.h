#ifndef __LIST_H__
#define __LIST_H__

#include "types.h"

#define list_entry( ptr, type, member ) \
	container_of( ptr, type, member )

#pragma pack( push, 1 )

struct list
{
	struct list* next;
	struct list* prev;
};

struct list_header
{
	int items;
	struct list head;
};

#pragma pack( pop )

static inline void init_list( struct list* list)
{
	list->next = list->prev = list;
}

static inline void init_list_header(struct list_header* l)
{
	l->items = 0;
	init_list( &(l->head) );	
}

static inline void list_add(struct list* new_item, struct list* prev_item)
{
	prev_item->next->prev = new_item;
	new_item->next = prev_item->next;
	new_item->prev = prev_item;
	prev_item->next = new_item;
}

static inline void list_del(struct list* del)
{
	del->prev->next = del->next;
	del->next->prev = del->prev;
	del->prev = del->next = 0;
}

static inline unsigned char list_empty( struct list_header* l )
{
	return !(l->items);
}

static inline struct list* list_pop( struct list_header* l )
{
	struct list* pop = l->head.next;
	list_del( pop );
	l->items--;
	return pop;
}

static inline void list_add_tail( struct list_header* l, struct list* item )
{
	list_add(item ,(l->head.prev) );
	l->items++;
}

static inline void list_remove(struct list_header* l, struct list* item)
{
	list_del(item);
	l->items--;
}
#endif
