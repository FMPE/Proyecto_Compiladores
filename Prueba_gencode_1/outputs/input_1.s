.data
print_fmt: .string "%ld \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $24, %rsp
 movq $1, %rax
 movq %rax, -8(%rbp)
 movq $10, %rax
 movq %rax, -16(%rbp)
 movq $1000000, %rax
 movq %rax, -24(%rbp)
 movq -8(%rbp), %rax
 movq -8(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq -16(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq -24(%rbp), %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
