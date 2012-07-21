.code
PUBLIC CallFunction
CallFunction PROC
;push rbp
sub rsp,20h
mov r10, qword ptr [rsp+48h] ; fifth parameter (function)
movsd xmm0, qword ptr [rsp+50h] ; sixth parameter (xmm0)
movsd xmm1, qword ptr [rsp+58h] ; seventh parameter (xmm1)
call r10
add rsp,20h
;pop rbp
ret
CallFunction ENDP
End

PUBLIC BeginFrame
BeginFrame PROC
push rbp
BeginFrame ENDP
End