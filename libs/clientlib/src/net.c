#include "mystring.h"
#include "mystdio.h"
#include "myerrno.h"

#include <stdint.h>
#include <stddef.h>

#include "net.h"


/* Syscall numbers */
#define SYS_socket  41
#define SYS_connect 42
#define SYS_send    44
#define SYS_recv    45
#define SYS_select  23

/* Syscall wrappers (unchanged except using standard errno) */
static long sys_socket(int domain, int type, int protocol) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(SYS_socket), "D"(domain), "S"(type), "d"(protocol) : "rcx", "r11", "memory");
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

static long sys_connect(int fd, const void *addr, size_t addrlen) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(SYS_connect), "D"(fd), "S"(addr), "d"(addrlen) : "rcx", "r11", "memory");
    if (ret < 0) { errno = -ret; return -1; }
    return 0;
}

static long sys_send(int fd, const void *buf, size_t len, int flags) {
    long ret;
    __asm__ volatile ("movl %k[flags], %%r10d\n\t" "syscall"
        : "=a"(ret) : "a"(SYS_send), "D"(fd), "S"(buf), "d"(len), [flags]"r"(flags)
        : "r10", "rcx", "r11", "memory");
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

static long sys_recv(int fd, void *buf, size_t len, int flags) {
    long ret;
    __asm__ volatile ("movl %k[flags], %%r10d\n\t" "syscall"
        : "=a"(ret) : "a"(SYS_recv), "D"(fd), "S"(buf), "d"(len), [flags]"r"(flags)
        : "r10", "rcx", "r11", "memory");
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

static long sys_select(int nfds, void *readfds, void *writefds, void *exceptfds, void *timeout) {
    long ret;
    register long r10_arg __asm__("r10") = (long)exceptfds;
    register long r8_arg  __asm__("r8")  = (long)timeout;
    __asm__ volatile ("syscall"
        : "=a"(ret) : "a"(SYS_select), "D"(nfds), "S"(readfds), "d"(writefds), "r"(r10_arg), "r"(r8_arg)
        : "rcx", "r11", "memory");
    if (ret < 0) { errno = -ret; return -1; }
    return ret;
}

static long sys_close(int fd) {
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(3), "D"(fd) : "rcx", "r11", "memory");
    if (ret < 0) { errno = -ret; return -1; }
    return 0;
}

/* FD_SET / FD_ZERO (unchanged) */
typedef struct { unsigned long fds_bits[1024 / (8*sizeof(long))]; } fd_set;
#define FD_ZERO(set) do { for (int i=0; i<16; i++) (set)->fds_bits[i]=0; } while(0)
#define FD_SET(fd, set) ((set)->fds_bits[(fd)/(8*sizeof(long))] |= 1UL << ((fd)%(8*sizeof(long))))

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

int read_exact(int fd, void *buf, size_t len, int timeout_ms) {
    uint8_t *p = buf;
    size_t off = 0;
    while (off < len) {
        int rc = wait_fd(fd, 0, timeout_ms);
        if (rc != 0) return rc;
        long n = sys_recv(fd, p + off, len - off, 0);
        if (n == 0) return 2;
        if (n < 0) {
            if (errno == 4 || errno == 11) continue;
            return -1;
        }
        off += n;
    }
    return 0;
}

int write_exact(int fd, const void *buf, size_t len, int timeout_ms) {
    const uint8_t *p = buf;
    size_t off = 0;
    while (off < len) {
        int rc = wait_fd(fd, 1, timeout_ms);
        if (rc != 0) return rc;
        long n = sys_send(fd, p + off, len - off, 0);
        if (n < 0) {
            if (errno == 4 || errno == 11) continue;
            return -1;
        }
        off += n;
    }
    return 0;
}

/* Frame sending – uses fill_name8, wr_u32_le, etc. from protocol.h */
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

int send_move(int fd, int8_t dx, int8_t dy, const char* prefix) {
    uint8_t payload[2] = {(uint8_t)dx, (uint8_t)dy};
    return send_frame(fd, prefix, "move", 0, 0, payload, sizeof(payload));
}

int send_attack(int fd, uint32_t whom, const char* prefix) {
    uint8_t payload[4];
    wr_u32_le(payload, whom);
    return send_frame(fd, prefix, "attack", 0, 0, payload, sizeof(payload));
}

int send_hello(int fd, const char *name, const char* prefix) {
    if (!name) name = "player";
    size_t name_len = strlen(name);
    if (name_len > 255) name_len = 255;
    uint8_t payload[258];                     // 2 bytes length + max 255 name + 1 spare
    uint16_t len = (uint16_t)name_len;
    wr_u16_le(payload, len);                  // write length (2 bytes)
    memcpy(payload + 2, name, name_len);      // copy name (max 255 bytes)
    // total written = 2 + name_len <= 2+255 = 257, fits in 258
    return send_frame(fd, prefix, "hello", 0, 0, payload, len + 2);
}

int connect_tcp(const char *host, const char *port, const char* prefix) {
    int port_num = 0;
    for (const char *p = port; *p; p++) port_num = port_num * 10 + (*p - '0');

    struct sockaddr_in {
        uint16_t sin_family;
        uint16_t sin_port;
        uint32_t sin_addr;
        uint8_t sin_zero[8];
    } addr;
    addr.sin_family = 2; // AF_INET
    addr.sin_port = (port_num >> 8) | ((port_num & 0xFF) << 8);
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

    int fd = sys_socket(2, 1, 0);
    if (fd < 0) {
        fprintf_stderr("socket failed: errno %d\n", errno);
        return -1;
    }
    if (sys_connect(fd, &addr, sizeof(addr)) < 0) {
        fprintf_stderr("connect(%s, %s) failed: errno %d\n", host, port, errno);
        sys_close(fd);
        return -1;
    }
    if (send_hello(fd, "player", prefix) < 0) {
        fprintf_stderr("send_hello failed, closing\n");
        sys_close(fd);
        return -1;
    }
    return fd;
}

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

int send_role(int fd, const char *role) {
    uint32_t len = strlen(role);
    uint8_t payload[32] = {};

    if (len + 3 > sizeof(payload)) {
        return 1;
    }

    wr_u32_le(payload, len);
    memcpy(payload + 2, role, len);
    return send_frame(fd, "role", "choose", 0, 0, payload, sizeof(payload));
}

int send_use(int fd, const char* prefix, const char *ability, uint32_t target) {
    uint32_t len = strlen(ability);
    uint8_t payload[32] = {};

    if (len + 3 > sizeof(payload)) {
        return 1;
    }

    wr_u32_le(payload, len);
    memcpy(payload + 2, ability, len);
    wr_u32_le(payload + 2 + len, target);
    return send_frame(fd, prefix, "use", 0, 0, payload, sizeof(payload));
}

int send_bomb(int fd, const char* prefix) {
    return send_frame(fd, prefix, "bomb", 0, 0, NULL, 0);
}
