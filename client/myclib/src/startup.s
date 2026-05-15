.intel_syntax noprefix
.section .text
.global _start

_start:
    # rsp points to:
    #   [rsp]   = argc
    #   [rsp+8] = argv[0]
    #   ...
    mov     rdi, [rsp]          # argc
    lea     rsi, [rsp+8]        # argv
    call    main                # int main(int argc, char **argv)

    # exit syscall
    mov     rdi, rax            # exit code = return value of main
    mov     rax, 60             # sys_exit
    syscall