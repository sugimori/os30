#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void)
{
	for(;;) {
		if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(struct FIFO32 *fifo, int data0)
{
	/* 書き込み先のFIFOバッファを記憶 */
	keyfifo = fifo;
	keydata0 = data0;
	/* キーボードコントローラの初期化 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

void inthandler21(int *esp) 
{
    // http://oswiki.osask.jp/?%28PIC%298259A
    int data;
    io_out8(PIC0_OCW2, 0x61); // IRQ-01 受付完了をPICに通知
    // http://oswiki.osask.jp/?%28AT%29keyboard
    data = io_in8(PORT_KEYDAT);

    fifo32_put(keyfifo, data + keydata0);

    return;
}