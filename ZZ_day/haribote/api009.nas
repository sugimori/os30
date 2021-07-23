CPU 486
BITS 32
    GLOBAL  api_alloctimer,api_inittimer, api_settimer, api_freetimer
[SECTION .text]	

api_alloctimer:     ; int api_alloctimer(void);
    MOV     EDX,16
    INT     0x40
    RET

api_inittimer:      ; void api_inittimer(int timer, int data);
    PUSH    EBX
    MOV     EDX,17
    MOV     EBX,[ESP+8]     ; timer
    MOV     EAX,[ESP+12]    ; data
    INT     0x40
    POP     EBX
    RET

api_settimer:       ; void api_settimer(int timer, int time);
    PUSH    EBX
    MOV     EDX,18
    MOV     EBX,[ESP+8]     ; timer
    MOV     EAX,[ESP+12]    ; time
    INT     0x40
    POP     EBX
    RET

api_freetimer:      ; void api_freetimer(int timer);
    PUSH    EBX
    MOV     EDX,19
    MOV     EBX,[ESP+8]     ; timer
    INT     0x40
    POP     EBX
    RET