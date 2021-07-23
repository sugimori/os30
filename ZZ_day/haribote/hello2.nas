BITS 32
        GLOBAL      HariMain

HariMain:
    MOV     EDX,2
    MOV     EBX,msg
    INT     0x40
    MOV     EDX,4
    INT     0x40

section  .data
msg:    
    DB      "Hello2",0