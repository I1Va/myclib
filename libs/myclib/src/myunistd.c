#include "myunistd.h"

#define SYS_getpid 39
#define SYS_time   201
#define SYS_close  3

long getpid(void) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(SYS_getpid) : "rcx", "r11");
    return ret;
}

long time(long *tloc) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(SYS_time), "D"(tloc) : "rcx", "r11");
    return ret;
}

int close(int fd) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(SYS_close), "D"(fd) : "rcx", "r11");
    if (ret < 0) return -1;
    return 0;
}