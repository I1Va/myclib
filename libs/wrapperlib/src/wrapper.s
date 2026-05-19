.intel_syntax noprefix
.text

.macro CALL_WITH_ALIGNED_STACK target
    push rbx
    mov rbx, rsp
    and rsp, -16                   # align for the C call
    call \target
    mov rsp, rbx
    pop rbx
    ret
.endm

.macro CDECL_WRAP name, target, argc
.global \name
.type \name, @function
\name:
    .if \argc >= 1
        mov rdi, QWORD PTR [rsp + 8]
    .endif
    .if \argc >= 2
        mov rsi, QWORD PTR [rsp + 16]
    .endif
    .if \argc >= 3
        mov rdx, QWORD PTR [rsp + 24]
    .endif
    .if \argc >= 4
        mov rcx, QWORD PTR [rsp + 32]
    .endif
    .if \argc >= 5
        mov r8,  QWORD PTR [rsp + 40]
    .endif
    .if \argc >= 6
        mov r9,  QWORD PTR [rsp + 48]
    .endif

    CALL_WITH_ALIGNED_STACK \target
.endm

CDECL_WRAP print_num_cdecl, print_num, 1
CDECL_WRAP scan_num_cdecl, scan_num, 0
CDECL_WRAP scan_str_cdecl, scan_str, 0
CDECL_WRAP strcmp_cdecl, strcmp, 2
CDECL_WRAP smaller_cdecl, smaller, 2
CDECL_WRAP bigger_cdecl, bigger, 2
CDECL_WRAP smaller_or_eq_cdecl, smaller_or_eq, 2
CDECL_WRAP bigger_or_eq_cdecl, bigger_or_eq, 2
CDECL_WRAP equal_cdecl, equal, 2
CDECL_WRAP not_equal_cdecl, not_equal, 2
CDECL_WRAP equal64_cdecl, equal64, 2
CDECL_WRAP not_equal64_cdecl, not_equal64, 2

CDECL_WRAP lang_client_connect_cdecl, lang_client_connect, 3
CDECL_WRAP lang_client_alive_cdecl, lang_client_alive, 0
CDECL_WRAP lang_client_set_dead_cdecl, lang_client_set_dead, 0
CDECL_WRAP lang_client_read_next_cdecl, lang_client_read_next, 0
CDECL_WRAP lang_client_close_cdecl, lang_client_close, 0
CDECL_WRAP lang_pan_prefix_cdecl, lang_pan_prefix, 0
CDECL_WRAP lang_pan_type_cdecl, lang_pan_type, 0
CDECL_WRAP lang_pan_id_cdecl, lang_pan_id, 0
CDECL_WRAP lang_pan_len_cdecl, lang_pan_len, 0
CDECL_WRAP lang_pan_flags_cdecl, lang_pan_flags, 0
CDECL_WRAP lang_pan_payload_i32_cdecl, lang_pan_payload_i32, 1
CDECL_WRAP lang_pan_payload_u32_cdecl, lang_pan_payload_u32, 1
CDECL_WRAP lang_pan_payload_bool_cdecl, lang_pan_payload_bool, 1
CDECL_WRAP lang_move_cdecl, lang_move, 3
CDECL_WRAP lang_attack_cdecl, lang_attack, 2
CDECL_WRAP lang_random_mod_cdecl, lang_random_mod, 1
CDECL_WRAP lang_choose_role_cdecl, lang_choose_role, 1
CDECL_WRAP lang_pan_prefix_is_cdecl, lang_pan_prefix_is, 1
CDECL_WRAP lang_pan_type_is_cdecl, lang_pan_type_is, 1
CDECL_WRAP lang_pan_send_use_cdecl, lang_pan_send_use, 3
