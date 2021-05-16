; naskfunc
; TAB=4

;[FORMAT "WCOFF"]				; オブジェクトファイルを作るモード	
[BITS 32]						; 32ビットモード用の機械語を作らせる
[CPU 486]						; 486め命令を使せ宣言


; オブジェクトファイルのための情報

;[FILE "naskfunc.nas"]			; ソースファイル名情報

		GLOBAL	io_hlt			; このプログラムに含まれる関数名
		GLOBAL	io_cli,io_sti,io_stihlt
		GLOBAL 	io_in8,io_in16,io_in32
		GLOBAL	io_out8,io_out16,io_out32
		GLOBAL	io_load_eflags,io_store_eflags
		GLOBAL	load_gdtr, load_idtr
		GLOBAL	asm_inthandler20, asm_inthandler21, asm_inthandler27, asm_inthandler2c
		GLOBAL	load_cr0, store_cr0
		GLOBAL	load_tr, taskswitch4, taskswitch3, farjmp, farcall
		GLOBAL	asm_cons_putchar
		EXTERN	inthandler20, inthandler21, inthandler2c, inthandler27, cons_putchar


; 以下は実際の関数

[SECTION .text]		; オブジェクトファイルではこれを書いてからプログラムを書く

io_hlt:	; void io_hlt(void);
		HLT
		RET

io_cli:
		CLI
		RET

io_sti:
		STI
		RET

io_stihlt:	
		STI
		HLT
		RET

io_in8:		;int io_in8(int port)
		MOV	EDX,[ESP+4]		; 32bit
		MOV	EAX,0			; 32bit
		IN 	AL,DX			; 下位8bi？？ DX(16bit)のポートから読み込でAL(8bit)にコピー
		RET

io_in16:	;int io_in16(int port);
		MOV EDX,[ESP+4]		; 32bit
		MOV EAX,0			; 32bit
		IN	AX,DX			; 16bit   AX(16bit)
		RET

io_in32: 	; int io_in32(int port);
		MOV	EDX,[ESP+4]
		IN	EAX,DX			; EAX(32bit)
		RET

io_out8:	; void io_out8(int port, int data)
		MOV EDX,[ESP+4]
		MOV	AL,[ESP+8]
		OUT DX,AL			;
		RET

io_out16:	; void io_out8(int port, int data)
		MOV EDX,[ESP+4]
		MOV	EAX,[ESP+8]
		OUT DX,AX
		RET			;

io_out32:	; void io_out8(int port, int data)
		MOV EDX,[ESP+4]
		MOV	EAX,[ESP+8]
		OUT DX,EAX
		RET

io_load_eflags:		; int io_load_eflags(void)
		PUSHFD
		POP		EAX
		RET
io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD
		RET

load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

asm_inthandler20:	;	void asm_inthandler20(void);
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
asm_inthandler21:	;	void asm_inthandler21(void);
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD


asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

load_cr0:		; int load_cr0(void);
	MOV		EAX,CR0
	RET


store_cr0:		; void store_cr0(int cr0);
	MOV		EAX,[ESP+4]
	MOV		CR0,EAX
	RET
	
load_tr:	; void load_tr(int tr);
	LTR		[ESP+4]		; tr
	RET

taskswitch4:	; void taskswitch4(void);
	JMP		4*8:0
	RET
taskswitch3:	; void taskswitch3(void);
	JMP		3*8:0
	RET

farjmp:		; void farjmp(int eip, int cs);
	JMP		FAR [ESP+4]		; eip, cs
	RET

farcall:	; void farcall(int eip, int cs);
	CALL	FAR [ESP+4]		; eip, cs
	RET

asm_cons_putchar:	
	STI
	PUSHAD
	PUSH	1
	AND		EAX,0xff	; AHやEAXの上位を0にして、EAXに文字コードが入った状態にする
	PUSH	EAX
	PUSH	DWORD [0x0fec]	;メモリを読み込んでその値をPUSHする
	CALL	cons_putchar
	ADD		ESP,12		; スタックに積んだデータを捨てる
	POPAD
	IRETD

