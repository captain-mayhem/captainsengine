.intel_syntax
.text
	.globl CallFunction
	.type CallFunction, @function
CallFunction:
push %rbp
mov %rbp, %rsp
push %r12
push %r13

movsd %xmm0, qword ptr [%rbp+0x38] /* sixth parameter (xmm0) */
movsd %xmm1, qword ptr [%rbp+0x40] /* seventh parameter (xmm1) */
mov %eax, dword ptr [%rbp+0x48]
sub %eax, 0x20
jmp check

loopi:
sub %eax, 8
mov %r10, qword ptr [%rbp+0x50]
lea %r12, [%r10 + %rax]
mov %r13, [%r12] /* now, the array element is in r13 */
push %r13

check:
cmp %eax, 0
jg loopi

sub %rsp, 0x20 /* shadow space */
mov %r10, qword ptr [%rbp+0x30] /* fifth parameter (function) */
/*call %r10*/
/*add %esp, dword ptr [%rbp+0x48]*/
add %rsp, 0x20
pop %r13
pop %r12
mov %rsp, %rbp
pop %rbp
ret
CallFunction.end:
	.size CallFunction, .-CallFunction
