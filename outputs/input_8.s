.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $1, %rax
 movl %eax, -32(%rbp)
 movq $1234567890123, %rax
 movq %rax, -28(%rbp)
 leaq -32(%rbp), %rax
 movq %rax, %rsi
 leaq -16(%rbp), %rdi
 movq $12, %rcx
 rep movsb
 leaq -16(%rbp), %rax
 addq $0, %rax
 movl (%rax), %eax
 cltq
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 leaq -16(%rbp), %rax
 addq $4, %rax
 movq (%rax), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
