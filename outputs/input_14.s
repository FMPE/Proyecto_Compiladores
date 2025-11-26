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
 movq $12, %rax
 movl %eax, -8(%rbp)
 movq $7, %rax
 movl %eax, -16(%rbp)
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
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 jmp .L_endif_1
.L_else_0:
 movl -16(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
.L_endif_1:
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
