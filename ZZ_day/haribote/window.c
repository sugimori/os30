#include "bootpack.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	// 上辺
	boxfill8(buf, xsize, COL8_000000,	0,			0,			xsize - 1,	1	);
	// boxfill8(buf, xsize, COL8_000000,	1,			1,			xsize - 2,	1	);
	// 左辺
	boxfill8(buf, xsize, COL8_000000,	0,			0,			1,			ysize -1);
	// boxfill8(buf, xsize, COL8_FFFFFF,	1,			1,			1,			ysize -2);

	// 右辺
	boxfill8(buf, xsize, COL8_000000,	xsize -2,	0,			xsize -1, 	ysize -1);
	// boxfill8(buf, xsize, COL8_FFFFFF,	xsize -1,	0,			xsize -1,	ysize -1);

	// 内側
	boxfill8(buf, xsize, COL8_FFFFFF,	2,			2,			xsize -3,	ysize -3);

	// 下辺
	boxfill8(buf, xsize, COL8_000000,	0,			ysize -2,	xsize -1,	ysize -1);
	// boxfill8(buf, xsize, COL8_FFFFFF,	0,			ysize -1,	xsize -1,	ysize -1);
	make_wtitle8(buf, xsize, title, act);
	return ;
}


void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};

	int x, y,i;
	char c, tc, tbc;
	if(act != 0) {
		for(i=0;i<6;i++) {
			boxfill8(buf, xsize, COL8_000000, 3, 5 + 3*i , xsize -4, 5 + 3 * i);
		}
		boxfill8(buf, xsize, COL8_FFFFFF, 3 + 10 - 2, 5 + 0 - 2, 3 + 10 + 15 + 2, 5 + 15 + 2);
		boxfill8(buf, xsize, COL8_000000, 3 + 10    , 5 + 0    , 3 + 10 + 15    , 5 + 15);
		boxfill8(buf, xsize, COL8_FFFFFF, 3 + 10 + 2, 5 + 0 + 2, 3 + 10 + 15 - 2, 5 + 15 - 2);
	} else {
		boxfill8(buf,xsize,COL8_FFFFFF, 3, 5, xsize - 4, 5 + 15);
	}
	x = xsize/2 - strlen(title) * 8 / 2;
	boxfill8(buf,xsize, COL8_FFFFFF, x - 5,4, x + strlen(title) * 8 + 5,20);
	putfonts8_asc(buf, xsize, x,4, COL8_000000, title);
	return ;
}


void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 -2, y0 -3, x1 +1, y0 -3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 -3, y0 -3, x0 -3, y1 +1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 -3, y1 +2, x1 +1, y1 +2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 +2, y0 -3, x1 +2, y1 +2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 -1, y0 -2, x1 +0, y0 -2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 -2, y0 -2, x0 -2, y1 +0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 -2, y1 +1, x1 +0, y1 +1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 +1, y0 -2, x1 +1, y1 +1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 -1, y0 -1, x1 +0, y1 +0);


	return;
}