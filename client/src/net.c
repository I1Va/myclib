/* net.c - freestanding, with handshake and robust syscalls */

#include "net.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/* ---------- Syscall numbers (Linux x86-64) ---------- */
#define SYS_write   1
#define SYS_close   3
#define SYS_socket  41
#define SYS_connect 42
#define SYS_send    44
#define SYS_recv    45
#define SYS_select  23

/* ---------- errno emulation ---------- */
int errno;   // .bss

/* ---------- Minimal string/memory functions ---------- */
static void *mymemcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

static void *mymemset(void *dest, int c, size_t n) {
    unsigned char *d = dest;
    for (size_t i = 0; i < n; i++) d[i] = (unsigned char)c;
    return dest;
}

static size_t mystrlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

/* ---------- Write to stderr ---------- */
static void write_stderr(const char *msg) {
    long ret;
    size_t len = mystrlen(msg);
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_write), "D"(2), "S"(msg), "d"(len)
        : "rcx", "r11", "memory"
    );
}

/* ---------- Minimal fprintf (only %s, %d) ---------- */
static void fprintf_stderr(const char *fmt, ...) {
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

/* ---------- Reliable syscall wrappers ---------- */
static long sys_socket(int domain, int type, int protocol) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_socket), "D"(domain), "S"(type), "d"(protocol)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

static long sys_connect(int fd, const void *addr, size_t addrlen) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_connect), "D"(fd), "S"(addr), "d"(addrlen)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) { errno = -ret; return -1; }
    return 0;
}

/* send syscall: arg4 (flags) must be in r10 */
static long sys_send(int fd, const void *buf, size_t len, int flags) {
    long ret;
    register long r10_arg __asm__("r10") = flags;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_send), "D"(fd), "S"(buf), "d"(len), "r"(r10_arg)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

/* recv syscall: arg4 (flags) in r10 */
static long sys_recv(int fd, void *buf, size_t len, int flags) {
    long ret;
    register long r10_arg __asm__("r10") = flags;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_recv), "D"(fd), "S"(buf), "d"(len), "r"(r10_arg)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

/* select syscall: arg4 (exceptfds) in r10, arg5 (timeout) in r8 */
static long sys_select(int nfds, void *readfds, void *writefds, void *exceptfds, void *timeout) {
    long ret;
    register long r10_arg __asm__("r10") = (long)exceptfds;
    register long r8_arg  __asm__("r8")  = (long)timeout;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_select), "D"(nfds), "S"(readfds), "d"(writefds), "r"(r10_arg), "r"(r8_arg)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

static long sys_close(int fd) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_close), "D"(fd)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) { errno = -ret; return -1; }
    return 0;
}

/* ---------- FD_SET / FD_ZERO ---------- */
typedef struct { unsigned long fds_bits[1024 / (8*sizeof(long))]; } fd_set;
#define FD_ZERO(set) do { for (int i=0; i<16; i++) (set)->fds_bits[i]=0; } while(0)
#define FD_SET(fd, set) ((set)->fds_bits[(fd)/(8*sizeof(long))] |= 1UL << ((fd)%(8*sizeof(long))))

/* ---------- wait_fd ---------- */
int wait_fd(int fd, int want_write, int timeout_ms) {
    for (;;) {
        fd_set rfds, wfds;
        struct { long tv_sec; long tv_usec; } tv;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        if (want_write) FD_SET(fd, &wfds);
        else FD_SET(fd, &rfds);
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        long rc = sys_select(fd+1, want_write ? 0 : &rfds, want_write ? &wfds : 0, 0, &tv);
        if (rc > 0) return 0;
        if (rc == 0) return 1;
        if (errno == 4) continue; // EINTR
        return -1;
    }
}

