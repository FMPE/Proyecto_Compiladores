.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $5, %rax
 movl %eax, -8(%rbp)
 movl -8(%rbp), %eax
 incq %rax
 movl %eax, -8(%rbp)
 movl -8(%rbp), %eax
 incq %rax
 movl %eax, -8(%rbp)
 movl -8(%rbp), %eax
 decq %rax
 movl %eax, -8(%rbp)
 movl -8(%rbp), %eax
 shlq $1, %rax
 movl %eax, -16(%rbp)
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
