BITS 32
  GLOBAL    alloca

alloca:
    ADD   EAX, -4
    SUB   ESP,EAX
    JMP   DWORD [ESP+EAX]   ; RETの代わり