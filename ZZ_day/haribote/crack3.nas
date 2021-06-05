BITS 32
        GLOBAL      HariMain
HariMain:
        MOV     AL,0x34
        OUT     0x43,AL
        MOV     AL,0xff
        OUT     0x40,AL
        MOV     AL,0xff
        OUT     0x40,AL

        MOV     EDX,4
        INT     0x40