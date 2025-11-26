.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $96, %rsp
 movq $1, %rax
 movq %rax, -8(%rbp)
 movq $0, %rax
 movq %rax, -16(%rbp)
 movq -8(%rbp), %rax
 testq %rax, %rax
 je .L_else_0
 movq $1, %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 jmp .L_endif_1
.L_else_0:
 movq $0, %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
.L_endif_1:
 movq -16(%rbp), %rax
 testq %rax, %rax
 je .L_else_2
 movq $1, %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 jmp .L_endif_3
.L_else_2:
 movq $0, %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
.L_endif_3:
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
