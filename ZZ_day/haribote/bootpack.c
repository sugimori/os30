#include "bootpack.h"

extern struct TIMERCTL timerctl;
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int color, int backcolor, char *s, int length);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void task_b_main(void);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct FIFO32 fifo;
	int fifobuf[128];
	char s[40], mcursor[256];
	int mx, my, i;
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
	struct TIMER *timer, *timer2, *timer3, *timer_ts;
	// cursor
	int cursor_x, cursor_c;
	//マルチタスク
	struct TSS32 tss_a, tss_b;
	

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	// FIFO初期化
	fifo32_init(&fifo, 32, fifobuf);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo,512,&mdec);
	init_pit();
	io_out8(PIC0_IMR, 0xf8); /* PITとPIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);

	/* メモリチェック */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	// multitask
	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR * )	ADR_GDT;
	set_segmdesc(gdt +  3,103, (int) &tss_a, AR_TSS32);
	set_segmdesc(gdt +  4,103, (int) &tss_b, AR_TSS32);
	load_tr(3 * 8);
	int task_b_esp;
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	tss_b.eip = (int)&task_b_main;
	tss_b.eflags = 0x00000202; // IF=1;
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;
	

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	*((int *) 0x0fec) = (int) sht_back;

	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win  = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 透明色なし
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明番号99？
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); // 透明色なし
	init_screen8(buf_back,binfo->scrnx,binfo->scrny); // 背景初期化
	init_mouse_cursor8(buf_mouse, 99);  // マウス初期化
	make_window8(buf_win,160,52,"window");
	// cursor
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;	
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
	unsigned long vramaddr = (unsigned long)(binfo->vram);
	sprintf(s, "VRAM = 0x%l", vramaddr);
	putfonts8_asc_sht(sht_back, 0, 150, COL8_FFFFFF, COL8_008484, s, 80);
	
	for(;;) {
		io_cli(); // 割り込み禁止
		if(fifo32_status(&fifo) == 0) {
			io_stihlt(); // 早すぎるので、HLTを入れるパターン
		} else {
			i = fifo32_get(&fifo);
			io_sti(); // 割り込み開始
			static char keytable[0x54] = {
				  0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',   0,   0,
				'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',   0,   0, 
				'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']',
				'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',
				  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '7',
				'8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '*'
			};

			if( i == 2) {
				farjmp(0, 4*8);
				timer_settime(timer_ts, 2);
			} else if(256 <= i && i <= 511) { // キーボード
				sprintf(s, "%x", i - 256);
				putfonts8_asc_sht(sht_back,0,16,COL8_FFFFFF,COL8_008484,s,2);
				if(i - 256 < 0x54) {
					if(keytable[i-256] != 0 && cursor_x < 144) { // 通常文字
						// 一文字表示してから、カーソルを１つ進める
						s[0] = keytable[i-256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
				
					}
				}
				if(i - 256 == 0x0e && cursor_x > 8) { // バックスペース
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}
				// カーソルの再表示
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);

			} else if (512 <= i && i <= 767) { // マウス
				if(mouse_decode(&mdec, i - 512) != 0) {
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
					if((mdec.btn & 0x01) != 0) {
						// 左ボタンを教えていたら動かす
						sheet_slide(sht_win, mx -80, my -8);
					}
				}

			} else if (i==10) { // 10sec timer
				putfonts8_asc_sht(sht_back,0,64,COL8_FFFFFF,COL8_008484,"10[sec]",7);
			} else if(i==3) {
				putfonts8_asc_sht(sht_back,0,80,COL8_FFFFFF,COL8_008484,"3[sec]",6);
			} else if(i <= 1) { // カーソル用タイマ
				if(i == 1) {
					timer_init(timer3, &fifo, 0);  // 次は0
					cursor_c = COL8_000000;
				} else {
					timer_init(timer3, &fifo, 1);  // 次は1
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7,43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x+8,44);
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

void task_b_main(void)
{
	struct FIFO32 fifo;
	struct TIMER *timer_ts;
	int i, fifobuf[128], count = 0;
	char s[11];
	struct SHEET *sht_back;

	sht_back = (struct SHEET *) *((int *) 0x0fec);

	fifo32_init(&fifo, 128, fifobuf);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 1);
	timer_settime(timer_ts, 2);

	for(;;) {
		count++;
		sprintf(s, "%10d", count);
		putfonts8_asc_sht(sht_back, 0, 180, COL8_FFFFFF, COL8_008484, s, 10);
		io_cli();
		if(fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if(i==1) {
				farjmp(0, 3 * 8);
				timer_settime(timer_ts, 2);
			}
		}
	}
}