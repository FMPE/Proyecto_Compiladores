.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl create_point
create_point:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movl -8(%rbp), %eax
 movl %eax, -32(%rbp)
 movl -16(%rbp), %eax
 movl %eax, -28(%rbp)
 movq -32(%rbp), %rax
 movq %rax, -24(%rbp)
 movq -24(%rbp), %rax
 jmp .L_return_create_point
 movq $0, %rax
.L_return_create_point:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $0, %rax
 movq %rax, %rsi
 movq $5, %rax
 movq %rax, %rdi
 call create_point
 movq %rax, -8(%rbp)
 leaq -8(%rbp), %rax
 addq $0, %rax
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
 movl %eax, -16(%rbp)
.L_while_begin_0:
 movl -16(%rbp), %eax
 pushq %rax
 movq $100, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setl %al
 movzbq %al, %rax
 cmpq $0, %rax
 je .L_while_end_1
 movl -16(%rbp), %eax
 pushq %rax
 movq $2, %rax
 movq %rax, %rcx
 popq %rax
 imulq %rcx, %rax
 movl %eax, -16(%rbp)
 jmp .L_while_begin_0
.L_while_end_1:
 movl -16(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
