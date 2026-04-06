# AX=(V-(X*Y+Z-540))/X
# 均为16位有符号整数
code segment
    assume cs:code ds:data
p-name proc far
;
    mov ax,data
    mov ds,ax
;

    MOV AX,X
    IMUL Y      ;两个16位数相乘，此时DX:AX=X*Y
    MOV BX,AX
    MOV CX,DX   ;CX:BX=X*Y

    MOV AX,Z
    CWD          ;将AX中的有符号数扩展到DX:AX中

    ADD BX,AX
    ADC CX,DX     ;CX:BX=X*Y+Z

    SUB BX,540
    SBB CX,0      ;CX:BX=X*Y+Z-540

    MOV AX,V
    CWD
    SUB AX,BX
    SBB DX,CX     ;DX:AX=V-(X*Y+Z-540)

    IDIV X        ;商存入AX=(V-(X*Y+Z-540))/X，余数存入DX

;
exit-dos:
    mov ax,4c00h;
    int 21h
;
p-name endp
code ends
    end p-name
