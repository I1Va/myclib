/* main.c - freestanding, uses mini‑libc */

#include "mystring.h"
#include "mystdio.h"
#include "mystdlib.h"
#include "myunistd.h"
#include "myerrno.h"

#include <stdint.h>
#include <stddef.h>

#include "bot.h"
#include "net.h"



int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf_stderr("usage: %s HOST PORT\n", argv[0]);
        return 1;
    }

    // seed the random generator
    srand((unsigned int)(time(0) ^ getpid()));

    int fd = connect_tcp(argv[1], argv[2]);
    if (fd < 0) return 1;

    BotState st;
    bot_state_init(&st, fd);

    uint8_t payload[PAN_MAX_PAYLOAD];

    while (st.alive) {
        PanHeader hdr;
        int rc = read_pan_header(fd, &hdr, IO_TIMEOUT_MS);
        if (rc == 1) continue;                     // timeout, retry
        if (rc == 2) {
            fprintf_stderr("server closed connection\n");
            break;
        }
        if (rc != 0) {
            perror("read header");
            break;
        }

        uint16_t len = hdr.len;

        if (len > 0) {
            rc = read_exact(fd, payload, len, IO_TIMEOUT_MS);
            if (rc == 1) {
                fprintf_stderr("payload timeout\n");
                break;
            }
            if (rc == 2) {
                fprintf_stderr("server closed during payload read\n");
                break;
            }
            if (rc != 0) {
                perror("read payload");
                break;
            }
        }

        if (strcmp(hdr.prefix, "person") == 0)
            handle_person_message(&st, hdr.type, payload, len);
        else if (strcmp(hdr.prefix, "srv") == 0)
            handle_srv_message(&st, hdr.type, payload, len);
    }

    close(fd);
    return 0;
}