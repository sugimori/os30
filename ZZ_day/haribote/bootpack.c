#include "bootpack.h"

extern struct TIMERCTL timerctl;
extern struct TIMER *task_timer;
#define KEYCMD_LED	0xed

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct FIFO32 fifo, keycmd;
	int fifobuf[128], keycmd_buf[128];
	char s[40], mcursor[256];
	int mx, my, i;
	unsigned char mouse_dbuf[3], mouse_phase;
	struct MOUSE_DEC mdec;
	// memory関連宣言
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	// sheet関連
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	// timer
	struct TIMER *timer;
	// cursor
	int cursor_x, cursor_c;
	//マルチタスク
	struct TASK *task_a, *task_cons;
	

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	// FIFO初期化
	fifo32_init(&fifo, 32, fifobuf,0);
	fifo32_init(&keycmd, 32, keycmd_buf,0);
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo,512,&mdec);
	init_pit();
	io_out8(PIC0_IMR, 0xf8); /* PITとPIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */


	/* メモリチェック */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);


	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	// sht_back
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 透明色なし
	init_screen8(buf_back,binfo->scrnx,binfo->scrny); // 背景初期化

	// sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); // 透明色なし
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);

	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int)&console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2);	// level=2, priority=2

	// sht_win
	sht_win = sheet_alloc(shtctl);
	buf_win  = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); // 透明色なし
	make_window8(buf_win,160,52,"window",1);
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	// cursor
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;	
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	// mouse
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明番号99？
	init_mouse_cursor8(buf_mouse, 99);  // マウス初期化
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back, 0,0); // 背景の位置を設定
	sheet_slide(sht_cons, 32,  100);
	sheet_slide(sht_win,  64,  56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);	// 背景は０固定？
	sheet_updown(sht_cons, 1);
	sheet_updown(sht_win, 2);
	sheet_updown(sht_mouse, 3);

	// http://oswiki.osask.jp/?%28AT%29keyboard
	static char keytable0[0x80] = {
			0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',   0,   0, // 00-0F
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',   0,   0, 'A', 'S', // 10-1F
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']',	'Z', 'X', 'C', 'V', // 20-2F
		'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0, // 30-3F
			0,   0,   0,   0,   0,   0,   0, '7',	'8', '9', '-', '4', '5', '6', '+', '1', // 40-4F
		'2', '3', '0', '.',	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 50-5F
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60-6F
			0,   0,   0,0x5c,   0,   0,   0,   0,   0,   0,   0,   0,   0,0x5c,   0,   0, // 70-7F
	};
	static char keytable1[0x80] = {
			0,   0, '!',0x22, '#', '$', '%', '&',0x27, '(', ')', '~', '=', '~',   0,   0, // 00-0F
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{',   0,   0, 'A', 'S', // 10-1F
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*',   0,   0, '}',	'Z', 'X', 'C', 'V', // 20-2F
		'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0, // 30-3F
			0,   0,   0,   0,   0,   0,   0, '7',	'8', '9', '-', '4', '5', '6', '+', '1', // 40-4F
		'2', '3', '0', '.',	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 50-5F
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60-6F
			0,   0,   0, '_',   0,   0,   0,   0,   0,   0,   0,   0,   0, '|',   0,   0, // 70-7F
	};
	int key_to = 0, key_shift = 0;
	int key_leds = (binfo->leds >> 4) & 7;	// 1: ScrollLock, 2: NumLock, 4: CapsLock
	int keycmd_wait = -1;
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);


	for(;;) {
		if(fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			// キーボードコントローラーに送るものがあれば送る
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli(); // 割り込み禁止
		if(fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_stihlt(); // 早すぎるので、HLTを入れるパターン
		} else {
			i = fifo32_get(&fifo);
			io_sti(); // 割り込み開始

			if(256 <= i && i <= 511) { // キーボード
				if(i - 256 < 0x80 ) {	// キーコードを文字コードに変換
					if(key_shift == 0) {
						s[0] = keytable0[i-256];
					} else {
						s[0] = keytable1[i-256];
					}
				} else {
					s[0] = 0;
				}
				if('A' <= s[0] && s[0] <= 'Z') {	// 入力がアルファベット
					if(((key_leds & 4) == 0 && key_shift == 0) 			// CapsLock=OFF & Shift=OFF
						|| ((key_leds & 4) != 0 && key_shift != 0)) {	// CapsLock=ON & Shift=ON
							s[0] += 0x20;	// 大文字を小文字に変換
						}
				}

				if(s[0] != 0) { // 通常文字
					if(key_to == 0) {	// タスクAへの入力
						if(cursor_x < 128) { 
							// 一文字表示してから、カーソルを１つ進める
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
					
						}
					} else {	// コンソールへ
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if(i - 256 == 0x0e) { // バックスペース
					if(key_to == 0) {	// タスクAへの入力
						if(cursor_x > 8) { 
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {	// コンソール
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if(i - 256 == 0x0f) { // TAB
					if(key_to == 0) {	// consoleをアクティブに
						key_to = 1;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;	// カーソルを消す
						boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
						fifo32_put(&task_cons->fifo,2);	// コンソールのカーソルON
					} else {			// task_aをアクティブに
						key_to = 0;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000; // カーソルを出す
						fifo32_put(&task_cons->fifo,3);	// コンソールのカーソルOFF
					}
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if(i - 256 == 0x1c) {	// ENTER
					if(key_to != 0) {	// コンソールへ
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if(i - 256 == 0x2a) {	// 左SHIFT ON
					key_shift |= 1;
				}
				if(i - 256 == 0x36) {	// 右SHIFT ON
					key_shift |= 2;
				}
				if(i - 256 == 0xaa) {	// 左SHIFT OFF
					key_shift &= ~1;
				}
				if(i - 256 == 0xb6) {	// 右SHIFT OFF
					key_shift &= ~2;
				}
				if(i - 256 == 0x3a) {	// CapsLock
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if(i - 256 == 0x45) {	// NumLock
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if(i - 256 == 0x46) {	// ScrollLock
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if(i -256 == 0xfa) {	// キーボードが無事デーをう受け取った
					keycmd_wait = -1;
				}
				if(i -256 == 0xfe) {	// キーボードが無事デーをう受け取れなかった
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}


				// カーソルの再表示
				if(cursor_c > 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { // マウス
				if(mouse_decode(&mdec, i - 512) != 0) {
					// マウスカーソルの移動
					mx += mdec.x;
					my += mdec.y;
					if(mx < 0) mx = 0;
					if(my < 0) my = 0;
					if(mx > binfo->scrnx - 1) mx = binfo->scrnx -1;
					if(my > binfo->scrny - 1) my = binfo->scrny -1;
					// sprintf(s, "(%d, %d)", mx, my);
					// putfonts8_asc_sht(sht_back,0,0,COL8_FFFFFF,COL8_008484,s,12);
					sheet_slide(sht_mouse,mx,my);
					if((mdec.btn & 0x01) != 0) {
						// 左ボタンを教えていたら動かす
						sheet_slide(sht_win, mx -80, my -8);
					}
				}

			} else if(i <= 1) { // カーソル用タイマ
				if(i == 1) {
					timer_init(timer, &fifo, 0);  // 次は0
					if(cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				} else {
					timer_init(timer, &fifo, 1);  // 次は1
					if(cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if(cursor_c >= 0 ) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7,43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x+8,44);
				}
			}
		}
	}
	return;
}