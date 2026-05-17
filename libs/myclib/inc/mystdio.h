#ifndef _MYSTDIO_H
#define _MYSTDIO_H

void fprintf_stderr(const char *fmt, ...);
void perror(const char *msg);
void print_num(int num);
int scan_num(void);
const char *scan_str(void);

#endif
