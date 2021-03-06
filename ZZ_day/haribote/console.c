#include "bootpack.h"

struct CONSOLE {
	struct SHEET *sht;
	int cur_x, cur_y, cur_c;
};

void cons_putchar(struct CONSOLE *cons, int chr, char move);
void cons_newline(struct CONSOLE *cons);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal);
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline);
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);


void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	int i, fifobuf[128];
	// FAT
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	char cmdline[30];
	cons.sht = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int *) 0x0fec) = (int) &cons;	// consの番地をAPIにわたす

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_raedfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

	// プロンプト
	cons_putchar(&cons, '>', 1);

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
					if(cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;				
					}
				} else {
					timer_init(timer, &task->fifo, 1); // 次は1
					if(cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if(i == 2) {	// カーソルON
				cons.cur_c = COL8_FFFFFF;
			}
			if(i == 3) {	// カーソルOFF
				cons.cur_c = -1;
				boxfill8(sheet->buf,sheet->bxsize,COL8_000000,cons.cur_x ,cons.cur_y,cons.cur_x+7,cons.cur_y+15);
			}

			if(256 <= i && i <= 511) {	// キーボードデータ
				if(i == 8 + 256) {
					// バックスペース
					if(cons.cur_x > 16) {
						cons_putchar(&cons,' ',0);
						cons.cur_x -= 8;

					}
				} else if ( i - 256 == 10) {
					// ENTER
					// カーソルをスペースで消す
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2] = 0; // 最後の文字を終端
					cons_newline(&cons);
					// コマンド実行
					cons_runcmd(cmdline, &cons, fat, memtotal);
					// プロンプト表示
					cons_putchar(&cons, '>', 1);
				} else {
					// 一般文字
					if(cons.cur_x < 240) {
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			// カーソル再表示
			if(cons.cur_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x+7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x+8, cons.cur_y+16);
		}
	}

}

void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if(s[0] == 0x09) {	// タブ
		for(;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if(cons->cur_x == 8 + 240) {	// 画面の端まで行ったら改行
				cons_newline(cons);
			}
			if(((cons->cur_x - 8) & 0x1f) == 0) {
				break;	// 32で割り切れたらbreak
			}
		}
	} else if (s[0] == 0x0a) {	// 改行lf
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	// CR復帰
		// 何もしない
	} else {	// 普通の文字
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if(move != 0) {
			// moveが0のときはカーソルを進めない
			cons->cur_x += 8;
			if(cons->cur_x == 8 + 240) {	// 右端まで来たので改行
				cons_newline(cons);
			}
		}
	}
	return;
}

void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for(; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return ;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
	int i;
	for(i=0; i < l; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return ;
}

void cons_newline(struct CONSOLE *cons) 
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	if (cons->cur_y < 28 + 16 * 7) {
		cons->cur_y += 16;
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
	cons->cur_x = 8;
	return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
	// コマンド実行
	if(strcmp(cmdline, "mem") == 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "clear") == 0){
		cmd_cls(cons);
	} else if (strcmp(cmdline, "dir") == 0) {
		cmd_dir(cons);
	} else if(strncmp(cmdline, "type ", 5) == 0) {
		cmd_type(cons, fat, cmdline);
	} else if (cmdline[0] != 0) {
		if(cmd_app(cons, fat, cmdline) == 0) {
			// コマンドでもなく、空行でもない
			cons_putstr0(cons, "Bad command.\n\n");
		}
	}	
	return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
	// memコマンド
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[30];
	sprintf(s, "total  %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;		
}

void cmd_cls(struct CONSOLE *cons)
{
	// clearコマンド
	int x,y;
	struct SHEET *sheet = cons->sht;
	for(y = 28; y < 28 + 16 * 8;y++) {
		for(x = 8; x < 8 + 8 * 30; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 8 * 30, 28 + 16 * 8);
	cons->cur_y = 28;	
	return;
}

void cmd_dir(struct CONSOLE *cons)
{
	// dir コマンド
	int i,j;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	char s[30];
	for(i=0;i<224;i++) {
		if(finfo[i].name[0] == 0x00) {
			break;
		}
		if(finfo[i].name[0] != 0xe5) {
			if((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d", finfo[i].size);
				for(j=0;j<8;j++) {
					s[j] = finfo[i].name[j];
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons, s);
				cons_newline(cons);
			}
		}
	}
	cons_newline(cons);	
	return;
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline +5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	int i;
	if(finfo != 0) {
		// ファイルが見つかった場合
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int)p, finfo->size);
	} else {
		// ファイルがみつかなかった場合
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);	
	return;
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	char name[18], *p, *q;
	int i;

	// コマンドラインからファイル名を生成
	for(i = 0; i < 13; i++) {
		if(cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;

	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if(finfo == 0 && name[i-1] != '.') {
		// 見つからなかったので、後ろに".HRB"をつけてもう一度探してみる
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);	
	}

	if(finfo != 0) {
		// ファイルが見つかった場合
		p = (char *)memman_alloc_4k(memman, finfo->size);
		q = (char *)memman_alloc_4k(memman, 64 * 1024);
		*((int *)0xfe8) = (int) p;
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, finfo->size -1, (int) p, AR_CODE32_ER);
		set_segmdesc(gdt + 1004, 64 * 1024 -1, (int) q, AR_DATA32_RW);
		if(finfo->size >= 8 && strncmp(p + 4, "Hari", 4) == 0) {
			p[0] = 0xe8;
			p[1] = 0x16;
			p[2] = 0x00;
			p[3] = 0x00;
			p[4] = 0x00;			
			p[5] = 0xcb;
		}
		start_app(0, 1003 * 8, 64 * 1024, 1004 * 8);
		memman_free_4k(memman, (int) p, finfo->size);
		memman_free_4k(memman, (int) q, 64 * 1024);
		cons_newline(cons);
		return 1;
	} 
	return 0;
}

void hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	int cs_base = *((int *)0xfe8);
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	if(edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	} else if(edx == 2) {
		cons_putstr0(cons, (char *) ebx + cs_base);
	} else if (edx == 3) {
		cons_putstr1(cons, (char *) ebx + cs_base, ecx);
	}
	return ;
}

int inthandler0d(int *esp)
{
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	return 1;
}