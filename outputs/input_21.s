.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl swap
swap:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq %rdi, -8(%rbp)
 leaq -8(%rbp), %rax
 addq $4, %rax
 movl (%rax), %eax
 cltq
 movl %eax, -16(%rbp)
 leaq -8(%rbp), %rax
 addq $0, %rax
 movl (%rax), %eax
 cltq
 movl %eax, -12(%rbp)
 movq -16(%rbp), %rax
 jmp .L_return_swap
 movq $0, %rax
.L_return_swap:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $24, %rsp
 movq $10, %rax
 movl %eax, -16(%rbp)
 movq $20, %rax
 movl %eax, -12(%rbp)
 movq -16(%rbp), %rax
 movq %rax, -8(%rbp)
 movq -8(%rbp), %rax
 movq %rax, %rdi
 call swap
 movq %rax, -24(%rbp)
 leaq -24(%rbp), %rax
 addq $0, %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