/* ---------- read_exact / write_exact ---------- */
int read_exact(int fd, void *buf, size_t len, int timeout_ms) {
    uint8_t *p = (uint8_t*)buf;
    size_t off = 0;
    while (off < len) {
        int rc = wait_fd(fd, 0, timeout_ms);
        if (rc != 0) return rc;
        long n = sys_recv(fd, p + off, len - off, 0);
        if (n == 0) return 2;
        if (n < 0) {
            if (errno == 4 || errno == 11 || errno == 11) continue;
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

int write_exact(int fd, const void *buf, size_t len, int timeout_ms) {
    const uint8_t *p = (const uint8_t*)buf;
    size_t off = 0;
    while (off < len) {
        int rc = wait_fd(fd, 1, timeout_ms);
        if (rc != 0) return rc;
        long n = sys_send(fd, p + off, len - off, 0);
        if (n < 0) {
            if (errno == 4 || errno == 11 || errno == 11) continue;
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

/* ---------- Frame sending (same as before) ---------- */
int send_frame(int fd, const char *prefix, const char *type,
               uint32_t msg_id, uint16_t flags,
               const void *payload, uint16_t payload_len) {
    uint8_t header[PAN_HEADER_LEN];
    fill_name8(header + 0, prefix);
    fill_name8(header + 8, type);
    wr_u32_le(header + 16, msg_id);
    wr_u16_le(header + 20, payload_len);
    wr_u16_le(header + 22, flags);
    if (write_exact(fd, header, sizeof(header), IO_TIMEOUT_MS) != 0) return -1;
    if (payload_len > 0 && payload != NULL)
        if (write_exact(fd, payload, payload_len, IO_TIMEOUT_MS) != 0) return -1;
    return 0;
}

/* ---------- High‑level protocol helpers ---------- */
int send_person_move(int fd, int8_t dx, int8_t dy) {
    uint8_t payload[2] = {(uint8_t)dx, (uint8_t)dy};
    return send_frame(fd, "person", "move", 0, 0, payload, sizeof(payload));
}

int send_person_attack(int fd, uint32_t whom) {
    uint8_t payload[4];
    wr_u32_le(payload, whom);
    return send_frame(fd, "person", "attack", 0, 0, payload, sizeof(payload));
}

/* NEW: Send "person:hello" with a name (mandatory handshake) */
int send_person_hello(int fd, const char *name) {
    if (!name) name = "player";
    size_t name_len = mystrlen(name);
    if (name_len > 255) name_len = 255;   // protocol limit? adjust as needed
    uint8_t payload[256];
    uint16_t len = (uint16_t)name_len;
    wr_u16_le(payload, len);
    mymemcpy(payload + 2, name, name_len);
    return send_frame(fd, "person", "hello", 0, 0, payload, len + 2);
}

/* ---------- connect_tcp with automatic hello ---------- */
int connect_tcp(const char *host, const char *port) {
    // Parse port
    int port_num = 0;
    for (const char *p = port; *p; p++) port_num = port_num * 10 + (*p - '0');
    // Build sockaddr_in
    struct sockaddr_in {
        uint16_t sin_family;
        uint16_t sin_port;
        uint32_t sin_addr;
        uint8_t sin_zero[8];
    } addr;
    addr.sin_family = 2; // AF_INET
    addr.sin_port = (port_num >> 8) | ((port_num & 0xFF) << 8); // htons
    // Parse IPv4 address
    uint32_t ip = 0;
    const char *h = host;
    int shift = 24;
    for (int i = 0; i < 4; i++) {
        unsigned long oct = 0;
        while (*h >= '0' && *h <= '9') oct = oct * 10 + (*h++ - '0');
        ip |= (oct << shift);
        shift -= 8;
        if (*h == '.') h++;
    }
    addr.sin_addr = ip;

    int fd = sys_socket(2, 1, 0); // AF_INET, SOCK_STREAM
    if (fd < 0) {
        fprintf_stderr("socket failed: errno %d\n", errno);
        return -1;
    }
    if (sys_connect(fd, &addr, sizeof(addr)) < 0) {
        fprintf_stderr("connect(%s, %s) failed: errno %d\n", host, port, errno);
        sys_close(fd);
        return -1;
    }

    // *** Send the mandatory handshake ***
    if (send_person_hello(fd, "player") < 0) {
        fprintf_stderr("send_person_hello failed, closing\n");
        sys_close(fd);
        return -1;
    }

    return fd;
}

/* ---------- read_pan_header ---------- */
int read_pan_header(int fd, PanHeader *hdr, int timeout_ms) {
    uint8_t raw[PAN_HEADER_LEN];
    int rc = read_exact(fd, raw, sizeof(raw), timeout_ms);
    if (rc != 0) return rc;
    unpack_name8(hdr->prefix, raw + 0);
    unpack_name8(hdr->type, raw + 8);
    hdr->id = rd_u32_le(raw + 16);
    hdr->len = rd_u16_le(raw + 20);
    hdr->flags = rd_u16_le(raw + 22);
    return 0;
}