.data
print_fmt: .string "%ld \n"
.text
.globl cuadrado
cuadrado:
 pushq %rbp
 movq %rsp, %rbp
 subq $8, %rsp
 movq %rdi,-8(%rbp)
 movq -8(%rbp), %rax
 movq %rax, %rcx
 movq -8(%rbp), %rax
 imulq %rcx, %rax
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
.end_cuadrado:
leave
ret
.globl doble
doble:
 pushq %rbp
 movq %rsp, %rbp
 subq $8, %rsp
 movq %rdi,-8(%rbp)
 movq -8(%rbp), %rax
 movq %rax, %rcx
 movq -8(%rbp), %rax
 addq %rcx, %rax
 jmp .end_doble
.end_doble:
leave
ret
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $0, %rsp
 movq $5, %rax
 movq %rax, 0(%rbp)
 movq $5, %rax
 mov %rax, %rdi
call doble
 movq %rax, %rsi
 leaq print_fmt(%rip), %rdi
 movl $0, %eax
 call printf@PLT
 movq $0, %rax
 jmp .end_main
.end_main:
leave
ret
.section .note.GNU-stack,"",@progbits
