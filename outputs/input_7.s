.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl res
res:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movq -8(%rbp), %rax
 pushq %rax
 movq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 subq %rcx, %rax
 jmp .L_return_res
 movq $0, %rax
.L_return_res:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $24, %rsp
 movq $30, %rax
 movq %rax, -8(%rbp)
 movq $20, %rax
 movq %rax, -16(%rbp)
 movq -16(%rbp), %rax
 movq %rax, %rsi
 movq -8(%rbp), %rax
 movq %rax, %rdi
 call res
 movq %rax, -24(%rbp)
 movq -24(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
