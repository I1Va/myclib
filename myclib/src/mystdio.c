#include "mystdio.h"
#include "mystring.h"
#include <stdarg.h>

#define SYS_write 1

static void write_stderr(const char *msg) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_write), "D"(2), "S"(msg), "d"(strlen(msg))
        : "rcx", "r11", "memory"
    );
}

void fprintf_stderr(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[256];
    char *p = buf;
    for (const char *f = fmt; *f; f++) {
        if (*f == '%') {
            f++;
            if (*f == 's') {
                const char *s = va_arg(args, const char*);
                while (*s) *p++ = *s++;
            } else if (*f == 'd') {
                int d = va_arg(args, int);
                char num[16];
                char *np = num;
                int neg = 0;
                if (d < 0) { neg = 1; d = -d; }
                do { *np++ = '0' + (d % 10); d /= 10; } while (d);
                if (neg) *np++ = '-';
                while (np > num) *p++ = *--np;
            } else if (*f == 'u') {
                unsigned int u = va_arg(args, unsigned int);
                char num[16];
                char *np = num;
                do { *np++ = '0' + (u % 10); u /= 10; } while (u);
                while (np > num) *p++ = *--np;
            } else if (*f == '%') {
                *p++ = '%';
            } else {
                *p++ = '%';
                *p++ = *f;
            }
        } else {
            *p++ = *f;
        }
    }
    *p = '\0';
    write_stderr(buf);
    va_end(args);
}

void perror(const char *msg) {
    extern int errno;
    fprintf_stderr("%s: errno %d\n", msg ? msg : "error", errno);
}