CPU 486
BITS 32
    GLOBAL  api_point
    GLOBAL  api_refreshwin
    GLOBAL  api_linewin
    GLOBAL  api_closewin
    GLOBAL  api_getkey
[SECTION .text]	

api_point:          ; void api_point(int win, int x, int y, int col);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBX
    MOV     EDX,11
    MOV     EBX,[ESP+16]    ; win
    MOV     ESI,[ESP+20]    ; x
    MOV     EDI,[ESP+24]    ; y
    MOV     EAX,[ESP+28]    ; col
    INT     0x40
    POP     EBX
    POP     ESI
    POP     EDI
    RET

api_refreshwin:     ; void api_refreshwin(int win , int x0, int y0, int x1, int y1);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBX
    MOV     EDX,12
    MOV     EBX,[ESP+16]    ; win
    MOV     EAX,[ESP+20]    ; x0
    MOV     ECX,[ESP+24]    ; y0
    MOV     ESI,[ESP+28]    ; x1
    MOV     EDI,[ESP+32]    ; y1
    INT     0x40
    POP     EBX
    POP     ESI
    POP     EDI
    RET

api_linewin:       ; void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBP
    PUSH    EBX
    MOV     EDX,13
    MOV     EBX,[ESP+20]    ; win
    MOV     EAX,[ESP+24]    ; x0
    MOV     ECX,[ESP+28]    ; y0
    MOV     ESI,[ESP+32]    ; x1
    MOV     EDI,[ESP+36]    ; y1
    MOV     EBP,[ESP+40]    ; col
    INT     0x40
    POP     EBX
    POP     EBP
    POP     ESI
    POP     EDI
    RET

api_closewin:       ; void api_closewin(int win);
    PUSH    EBX
    MOV     EDX,14
    MOV     EBX,[ESP+8]     ; win
    INT     0x40
    POP     EBX
    RET

api_getkey:         ; int api_getkey(int mode);
    MOV     EDX,15
    MOV     EAX,[ESP+4]     ; mode
    INT     0x40
    RET