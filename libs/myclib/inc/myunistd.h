#ifndef _MYUNISTD_H
#define _MYUNISTD_H

#include <stddef.h>

long getpid(void);
long time(long *tloc);
int close(int fd);

#endif