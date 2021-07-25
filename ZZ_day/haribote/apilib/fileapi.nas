CPU 486
BITS 32
    GLOBAL  api_fopen
    GLOBAL  api_fclose
    GLOBAL  api_fseek
    GLOBAL  api_fsize
    GLOBAL  api_fread
SECTION .text

api_fopen:    ; int api_fopen(char *fname);
    PUSH    EBX
    MOV     EDX,21
    MOV     EBX,[ESP+8]     ; fname
    INT     0x40
    POP     EBX
    RET

api_fclose:   ; void api_flose(int fhandle);
    MOV     EDX,22
    MOV     EAX,[ESP+4]     ; fhandle
    INT     0x40
    RET

api_fseek:    ; void api_fseek(int fhandle, int offset, int mode);
    PUSH    EBX
    MOV     EDX,23
    MOV     EAX,[ESP+8]     ; fhandle
    MOV     ECX,[ESP+16]    ; mode
    MOV     EBX,[ESP+12]    ; offset
    INT     0x40
    POP     EBX
    RET

api_fsize:    ; int api_fsize(int fhandle, int mode);
    MOV     EDX,24
    MOV     EAX,[ESP+4]     ; fhandle
    MOV     ECX,[ESP+8]     ; mode
    INT     0x40
    RET

api_fread:    ; int api_fread(char *buf, int maxsize, int fhandle);
    PUSH    EBX
    MOV     EDX,25
    MOV     EAX,[ESP+16]    ; fhandle
    MOV     ECX,[ESP+12]    ; maxsize
    MOV     EBX,[ESP+8]     ; buf
    INT     0x40
    POP     EBX
    RET