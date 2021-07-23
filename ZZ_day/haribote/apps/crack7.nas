BITS 32
    GLOBAL    HariMain

HariMain:
    MOV   AX, 4
    MOV   DS,AX
    CMP   DWORD [DS:0x0004],'Hari'
    JNE   fin

    MOV   ECX,[DS:0x0000]     ; こあアプリのデータセグメントの大きをを読み取る
    MOV   AX,12
    MOV   DS,AX

crackloop:
    ADD   ECX,-1
    MOV   BYTE [DS:ECX],123
    CMP   ECX,0
    JNE   crackloop

fin:
    MOV   EDX,4
    INT   0x40