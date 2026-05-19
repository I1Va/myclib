#ifndef NET_H
#define NET_H

#include "protocol.h"

#include <stddef.h>
#include <stdint.h>

int wait_fd(int fd, int want_write, int timeout_ms);
int read_exact(int fd, void *buf, size_t len, int timeout_ms);
int write_exact(int fd, const void *buf, size_t len, int timeout_ms);
int connect_tcp(const char *host, const char *port, const char* prefix);
int send_frame(int fd, const char *prefix, const char *type,
               uint32_t msg_id, uint16_t flags,
               const void *payload, uint16_t payload_len);
int send_move(int fd, int8_t dx, int8_t dy, const char* prefix);
int send_attack(int fd, uint32_t whom, const char* prefix);
int send_role(int fd, const char *role);
int read_pan_header(int fd, PanHeader *hdr, int timeout_ms);
int send_use(int fd, const char* prefix, const char *ability, uint32_t target);

#endif
