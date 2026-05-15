#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

long getpid(void);
long time(long *tloc);
int close(int fd);

#endif