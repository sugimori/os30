; naskfunc
; TAB=4

;[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��
[CPU 486]						; 486�ߖ��߂��g���錾


; �I�u�W�F�N�g�t�@�C���̂��߂̏��

;[FILE "naskfunc.nas"]			; �\�[�X�t�@�C�������

		GLOBAL	io_hlt,write_mem8			; ���̃v���O�����Ɋ܂܂��֐���


; �ȉ��͎��ۂ̊֐�

[SECTION .text]		; �I�u�W�F�N�g�t�@�C���ł͂���������Ă���v���O����������

io_hlt:	; void io_hlt(void);
		HLT
		RET

write_mem8:		;void write_mem8(int addr, int data)
		MOV		ECX,[ESP+4]		; �J�E���^���W�X�^
		MOV		AL,[ESP+8]		; �A�L�������[�^���W�X�^
		MOV		[ECX],AL
		RET