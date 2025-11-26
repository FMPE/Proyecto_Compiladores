.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl fibonacci
fibonacci:
 pushq %rbp
 movq %rsp, %rbp
 subq $40, %rsp
 movq %rdi, -8(%rbp)
 movq $0, %rax
 movl %eax, -16(%rbp)
 movq $1, %rax
 movl %eax, -24(%rbp)
 movq $0, %rax
 movl %eax, -32(%rbp)
.L_while_begin_0:
 movl -32(%rbp), %eax
 pushq %rax
 movl -8(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setl %al
 movzbq %al, %rax
 cmpq $0, %rax
 je .L_while_end_1
 movl -16(%rbp), %eax
 pushq %rax
 movl -24(%rbp), %eax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -40(%rbp)
 movl -24(%rbp), %eax
 movl %eax, -16(%rbp)
 movl -40(%rbp), %eax
 movl %eax, -24(%rbp)
 movl -32(%rbp), %eax
 pushq %rax
 movq $1, %rax
 movq %rax, %rcx
 popq %rax
 addq %rcx, %rax
 movl %eax, -32(%rbp)
 jmp .L_while_begin_0
.L_while_end_1:
 movl -16(%rbp), %eax
 jmp .L_return_fibonacci
 movq $0, %rax
.L_return_fibonacci:
 leave
 ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $40, %rsp
 movq $0, %rax
 movl %eax, -32(%rbp)
 movq $10, %rax
 movq %rax, %rdi
 call fibonacci
 movl %eax, -40(%rbp)
 movl -40(%rbp), %eax
 pushq %rax
 movq $50, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setg %al
 movzbq %al, %rax
 cmpq $0, %rax
 je .L_and_false_4
 movl -40(%rbp), %eax
 pushq %rax
 movq $100, %rax
 movq %rax, %rcx
 popq %rax
 cmpq %rcx, %rax
 movq $0, %rax
 setle %al
 movzbq %al, %rax
 cmpq $0, %rax
 je .L_and_false_4
 movq $1, %rax
 jmp .L_and_end_5
.L_and_false_4:
 movq $0, %rax
.L_and_end_5:
 cmpq $0, %rax
 je .L_else_2
 movl -40(%rbp), %eax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 jmp .L_endif_3
.L_else_2:
.L_endif_3:
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
