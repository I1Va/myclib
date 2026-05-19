#ifndef LANG_PAN_H
#define LANG_PAN_H

#include <stdint.h>

int lang_client_connect(const char*, const char*, const char*);
int lang_client_alive(void);
int lang_client_set_dead(void);
int lang_client_read_next(void);
int lang_client_close(void);

int64_t lang_pan_prefix(void);
int64_t lang_pan_type(void);
int lang_pan_id(void);
int lang_pan_len(void);
int lang_pan_flags(void);

int lang_pan_payload_i32(int offset);
int lang_pan_payload_u32(int offset);
int lang_pan_payload_bool(int offset);

int lang_move(const char* prefix, int dx, int dy);
int lang_attack(const char* prefix, int target);
int lang_random_mod(int max);

#endif
