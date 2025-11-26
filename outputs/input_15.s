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
 movq $0, -16(%rbp)
 movq $0, %rax
 movl %eax, -8(%rbp)
 movq $0, %rax
 movl %eax, -16(%rbp)
.L_while_begin_0:
 movl -8(%rbp), %eax
 pushq %rax
 movq $5, %rax
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
 movl -8(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -16(%rbp)
 movl -8(%rbp), %eax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -8(%rbp)
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
