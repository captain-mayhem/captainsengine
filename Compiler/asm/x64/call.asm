.data
arr qword 10
.code
PUBLIC CallFunction
CallFunction PROC
push rbp
mov rbp, rsp
push r12
push r13

;sub esp, dword ptr [rbp+48h] ; eighth parameter - before subtraction (stacksize)
movsd xmm0, qword ptr [rbp+38h] ; sixth parameter (xmm0)
movsd xmm1, qword ptr [rbp+40h] ; seventh parameter (xmm1)
mov eax, dword ptr [rbp+48h]
sub eax, 20h
jmp check

loopi:
sub eax, 8
mov r10, qword ptr [rbp+50h]
lea r12, [r10 + rax]
mov r13, [r12] ; now, the array element is in r13
;lea r12, [rsp + rax + 20h]
;mov [r12], r13
push r13

check:
cmp eax, 0h
jg loopi

sub esp, 20h ; shadow space
mov r10, qword ptr [rbp+30h] ; fifth parameter (function)
call r10
add esp,dword ptr [rbp+48h]
pop r13
pop r12
pop rbp
ret
CallFunction ENDP
End
