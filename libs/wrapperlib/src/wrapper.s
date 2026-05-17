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
CDECL_WRAP str_cmp, strcmp, 2
