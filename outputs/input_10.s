.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl suma_p
suma_p:
 pushq %rbp
 movq %rsp, %rbp
 subq $8, %rsp
 movq %rdi, -8(%rbp)
 leaq -8(%rbp), %rax
 movl (%rax), %eax
 cltq
 pushq %rax
 leaq -8(%rbp), %rax
 addq $4, %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 jmp .L_return_suma_p
 movq $0, %rax
.L_return_suma_p:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $0, -8(%rbp)
 movq $10, %rax
 movl %eax, -16(%rbp)
 movq $20, %rax
 movl %eax, -12(%rbp)
 movq -16(%rbp), %rax
 movq %rax, -8(%rbp)
 movq -8(%rbp), %rax
 movq %rax, %rdi
 call suma_p
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
