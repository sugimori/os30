BITS 32
        GLOBAL      HariMain
HariMain:
        MOV     EDX,123456789
        INT     0x40
        MOV     EDX,4
        INT     0x40
        