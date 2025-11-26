.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 leaq -24(%rbp), %rax
 pushq %rax
 movq $0, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 pushq %rax
 movq $10, %rax
 popq %rdi
 movl %eax, (%rdi)
 leaq -24(%rbp), %rax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 pushq %rax
 movq $20, %rax
 popq %rdi
 movl %eax, (%rdi)
 leaq -24(%rbp), %rax
 pushq %rax
 movq $2, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 pushq %rax
 movq $30, %rax
 popq %rdi
 movl %eax, (%rdi)
 leaq -24(%rbp), %rax
 pushq %rax
 movq $4, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 pushq %rax
 movq $50, %rax
 popq %rdi
 movl %eax, (%rdi)
 leaq -24(%rbp), %rax
 pushq %rax
 movq $0, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 movl (%rax), %eax
 cltq
 pushq %rax
 leaq -24(%rbp), %rax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 pushq %rax
 leaq -24(%rbp), %rax
 pushq %rax
 movq $2, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -32(%rbp)
 movl -32(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 leaq -24(%rbp), %rax
 pushq %rax
 movq $4, %rax
 movq %rax, %rcx
 popq %rax
 leaq (%rax, %rcx, 4), %rax
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
