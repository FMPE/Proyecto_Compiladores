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
 movl %eax, -8(%rbp)
 movq $0, %rax
 movq %rax, -16(%rbp)
.L_for_begin_0:
 movq $10, %rax
 movq %rax, %rcx
 movq -16(%rbp), %rax
 cmpq %rcx, %rax
 jge .L_for_end_1
 movl -8(%rbp), %eax
 pushq %rax
 movq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -8(%rbp)
 movq -16(%rbp), %rax
 addq $1, %rax
 movq %rax, -16(%rbp)
 jmp .L_for_begin_0
.L_for_end_1:
 movl -8(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
