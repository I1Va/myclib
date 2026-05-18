#ifndef LANG_PAN_H
#define LANG_PAN_H

#include <stdint.h>

int lang_client_connect(const char*, const char*);
int lang_client_alive(void);
int lang_client_set_dead(void);
int lang_client_read_next(void);
int lang_client_close(void);

int64_t lang_pan_prefix(void);
int64_t lang_pan_type(void);
int lang_pan_id(void);
int lang_pan_len(void);
int lang_pan_flags(void);

int64_t lang_pan_const_person(void);
int64_t lang_pan_const_srv(void);

int lang_pan_is_person(void);
int lang_pan_is_srv(void);

int lang_pan_type_is_tick(void);
int lang_pan_type_is_hp(void);
int lang_pan_type_is_at(void);
int lang_pan_type_is_sees(void);
int lang_pan_type_is_wall(void);

int lang_pan_type_is_has_pref(void);
int lang_pan_type_is_name(void);
int lang_pan_type_is_id(void);
int lang_pan_type_is_level(void);
int lang_pan_type_is_r_set_lvl(void);

int lang_pan_payload_i32(int offset);
int lang_pan_payload_u32(int offset);
int lang_pan_payload_bool(int offset);

int lang_person_move(int dx, int dy);
int lang_person_attack(int target);
int lang_random_mod(int max);

#endif
