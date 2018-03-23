#ifndef __BITMAP_H__
#define __BITMAP_H__

#define BITMAP_SET		1
#define BITMAP_CLEAR	0
#define BITS_PER_BYTE	8

struct bitmap
{
	int nr_entry;
	char *map;
};

void init_bitmap(struct bitmap* bm, int nr_entry, char* map);
void bitmap_set(struct bitmap* bm, int loc, int num);
void bitmap_clear(struct bitmap* bm, int loc, int num);
int bitmap_test(struct bitmap* bm, int loc, int num, int set);
int bitmap_find_free(struct bitmap* bm, int num);
#endif
