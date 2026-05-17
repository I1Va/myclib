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

#define SYS_write 1
#define SYS_read 0
#define SYS_exit 60
void print_num(int num) {
    char buf[16] = {0};
    char *rsi = buf + 15;
    int is_negative = 0;

    if (num < 0) {
        num = -num;
        is_negative = 1;
    }

    do {
        rsi--;
        *rsi = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    if (is_negative) {
        rsi--;
        *rsi = '-';
    }

    char *rdi = rsi;
    while (*rdi != '\0') {
        rdi++;
    }
    *rdi = '\n';

    long rdx = (rdi + 1) - rsi;

    long syscall_ret;
    __asm__ __volatile__(
        "syscall"
        : "=a"(syscall_ret)
        : "a"(SYS_write), "D"(1), "S"(rsi), "d"(rdx)
        : "rcx", "r11", "memory"
    );
}

int scan_num(void) {
    char buf[16] = {0};
    long bytes_read;

    __asm__ __volatile__(
        "syscall"
        : "=a"(bytes_read)
        : "a"(SYS_read), "D"(0), "S"(buf), "d"(16)
        : "rcx", "r11", "memory"
    );

    if (bytes_read <= 0) {
        __asm__ __volatile__(
            "syscall"
            :
            : "a"(SYS_exit), "D"(0)
        );
    }

    int result = 0;
    int is_negative = 0;
    int rcx = 0;

    if (buf[rcx] == '-') {
        rcx++;
        is_negative = 1;
    }

    while (1) {
        unsigned char dl = (unsigned char)buf[rcx];
        if (dl < '0' || dl > '9') {
            break;
        }

        result = (result * 10) + (dl - '0');
        rcx++;
    }

    if (is_negative) {
        result = -result;
    }

    return result;
}

const char *scan_str(void) {
    static char buf[256] = {0};
    long bytes_read;

    __asm__ __volatile__(
        "syscall"
        : "=a"(bytes_read)
        : "a"(SYS_read), "D"(0), "S"(buf), "d"(sizeof(buf) - 1)
        : "rcx", "r11", "memory"
    );

    if (bytes_read <= 0) {
        buf[0] = '\0';
        return buf;
    }

    if (bytes_read >= (long) sizeof(buf)) {
        bytes_read = sizeof(buf) - 1;
    }

    buf[bytes_read] = '\0';
    for (long i = 0; i < bytes_read; i++) {
        if (buf[i] == '\n' || buf[i] == '\r') {
            buf[i] = '\0';
            break;
        }
    }

    return buf;
}
