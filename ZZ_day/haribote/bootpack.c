#include "bootpack.h"

extern struct KEYBUF keybuf;

void HariMain(void)
{
	struct BOOTINFO *binfo;
	char s[20], mcursor[256];
	int mx, my;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */


	init_palette();
	binfo = (struct BOOTINFO *)ADR_BOOTINFO;

	init_screen(binfo->vram,binfo->scrnx,binfo->scrny);

	// putfonts8_asc(binfo->vram, binfo->scrnx,  8,  8, COL8_FFFFFF, "ABC 123");
	// putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Haribote OS.");
	// putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Haribote OS.");

	// sprintf(s, "scrnx = %d", binfo->scrnx);
	// putfonts8_asc(binfo->vram, binfo->scrnx, 16,64, COL8_FFFFFF, s);

	/* マウスを書く */
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */


	for(;;) {
		io_cli(); // 割り込み禁止
		if(keybuf.len == 0) {
			io_stihlt(); // 割り込み開始
		} else {
			int i = keybuf.data[keybuf.next_r];
			keybuf.len--;
			keybuf.next_r++;
			if(keybuf.next_r == 32) {
				keybuf.next_r = 0;
			}
			io_sti(); // 割り込み開始
			sprintf(s, "%x", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}
	return;
}




