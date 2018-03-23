#ifndef __STRING_H__
#define __STRING_H__

#include <stdarg.h>

int strlen(const char* str);
int strcmp(char* a, char* b);
void strcpy(char* dst, char* src);
void reverse_string(char* str);

/* string to int */
int stoi(const char* str);
int itos(long val, char* str, unsigned char hex);

int sprintf(char* buf, const char* fstring, ...);
int vsprintf(char* buf, const char* fstring, va_list ap);

#endif
