.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl crear
crear:
 pushq %rbp
 movq %rsp, %rbp
 subq $24, %rsp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movl -8(%rbp), %eax
 movl %eax, -24(%rbp)
 movl -16(%rbp), %eax
 movl %eax, -20(%rbp)
 movq -24(%rbp), %rax
 jmp .L_return_crear
 movq $0, %rax
.L_return_crear:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $8, %rsp
 movq $0, -8(%rbp)
 movq $9, %rax
 movq %rax, %rsi
 movq $5, %rax
 movq %rax, %rdi
 call crear
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
