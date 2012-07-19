.code
PUBLIC CallFunction
CallFunction PROC
push rbp
sub rsp,20
call r8
add rsp,20
pop rbp
ret
CallFunction ENDP
End