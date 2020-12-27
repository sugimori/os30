#include "bootpack.h"

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct FIFO32 *mousefifo;
int mousedata0;

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	// 書き込み先のFIFOバッファを記憶
	mousefifo = fifo;
	mousedata0 = data0;
	// マウス有効
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return;
}


int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if(mdec->phase == 0) {
		if(dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		if((dat & 0xc8) == 0x08) { // 正しい１バイト目
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07; // 00000111 下位3bit
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];

		if((mdec->buf[0] & 0x10) != 0) { // x方向にマイナス
			mdec->x |= 0xffffff00;
		}
		if((mdec->buf[0] & 0x20) != 0) { // y方向にマイナス
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; // 符号が逆

		return 1;
	}
	return -1;
}


void inthandler2c(int *esp)
/* PS/2マウスからの割り込み */
{
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64); // IRQ-12受付完了 スレーブ
    io_out8(PIC0_OCW2, 0x62); // IRQ-02受付完了 マスター

    data = io_in8(PORT_KEYDAT);
    fifo32_put(mousefifo, data + mousedata0);
    return;
}