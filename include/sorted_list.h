#ifndef __SORTED_LIST_H__
#define __SORTED_LIST_H__

#include "list.h"
#include <stdio.h>

#pragma pack(push, 1)

struct sorted_list
{
	long key;
	struct sorted_list* prev;
	struct sorted_list* next;
};

struct sorted_list_header
{
	int items;
	struct sorted_list head;
};

#pragma pack( pop )

static inline void init_sorted_list( struct sorted_list* list )
{
	list->next = list->prev = list;
}

static inline void init_sorted_list_header( struct sorted_list_header* l )
{
	l->items = 0;
	init_sorted_list( &(l->head) );	
}

static inline void sorted_list_add_after( struct sorted_list* new_item, struct sorted_list* prev_item )
{
	prev_item->next->prev = new_item;
	new_item->next = prev_item->next;
	new_item->prev = prev_item;
	prev_item->next = new_item;
}

static inline void sorted_list_del( struct sorted_list* del )
{
	del->prev->next = del->next;
	del->next->prev = del->prev;
	del->prev = del->next = 0;
}

static inline unsigned char sorted_list_empty( struct sorted_list_header* l )
{
	return !(l->items);
}

static inline struct sorted_list* sorted_list_pop( struct sorted_list_header* l )
{
	if( l->items )
	{		
		struct sorted_list* pop = l->head.next;
		sorted_list_del( pop );
		l->items--;
		return pop;
	}
	else
	{
		return 0;
	}
}

static inline void sorted_list_add( struct sorted_list_header* l, struct sorted_list* new_item, long key )
{
	struct sorted_list* head = &l->head;
	struct sorted_list* next_item = head->next;

	while( next_item != head )
	{
		if ( next_item->key < key )
		{
			next_item = next_item->next;
		}
		else
		{
			break;
		}
	}

	sorted_list_add_after( new_item, next_item->prev );

	new_item->key = key;
	l->items++;
}

#endif
