/* This file is initially written by J. Hyun on 20180119 */

#include <bitmap.h>
#include <memutils.h>

void init_bitmap(struct bitmap *bitmap, int nr_entry, char* map)
{
	unsigned long *pos;
	
	bitmap->nr_entry = nr_entry;
	bitmap->map = map;
	
	memset(map, 0, (nr_entry - 1)/8 + 1);
}

void bitmap_set(struct bitmap* bm, int loc)
{
	char* pos = bm->map;
	char val;

	pos += loc / sizeof(char);
	loc = loc % sizeof(char);
	val = *pos;

	val |= (char)1 << loc;
	*pos = val;
}

void bitmap_clear(struct bitmap* bm, int loc)
{
	char* pos = bm->map;
	char val;

	pos += loc / sizeof(char);
	loc = loc % sizeof(char);
	val = *pos;

	val &= ~((char)1 << loc);
	*pos = val;
}

int bitmap_test_set(struct bitmap* bm, int loc)
{
	char* pos = bm->map;
	char val;

	pos += loc / sizeof(char);
	loc = loc % sizeof(char);
	val = *pos;

	if(!(val & ((char)1 << loc)))
	{
		val |= (char)1 << loc;
		*pos = val;
		return 0;
	}
	else
		return -1;
}

int bitmap_test_clear(struct bitmap* bm, int loc)
{
	char* pos = bm->map;
	char val;

	pos += loc / sizeof(char);
	loc = loc % sizeof(char);
	val = *pos;

	if((val & ((char)1 << loc)))
	{
		val &= ~((char)1 << loc);
		*pos = val;
		return 0;
	}
	else
		return -1;
}
