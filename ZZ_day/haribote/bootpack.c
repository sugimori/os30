#include "bootpack.h"

void HariMain(void)
{
	struct BOOTINFO *binfo;
	char s[20], mcursor[256];
	int mx, my;

	init_gdtidt();
	init_pic();

	init_palette();
	binfo = (struct BOOTINFO *)ADR_BOOTINFO;

	init_screen(binfo->vram,binfo->scrnx,binfo->scrny);

	putfonts8_asc(binfo->vram, binfo->scrnx,  8,  8, COL8_FFFFFF, "ABC 123");
	putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Haribote OS.");
	putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Haribote OS.");

	sprintf(s, "scrnx = %d", binfo->scrnx);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16,64, COL8_FFFFFF, s);

	/* マウスを書く */
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 100, COL8_FFFFFF, s);

	for(;;) {
		io_hlt();
	}
	return;
}




