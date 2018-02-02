#ifndef __BITMAP_H__
#define __BITMAP_H__


struct bitmap
{
	int nr_entry;
	char *map;
};

void init_bitmap(struct bitmap* bm, int nr_entry, char* map);
void bitmap_set(struct bitmap* bm, int loc);
void bitmap_clear(struct bitmap* bm, int loc);
int bitmap_test_set(struct bitmap* bm, int loc);
int bitmap_test_clear(struct bitmap* bm, int loc);

#endif
