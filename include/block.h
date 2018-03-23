#ifndef __BLOCK_H__
#define __BLOCK_H__

int block_read(unsigned int offset, unsigned char *buf);
int block_write(unsigned int offset, unsigned char *buf);

/* block_base in sector number */
unsigned int block_base;

#endif
