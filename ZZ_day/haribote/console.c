#include "bootpack.h"

struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();

	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
	char s[30], cmdline[30], *p;
	int x,y;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	// FAT
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);

	file_raedfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

	fifo32_init(&task->fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	// プロンプト
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for(;;) {
		io_cli();
		if(fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if(i<=1) { // カーソル用タイマー
				if(i != 0) {
					timer_init(timer, &task->fifo, 0); // 次は0
					if(cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;				
					}
				} else {
					timer_init(timer, &task->fifo, 1); // 次は1
					if(cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if(i == 2) {	// カーソルON
				cursor_c = COL8_FFFFFF;
			}
			if(i == 3) {	// カーソルOFF
				cursor_c = -1;
				boxfill8(sheet->buf,sheet->bxsize,COL8_000000,cursor_x,cursor_y,cursor_x+7,43);
			}

			if(256 <= i && i <= 511) {	// キーボードデータ
				if(i == 8 + 256) {
					// バックスペース
					if(cursor_x > 16) {
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;

					}
				} else if ( i - 256 == 10) {
					// ENTER
					// カーソルをスペースで消す
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0; // 最後の文字を終端
					cursor_y = cons_newline(cursor_y, sheet);
					// コマンド実行
					if(strcmp(cmdline, "mem") == 0) {
						// memコマンド
						sprintf(s, "total  %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);	
					} else if (strcmp(cmdline, "clear") == 0){
						// clearコマンド
						for(y = 28; y < 28 + 16 * 8;y++) {
							for(x = 8; x < 8 + 8 * 30; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 8 * 30, 28 + 16 * 8);
						cursor_y = 28;
					} else if (strcmp(cmdline, "dir") == 0) {
						// dir コマンド
						for(x=0;x<224;x++) {
							if(finfo[x].name[0] == 0x00) {
								break;
							}
							if(finfo[x].name[0] != 0xe5) {
								if((finfo[x].type & 0x18) == 0) {
									sprintf(s, "filename.ext   %7d", finfo[x].size);
									for(y=0;y<8;y++) {
										s[y] = finfo[x].name[y];
									}
									s[ 9] = finfo[x].ext[0];
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if(strncmp(cmdline, "type ", 5) == 0) {
						// type コマンド
						// ファイル名
						for(y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						y = 0;
						for(x=5; y < 11 && cmdline[x] != 0; x++) {
							if(cmdline[x] == '.' && y <= 8) {
								y=8;	// .がきたら拡張子なのでスキップ
							} else {
								s[y] = cmdline[x];
								if('a' <= s[y] && s[y] <= 'z') {
									// 小文字だったら大文字に
									s[y] -= 0x20;
								}
								y++;
							}
						}
						// ファイルを探す
						for(x=0;x<224;) {
							if(finfo[x].name[0] == 0x00) {
								break;
							}
							if((finfo[x].type & 0x18) == 0) {
								for(y=0;y<11;y++) {
									if(finfo[x].name[y] != s[y]) {
										goto type_next_file;	// x++;continue;でいいのでは？
									}
								}
								break; // ファイルが見つかった
							}
type_next_file:
							x++;
						}
						if(x<224 && finfo[x].name[0] != 0x00) {
							// ファイルが見つかった場合
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							cursor_x = 8;
							for(y=0;y<finfo[x].size;y++) {
								// １文字ずつ表示
								s[0] = p[y];
								s[1] = 0;
								if(s[0] == 0x09) {	// タブ
									for(;;) {
										putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										if(cursor_x == 8 + 240) {	// 画面の端まで行ったら改行
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
										if(((cursor_x - 8) & 0x1f) == 0) {
											break;	// 32で割り切れたらbreak
										}
									}
								} else if (s[0] == 0x0a) {	// 改行lf
									cursor_x = 8;
									cursor_y = cons_newline(cursor_y, sheet);
								} else if (s[0] == 0x0d) {	// CR復帰
									// 何もしない
								} else {
									putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
									cursor_x += 8;
									if(cursor_x == 8 + 240) {	// 右端まで来たので改行
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							memman_free_4k(memman, (int)p, finfo[x].size);
						} else {
							// ファイルがみつかなかった場合
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);

					} else if (cmdline[0] != 0) {
						// コマンドでもなく、空行でもない
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);						
					}
					// プロンプト表示
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				} else {
					// 一般文字
					if(cursor_x < 240) {
						s[0] = i -256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			// カーソル再表示
			if(cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x+7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x+8, cursor_y+16);
		}



	}

}

int cons_newline(int cursor_y, struct SHEET *sheet) 
{
	int x, y;
	if (cursor_y < 28 + 16 * 7) {
		cursor_y += 16;
	} else {
		// スクロール
		for(y = 28; y < 28 + 16 * 7; y++) {
			for( x = 8; x < 8 + 8 * 30; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		// 最後の行は黒で塗りつぶす
		for(y = 28 + 16 * 7; y < 28 + 16 * 8; y++) {
			for( x = 8; x < 8 + 8 * 30; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8,28, 8 + 8 * 30, 28 + 16 * 8);
	}
	return cursor_y;
}