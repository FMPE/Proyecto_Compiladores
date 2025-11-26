.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl make_big
make_big:
 pushq %rbp
 movq %rsp, %rbp
 subq $96, %rsp
 movq $1, %rax
 movl %eax, -16(%rbp)
 movq $2, %rax
 movl %eax, -12(%rbp)
 movq $3, %rax
 movl %eax, -8(%rbp)
 leaq -16(%rbp), %rax
 jmp .L_return_make_big
 movq $0, %rax
.L_return_make_big:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $96, %rsp
 call make_big
 movq %rax, %rsi
 leaq -16(%rbp), %rdi
 movq $12, %rcx
 rep movsb
 leaq -16(%rbp), %rax
 addq $8, %rax
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
