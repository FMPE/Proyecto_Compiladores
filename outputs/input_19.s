.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl make_point
make_point:
 pushq %rbp
 movq %rsp, %rbp
 subq $104, %rsp
 movq %rdi, -8(%rbp)
 movq %rsi, -16(%rbp)
 movl -8(%rbp), %eax
 movl %eax, -24(%rbp)
 movl -16(%rbp), %eax
 movl %eax, -20(%rbp)
 movq -24(%rbp), %rax
 jmp .L_return_make_point
 movq $0, %rax
.L_return_make_point:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $88, %rsp
 movq $2, %rax
 movq %rax, %rsi
 movq $1, %rax
 movq %rax, %rdi
 call make_point
 movq %rax, -8(%rbp)
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
