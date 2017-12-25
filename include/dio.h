#ifndef __DIO_H__
#define __DIO_H__


/* 
 * in_x, out_x
 * 
 * "in" stands for reading from peripheral memory and
 * "out" for writing to.
 * postfix - b, w, l ( 1, 2, 4 bytes I/O )
 */
unsigned char in_b( unsigned short port );
void out_b ( unsigned short port, unsigned char data );

#endif
