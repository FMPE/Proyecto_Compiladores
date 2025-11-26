.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl mayor
mayor:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movl -8(%rbp), %eax
 pushq %rax
 movl -16(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setg %al
 movzbq %al, %rax
 cmpq $0, %rax
 je .L_else_0
 movl -8(%rbp), %eax
 jmp .L_return_mayor
 jmp .L_endif_1
.L_else_0:
.L_endif_1:
 movl -16(%rbp), %eax
 jmp .L_return_mayor
 movq $0, %rax
.L_return_mayor:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $0, -8(%rbp)
 movq $3, %rax
 movl %eax, -16(%rbp)
 movq $10, %rax
 movl %eax, -12(%rbp)
 movq -16(%rbp), %rax
 movq %rax, -8(%rbp)
 leaq -8(%rbp), %rax
 addq $4, %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rsi
 leaq -8(%rbp), %rax
 addq $0, %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rdi
 call mayor
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
