.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $0, -8(%rbp)
 movq $1, %rax
 pushq %rax
 movq $2, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -16(%rbp)
 movq $10, %rax
 pushq %rax
 movq $3, %rax
 movq %rax, %rcx
 popq %rax
 subq %rcx, %rax
 movl %eax, -12(%rbp)
 movq -16(%rbp), %rax
 movq %rax, -8(%rbp)
 leaq -8(%rbp), %rax
 addq $0, %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 leaq -8(%rbp), %rax
 addq $4, %rax
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
