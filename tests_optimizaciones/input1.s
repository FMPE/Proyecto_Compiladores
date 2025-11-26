.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $112, %rsp
 movq $5, %rax
 movl %eax, -8(%rbp)
 movq $10, %rax
 movl %eax, -16(%rbp)
 movl -8(%rbp), %eax
 pushq %rax
 movl -16(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -24(%rbp)
 # DAG: reutilizando subexpresion
 movl -24(%rbp), %eax
 movl %eax, -32(%rbp)
 movl -24(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movl -32(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
