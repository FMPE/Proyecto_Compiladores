.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl suma
suma:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movq -8(%rbp), %rax
 pushq %rax
 movq -16(%rbp), %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 jmp .L_return_suma
 movq $0, %rax
.L_return_suma:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $16, %rsp
 movq $0, -8(%rbp)
 movq $0, -16(%rbp)
 movq $10000000000, %rax
 movq %rax, -8(%rbp)
 movq $20000000000, %rax
 movq %rax, -16(%rbp)
 movq -16(%rbp), %rax
 movq %rax, %rsi
 movq -8(%rbp), %rax
 movq %rax, %rdi
 call suma
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
