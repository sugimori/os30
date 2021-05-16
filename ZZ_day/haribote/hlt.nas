BITS 32
    MOV     AL,'A'
    CALL    0xc52
fin:
    HLT
    JMP fin