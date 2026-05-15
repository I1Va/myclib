.intel_syntax noprefix
.text

.global print
.type print, @function
print:
    mov edi, DWORD PTR [rsp + 8]   # read argument before touching rsp
    push rbx
    mov rbx, rsp
    and rsp, -16                   # align for the C call
    call print_V_amd64
    mov rsp, rbx
    pop rbx
    ret

.global scan
.type scan, @function
scan:
    push rbx
    mov rbx, rsp
    and rsp, -16
    call scan_V_amd64
    mov rsp, rbx
    pop rbx
    ret
