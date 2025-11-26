.data
print_fmt: .string "%ld \n"
print_float_fmt: .string "%f \n"
.text
.globl main
main:
 pushq %rbp
 movq %rsp, %rbp
 subq $32, %rsp
 movq $0, -8(%rbp)
 movq $0, -16(%rbp)
 movabsq $4614253070214989087, %rax
 movq %rax, %xmm0
 movq %rax, %xmm0
 cvtsd2ss %xmm0, %xmm0
 movd %xmm0, %eax
 movl %eax, -8(%rbp)
 movabsq $4613303441197561744, %rax
 movq %rax, %xmm0
 movq %rax, -16(%rbp)
 movl -8(%rbp), %eax
 pushq %rax
 movabsq $4609434218613702656, %rax
 movq %rax, %xmm0
 movq %rax, %rcx
 popq %rax
 movq %rax, %xmm0
 movq %rcx, %xmm1
 cvtss2sd %xmm0, %xmm0
 addsd %xmm1, %xmm0
 movq %xmm0, %rax
 movq %rax, %xmm0
 cvtsd2ss %xmm0, %xmm0
 movd %xmm0, %eax
 movl %eax, -24(%rbp)
 movq -16(%rbp), %rax
 pushq %rax
 movabsq $4611686018427387904, %rax
 movq %rax, %xmm0
 movq %rax, %rcx
 popq %rax
 movq %rax, %xmm0
 movq %rcx, %xmm1
 mulsd %xmm1, %xmm0
 movq %xmm0, %rax
 movq %rax, -32(%rbp)
 movl -24(%rbp), %eax
 movq %rax, %xmm0
 cvtss2sd %xmm0, %xmm0
 leaq print_float_fmt(%rip), %rdi
 movl $1, %eax
 call printf@PLT
 movq -32(%rbp), %rax
 movq %rax, %xmm0
 leaq print_float_fmt(%rip), %rdi
 movl $1, %eax
 call printf@PLT
 movq $0, %rax
.L_return_main:
 leave
 ret
.section .note.GNU-stack,"",@progbits
