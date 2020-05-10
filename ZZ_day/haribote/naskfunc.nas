; naskfunc
; TAB=4

;[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��
[CPU 486]						; 486�ߖ��߂��g���錾


; �I�u�W�F�N�g�t�@�C���̂��߂̏��

;[FILE "naskfunc.nas"]			; �\�[�X�t�@�C�������

		GLOBAL	io_hlt			; ���̃v���O�����Ɋ܂܂��֐���
		GLOBAL	io_cli,io_sti,io_stihlt
		GLOBAL 	io_in8,io_in16,io_in32
		GLOBAL	io_out8,io_out16,io_out32
		GLOBAL	io_load_eflags,io_store_eflags
		GLOBAL	load_gdtr, load_idtr


; �ȉ��͎��ۂ̊֐�

[SECTION .text]		; �I�u�W�F�N�g�t�@�C���ł͂���������Ă���v���O����������

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
		IN 	AL,DX			; ����8bi�H�H DX(16bit)�̃|�[�g����ǂݍ���AL(8bit)�ɃR�s�[
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
