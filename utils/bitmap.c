/* This file is initially written by J. Hyun on 20180119 */

#include <bitmap.h>
#include <memutils.h>

void init_bitmap(struct bitmap *bitmap, int nr_entry, char* map)
{
	bitmap->nr_entry = nr_entry;
	bitmap->map = map;
	
	memset(map, 0, (nr_entry - 1)/BITS_PER_BYTE + 1);
}

void bitmap_set(struct bitmap* bm, int loc, int num)
{
	int from = loc;
	int to = loc + num;
	char* pos = bm->map;
	char val;
	int i;

	pos += from / BITS_PER_BYTE;
	val = *pos;

	while(from < to)
	{
		val = val | (char)1 << (from++ % BITS_PER_BYTE);
		if(!(from % BITS_PER_BYTE))
		{
			*pos++ = val;
			val = *pos;
		}
	}
	*pos = val;
}

void bitmap_clear(struct bitmap* bm, int loc, int num)
{
	int from = loc;
	int to = loc + num;
	char* pos = bm->map;
	char val;
	int i;

	pos += from / BITS_PER_BYTE;
	val = *pos;

	while(from < to)
	{
		val = val & ~((char)1 << (from++ % BITS_PER_BYTE));
		if(!(from % BITS_PER_BYTE))
		{
			*pos++ = val;
			val = *pos;
		}
	}
	*pos = val;
}

int bitmap_test(struct bitmap* bm, int loc, int num, int set)
{
	int from = loc;
	int to = loc + num;
	char* pos = bm->map;
	char val;
	int i;

	pos += from / BITS_PER_BYTE;
	val = *pos;

	while(from < to)
	{
		if(set && !((val >> (from % BITS_PER_BYTE)) % 1))
				return -1;
		if(!set && ((val >> (from % BITS_PER_BYTE)) % 1))
				return -1;

		if(!(++from % BITS_PER_BYTE))
			val = *pos++;
	}
	return 0;
}

int bitmap_find_free(struct bitmap* bm, int num)
{
	char* pos = bm->map;
	int from = 0;
	int to = from + num;

	while(to < bm->nr_entry)
	{
		if(bitmap_test(bm, from, num, BITMAP_CLEAR) < 0)
		{
			from++;
			to++;
		}
		else
		{
			bitmap_set(bm, from, num);
			return from;
		}
	}
	return -1;
}


