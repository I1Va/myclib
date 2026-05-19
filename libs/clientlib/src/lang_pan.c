#include "lang_pan.h"

#include "net.h"
#include "protocol.h"

#include "myerrno.h"
#include "mystdio.h"
#include "mystdlib.h"
#include "mystring.h"
#include "myunistd.h"

#include <stdint.h>

static int g_fd = -1;
static int g_alive = 0;
static PanHeader g_hdr;
static uint8_t g_payload[PAN_MAX_PAYLOAD];
static int g_has_message = 0;

static int64_t pack_name8(const char* name)
{
    uint64_t packed = 0;

    for (int i = 0; i < PAN_NAME_LEN && name[i] != '\0'; i++) {
        packed |= ((uint64_t)(uint8_t) name[i]) << (8 * i);
    }

    return (int64_t) packed;
}

int lang_client_connect(const char* ip, const char* port, const char* prefix)
{
    srand((unsigned int)(time(0) ^ getpid()));

    g_fd = connect_tcp(ip, port, prefix);
    if (g_fd < 0) {
        return 1;
    }

    g_alive = 1;
    g_has_message = 0;

    return 0;
}

int lang_client_alive(void)
{
    return g_alive;
}

int lang_client_set_dead(void)
{
    g_alive = 0;
    return 0;
}

int lang_client_read_next(void)
{
    if (!g_alive || g_fd < 0) {
        return 2;
    }

    int rc = read_pan_header(g_fd, &g_hdr, IO_TIMEOUT_MS);
    if (rc == 1) {
        g_has_message = 0;
        return 1;
    }
    if (rc == 2) {
        fprintf_stderr("server closed connection\n");
        g_alive = 0;
        g_has_message = 0;
        return 2;
    }
    if (rc != 0) {
        perror("read header");
        g_alive = 0;
        g_has_message = 0;
        return -1;
    }

    uint16_t len = g_hdr.len;
    if (len > 0) {
        rc = read_exact(g_fd, g_payload, len, IO_TIMEOUT_MS);
        if (rc == 1) {
            fprintf_stderr("payload timeout\n");
            g_alive = 0;
            g_has_message = 0;
            return 1;
        }
        if (rc == 2) {
            fprintf_stderr("server closed during payload read\n");
            g_alive = 0;
            g_has_message = 0;
            return 2;
        }
        if (rc != 0) {
            perror("read payload");
            g_alive = 0;
            g_has_message = 0;
            return -1;
        }
    }

    g_has_message = 1;
    return 0;
}

int lang_client_close(void)
{
    int rc = 0;

    if (g_fd >= 0) {
        rc = close(g_fd);
        g_fd = -1;
    }

    g_alive = 0;
    g_has_message = 0;

    return rc;
}

int64_t lang_pan_prefix(void)
{
    return g_has_message ? pack_name8(g_hdr.prefix) : 0;
}

int64_t lang_pan_type(void)
{
    return g_has_message ? pack_name8(g_hdr.type) : 0;
}

int lang_pan_id(void)
{
    return g_has_message ? (int) g_hdr.id : 0;
}

int lang_pan_len(void)
{
    return g_has_message ? (int) g_hdr.len : 0;
}

int lang_pan_flags(void)
{
    return g_has_message ? (int) g_hdr.flags : 0;
}

int lang_pan_payload_i32(int offset)
{
    if (!g_has_message || offset < 0 || offset + 4 > g_hdr.len) {
        return 0;
    }

    return rd_i32_le(g_payload + offset);
}

int lang_pan_payload_u32(int offset)
{
    if (!g_has_message || offset < 0 || offset + 4 > g_hdr.len) {
        return 0;
    }

    return (int) rd_u32_le(g_payload + offset);
}

int lang_pan_payload_bool(int offset)
{
    if (!g_has_message || offset < 0 || offset + 1 > g_hdr.len) {
        return 0;
    }

    return rd_bool_u8(g_payload + offset);
}

int lang_move(const char* prefix, int dx, int dy)
{
    if (g_fd < 0) {
        g_alive = 0;
        return -1;
    }

    if (send_move(g_fd, (int8_t) dx, (int8_t) dy, prefix) != 0) {
        fprintf_stderr("send move failed\n");
        g_alive = 0;
        return -1;
    }

    fprintf_stderr("move %d %d\n", dx, dy);
    return 0;
}

int lang_attack(const char* prefix, int target)
{
    if (g_fd < 0) {
        g_alive = 0;
        return -1;
    }

    if (send_attack(g_fd, (uint32_t) target, prefix) != 0) {
        fprintf_stderr("send attack failed\n");
        g_alive = 0;
        return -1;
    }

    fprintf_stderr("attack %d\n", target);
    return 0;
}

int lang_random_mod(int max)
{
    if (max <= 0) {
        return 0;
    }

    return rand() % max;
}

int lang_pan_prefix_is(const char* prefix)
{
    return g_has_message && strcmp(g_hdr.prefix, prefix) == 0;
}

int lang_pan_type_is(const char* type)
{
    return g_has_message && strcmp(g_hdr.type, type) == 0;
}

int lang_choose_role(const char* role)
{
    if (send_role(g_fd, role) != 0) {
        fprintf_stderr("send role is failed\n");
        g_alive = 0;
        return -1;
    }

    fprintf_stderr("send role %s\n", role);
	return 0;
}

int lang_pan_send_use(const char* prefix, const char* ability, uint32_t target)
{
    if (send_use(g_fd, prefix, ability, target) != 0) {
        fprintf_stderr("send use is failed\n");
        g_alive = 0;
        return -1;
    }

    fprintf_stderr("send use %s\n", ability);
    return 0;
}

int lang_pan_send_bomb(const char* prefix)
{
    if (send_bomb(g_fd, prefix) != 0) {
        fprintf_stderr("send bomb is failed\n");
        g_alive = 0;
        return -1;
    }

    fprintf_stderr("send bomb\n");
    return 0;
}
