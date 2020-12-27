#include "bootpack.h"

extern struct FIFO8 keyfifo,mousefifo;
extern struct TIMERCTL timerctl;
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int color, int backcolor, char *s, int length);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128], timerbuf[8], timerbuf2[8], timerbuf3[8];
	int mx, my, i, count=0;
	unsigned char mouse_dbuf[3], mouse_phase;
	struct MOUSE_DEC mdec;
	// memory関連宣言
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	// sheet関連
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	// timer
	struct FIFO8 timerfifo;
	struct TIMER *timer, *timer2, *timer3;
	

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	init_pit();
	io_out8(PIC0_IMR, 0xf8); /* PITとPIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	fifo8_init(&timerfifo, 8, timerbuf);
	timer = timer_alloc();
	timer_init(timer, &timerfifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &timerfifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &timerfifo, 1);
	timer_settime(timer3, 50);

	

	init_keyboard();
	enable_mouse(&mdec);
	/* メモリチェック */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win  = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 透明色なし
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明番号99？
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); // 透明色なし
	init_screen8(buf_back,binfo->scrnx,binfo->scrny); // 背景初期化
	init_mouse_cursor8(buf_mouse, 99);  // マウス初期化
	make_window8(buf_win,160,52,"counter");
	sheet_slide(sht_back, 0,0); // 背景の位置を設定
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back, 0);	// 背景は０固定？
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);
	
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	sprintf(s,"memory %dMB    free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

	
	for(;;) {
		count++;
		// sprintf(s, "%d", timerctl.count);
		// putfonts8_asc_sht(sht_win, 40,28,COL8_000000,COL8_C6C6C6,s, 10);
		// putfonts8_asc_sht(sht_back, 0,120,COL8_000000,COL8_C6C6C6,"dummy", 10);	// ダミー

		io_cli(); // 割り込み禁止
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) + fifo8_status(&timerfifo) == 0) {
			io_stihlt(); // 割り込み開始
		} else {
			if(fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti(); // 割り込み開始
				sprintf(s, "%x", i);
				putfonts8_asc_sht(sht_back,0,16,COL8_FFFFFF,COL8_008484,s,2);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec, i) == 1) {
					sprintf(s, "[lcr %d %d]", mdec.x, mdec.y);
					if((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back,32,16,COL8_FFFFFF,COL8_008484,s,15);

					// マウスカーソルの移動
					mx += mdec.x;
					my += mdec.y;
					if(mx < 0) mx = 0;
					if(my < 0) my = 0;
					if(mx > binfo->scrnx - 1) mx = binfo->scrnx -1;
					if(my > binfo->scrny - 1) my = binfo->scrny -1;
					sprintf(s, "(%d, %d)", mx, my);
					putfonts8_asc_sht(sht_back,0,0,COL8_FFFFFF,COL8_008484,s,12);
					sheet_slide(sht_mouse,mx,my);
				}

			} else if (fifo8_status(&timerfifo) != 0) {
				i = fifo8_get(&timerfifo);
				io_sti();
				if(i==10) {
					putfonts8_asc_sht(sht_back,0,64,COL8_FFFFFF,COL8_008484,"10[sec]",7);
					sprintf(s,"%d", count);
					putfonts8_asc_sht(sht_win, 40,28,COL8_000000,COL8_C6C6C6,s, 10);
				} else if(i==3) {
					putfonts8_asc_sht(sht_back,0,80,COL8_FFFFFF,COL8_008484,"3[sec]",6);
					count = 0;
				} else {
					if(i!=0) {
						timer_init(timer3, &timerfifo, 0); 
						boxfill8(buf_back, binfo->scrnx,COL8_FFFFFF, 8, 96,15,111);
					} else {
						timer_init(timer3, &timerfifo, 1); 
						boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
					}
					timer_settime(timer3, 50);
					sheet_refresh(sht_back, 8, 96,16,112);
				}
			}
		}
	}
	return;
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int color, int backcolor, char *s, int length)
{
	boxfill8(sht->buf, sht->bxsize,backcolor,x,y,x+length*8-1,y+15);
	putfonts8_asc(sht->buf,sht->bxsize,x,y,color,s);
	sheet_refresh(sht,x,y,x+length*8,y+16);
	return;
}