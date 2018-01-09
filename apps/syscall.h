#ifndef __GYSCALL_H__
#define __GYSCALL_H__

int exec(char* filename);
int exit(int status);
int sleep(int sec);
void* malloc(int size);
int open(char* filename);
int read(int fd, char* buf, int size);
int write(int fd, char* buf, int size);

#endif
