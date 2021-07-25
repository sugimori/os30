#include "bootpack.h"
extern struct TASKCTL *taskctl;

void cons_putchar(struct CONSOLE *cons, int chr, char move);
void cons_newline(struct CONSOLE *cons);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_langmode(struct CONSOLE *cons, char *cmdline);
void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_exit(struct CONSOLE *cons, int *fat);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal);
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);
void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col);

void console_task(struct SHEET *sheet, unsigned int memtotal) {
  struct TASK *task = task_now();
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEHANDLE fhandle[8];
  int i;
  // FAT
  int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
  struct CONSOLE cons;
  char cmdline[30];
  cons.sht = sheet;
  cons.cur_x = 8;
  cons.cur_y = 28;
  cons.cur_c = -1;
  task->cons = &cons;
  task->cmdline = cmdline;
  unsigned char *nihongo = (char *)*((int *)0x0fe8);

  for (i = 0; i < 8; i++) {
    fhandle[i].buf = 0;  // 未使用
  }
  task->fhandle = fhandle;
  task->fat = fat;

  if (nihongo[4096] != 0xff) {  // 日本フォンをを読み込めたか
    task->langmode = 1;
  } else {
    task->langmode = 0;
  }

  if (cons.sht != 0) {
    cons.timer = timer_alloc();
    timer_init(cons.timer, &task->fifo, 1);
    timer_settime(cons.timer, 50);
  }
  file_raedfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

  // プロンプト
  cons_putchar(&cons, '>', 1);

  for (;;) {
    io_cli();
    if (fifo32_status(&task->fifo) == 0) {
      task_sleep(task);
      io_sti();
    } else {
      i = fifo32_get(&task->fifo);
      io_sti();
      if (i <= 1 && cons.sht != 0) {  // カーソル用タイマー
        if (i != 0) {
          timer_init(cons.timer, &task->fifo, 0);  // 次は0
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_FFFFFF;
          }
        } else {
          timer_init(cons.timer, &task->fifo, 1);  // 次は1
          if (cons.cur_c >= 0) {
            cons.cur_c = COL8_000000;
          }
        }
        timer_settime(cons.timer, 50);
      }
      if (i == 2) {  // カーソルON
        cons.cur_c = COL8_FFFFFF;
      }
      if (i == 3) {  // カーソルOFF
        if (cons.sht != 0) {
          boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7,
                   cons.cur_y + 15);
        }
        cons.cur_c = -1;
      }
      if (i == 4) {  // コンソーバツ✗ボタン
        cmd_exit(&cons, fat);
      }

      if (256 <= i && i <= 511) {  // キーボードデータ
        if (i == 8 + 256) {
          // バックスペース
          if (cons.cur_x > 16) {
            cons_putchar(&cons, ' ', 0);
            cons.cur_x -= 8;
          }
        } else if (i - 256 == 10) {
          // ENTER
          // カーソルをスペースで消す
          cons_putchar(&cons, ' ', 0);
          cmdline[cons.cur_x / 8 - 2] = 0;  // 最後の文字を終端
          cons_newline(&cons);
          // コマンド実行
          cons_runcmd(cmdline, &cons, fat, memtotal);
          if (cons.sht == 0) {
            cmd_exit(&cons, fat);
          }
          // プロンプト表示
          cons_putchar(&cons, '>', 1);
        } else {
          // 一般文字
          if (cons.cur_x < 240) {
            cmdline[cons.cur_x / 8 - 2] = i - 256;
            cons_putchar(&cons, i - 256, 1);
          }
        }
      }
      if (cons.sht != 0) {
        // カーソル再表示
        if (cons.cur_c >= 0) {
          boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7,
                   cons.cur_y + 15);
        }
        sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
      }
    }
  }
}

void cons_putchar(struct CONSOLE *cons, int chr, char move) {
  char s[2];
  s[0] = chr;
  s[1] = 0;
  if (s[0] == 0x09) {  // タブ
    for (;;) {
      if (cons->sht != 0) {
        putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
      }
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {  // 画面の端まで行ったら改行
        cons_newline(cons);
      }
      if (((cons->cur_x - 8) & 0x1f) == 0) {
        break;  // 32で割り切れたらbreak
      }
    }
  } else if (s[0] == 0x0a) {  // 改行lf
    cons_newline(cons);
  } else if (s[0] == 0x0d) {  // CR復帰
                              // 何もしない
  } else {                    // 普通の文字
    if (cons->sht != 0) {
      putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
    }
    if (move != 0) {
      // moveが0のときはカーソルを進めない
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {  // 右端まで来たので改行
        cons_newline(cons);
      }
    }
  }
  return;
}

void cons_putstr0(struct CONSOLE *cons, char *s) {
  for (; *s != 0; s++) {
    cons_putchar(cons, *s, 1);
  }
  return;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l) {
  int i;
  for (i = 0; i < l; i++) {
    cons_putchar(cons, s[i], 1);
  }
  return;
}

void cons_newline(struct CONSOLE *cons) {
  int x, y;
  struct SHEET *sheet = cons->sht;
  if (cons->cur_y < 28 + 16 * 7) {
    cons->cur_y += 16;
  } else {
    // スクロール
    if (sheet != 0) {
      for (y = 28; y < 28 + 16 * 7; y++) {
        for (x = 8; x < 8 + 8 * 30; x++) {
          sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
        }
      }
      // 最後の行は黒で塗りつぶす
      for (y = 28 + 16 * 7; y < 28 + 16 * 8; y++) {
        for (x = 8; x < 8 + 8 * 30; x++) {
          sheet->buf[x + y * sheet->bxsize] = COL8_000000;
        }
      }
      sheet_refresh(sheet, 8, 28, 8 + 8 * 30, 28 + 16 * 8);
    }
  }
  cons->cur_x = 8;
  return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal) {
  // コマンド実行
  if (strcmp(cmdline, "mem") == 0 && cons->sht != 0) {
    cmd_mem(cons, memtotal);
  } else if (strcmp(cmdline, "clear") == 0 && cons->sht != 0) {
    cmd_cls(cons);
  } else if (strcmp(cmdline, "dir") == 0 && cons->sht != 0) {
    cmd_dir(cons);
  } else if (strcmp(cmdline, "exit") == 0) {
    cmd_exit(cons, fat);
  } else if (strncmp(cmdline, "start ", 6) == 0) {
    cmd_start(cons, cmdline, memtotal);
  } else if (strncmp(cmdline, "ncst ", 5) == 0) {
    cmd_ncst(cons, cmdline, memtotal);
  } else if (strncmp(cmdline, "langmode ", 9) == 0) {
    cmd_langmode(cons, cmdline);
  } else if (cmdline[0] != 0) {
    if (cmd_app(cons, fat, cmdline) == 0) {
      // コマンドでもなく、空行でもない
      cons_putstr0(cons, "Bad command.\n\n");
    }
  }
  return;
}

void cmd_langmode(struct CONSOLE *cons, char *cmdline) {
  struct TASK *task = task_now();
  unsigned char mode = cmdline[9] - '0';
  if (mode <= 1) {
    task->langmode = mode;
  } else {
    cons_putstr0(cons, "mode number error.\n");
  }
  cons_newline(cons);
  return;
}

void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal) {
  struct TASK *task = open_constask(0, memtotal);
  struct FIFO32 *fifo = &task->fifo;
  int i;
  // コマンドラインに入さされた文字をｗ、１文字ずつ新しこコンソーに入力
  for (i = 5; cmdline[i] != 0; i++) {
    fifo32_put(fifo, cmdline[i] + 256);
  }
  fifo32_put(fifo, 10 + 256);  // ENTER
  cons_newline(cons);
  return;
}

void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal) {
  struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0x0fe4);
  struct SHEET *sht = open_console(shtctl, memtotal);
  struct FIFO32 *fifo = &sht->task->fifo;
  int i;
  sheet_slide(sht, 32, 34);
  sheet_updown(sht, shtctl->top);
  // コマンドラインに入さされた文字をｗ、１文字ずつ新しこコンソーに入力
  for (i = 6; cmdline[i] != 0; i++) {
    fifo32_put(fifo, cmdline[i] + 256);
  }
  fifo32_put(fifo, 10 + 256);  // ENTER
  cons_newline(cons);
  return;
}

void cmd_exit(struct CONSOLE *cons, int *fat) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct TASK *task = task_now();
  struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0x0fe4);
  struct FIFO32 *fifo = (struct FIFO32 *)*((int *)0x0fec);
  if (cons->sht != 0) {
    timer_cancel(cons->timer);
  }
  memman_free_4k(memman, (int)fat, 4 * 2880);
  io_cli();
  if (cons->sht != 0) {
    fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);  // 768〜1023
  } else {
    fifo32_put(fifo, task - taskctl->task0 + 1024);  // 1024〜2023
  }
  io_sti();
  for (;;) {
    task_sleep(task);
  }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) {
  // memコマンド
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  char s[30];
  sprintf(s, "total  %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
  cons_putstr0(cons, s);
  return;
}

void cmd_cls(struct CONSOLE *cons) {
  // clearコマンド
  int x, y;
  struct SHEET *sheet = cons->sht;
  for (y = 28; y < 28 + 16 * 8; y++) {
    for (x = 8; x < 8 + 8 * 30; x++) {
      sheet->buf[x + y * sheet->bxsize] = COL8_000000;
    }
  }
  sheet_refresh(sheet, 8, 28, 8 + 8 * 30, 28 + 16 * 8);
  cons->cur_y = 28;
  return;
}

void cmd_dir(struct CONSOLE *cons) {
  // dir コマンド
  int i, j;
  struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
  char s[30];
  for (i = 0; i < 224; i++) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if (finfo[i].name[0] != 0xe5) {
      if ((finfo[i].type & 0x18) == 0) {
        sprintf(s, "filename.ext   %7d", finfo[i].size);
        for (j = 0; j < 8; j++) {
          s[j] = finfo[i].name[j];
        }
        s[9] = finfo[i].ext[0];
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

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline) {
  int seqsiz, datsiz, esp, dathrb;
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  char name[18], *p, *q;
  struct TASK *task = task_now();
  struct SHTCTL *shtctl;
  struct SHEET *sht;
  int i;

  // コマンドラインからファイル名を生成
  for (i = 0; i < 13; i++) {
    if (cmdline[i] <= ' ') {
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] = 0;

  finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == 0 && name[i - 1] != '.') {
    // 見つからなかったので、後ろに".HRB"をつけてもう一度探してみる
    name[i] = '.';
    name[i + 1] = 'H';
    name[i + 2] = 'R';
    name[i + 3] = 'B';
    name[i + 4] = 0;
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  }

  if (finfo != 0) {
    // ファイルが見つかった場合
    p = (char *)memman_alloc_4k(memman, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
    if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
      seqsiz = *((int *)(p + 0x0000));
      esp = *((int *)(p + 0x000c));
      datsiz = *((int *)(p + 0x0010));
      dathrb = *((int *)(p + 0x0014));
      q = (char *)memman_alloc_4k(memman, seqsiz);
      task->ds_base = (int)q;  //*((int *)0xfe8) = (int)q;
      set_segmdesc(task->ldt + 0, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
      set_segmdesc(task->ldt + 1, seqsiz - 1, (int)q, AR_DATA32_RW + 0x60);
      for (i = 0; i < datsiz; i++) {
        q[esp + i] = p[dathrb + i];
      }
      start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));  // +4でLDT内のセグメンという意味
      // 終了処理
      shtctl = (struct SHTCTL *)*((int *)0x0fe4);
      for (i = 0; i < MAX_SHEETS; i++) {
        sht = &(shtctl->sheets0[i]);
        if ((sht->flags & 0x11) == 0x11 && sht->task == task) {
          // アプリが開きっぱなしにした下敷きを発見
          sheet_free(sht);
        }
      }
      for (i = 0; i < 8; i++) {  // クローズしてないファイルをクローズ
        if (task->fhandle[i].buf != 0) {
          memman_free_4k(memman, (int)task->fhandle[i].buf, task->fhandle[i].size);
          task->fhandle[i].buf = 0;
        }
      }
      timer_cancelall(&task->fifo);
      memman_free_4k(memman, (int)q, seqsiz);
    } else {
      cons_putstr0(cons, ".hrb file format error.\n");
    }
    memman_free_4k(memman, (int)p, finfo->size);
    cons_newline(cons);
    return 1;
  }
  return 0;
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
  struct TASK *task = task_now();
  int ds_base = task->ds_base;
  struct CONSOLE *cons = task->cons;
  struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0x0fe4);
  struct SHEET *sht;
  struct FIFO32 *sys_fifo = (struct FIFO32 *)*((int *)0xfec);
  struct FILEINFO *finfo;
  struct FILEHANDLE *fh;
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  char s[20];
  int i;
  int *reg = &eax + 1;  // eaxの次の番地
                        // 保存のためにPUSHADを強引に書き換える
                        // reg[0]: EDI, reg[1]: ESI, reg[2]: EBP, reg[3]: ESP
                        // reg[4]: EBX, reg[5]: EDX, reg[6]: ECX, reg[7]: EAX

  if (edx == 1) {
    cons_putchar(cons, eax & 0xff, 1);
  } else if (edx == 2) {
    cons_putstr0(cons, (char *)ebx + ds_base);
  } else if (edx == 3) {
    cons_putstr1(cons, (char *)ebx + ds_base, ecx);
  } else if (edx == 4) {
    return &(task->tss.esp0);
  } else if (edx == 5) {
    // int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
    // EDX=5, EBX=window buffer, ESI=xsize, EDI=ysize, EAX=透明色, ECX=window
    // name
    sht = sheet_alloc(shtctl);
    sht->task = task;
    sht->flags |= 0x10;
    sheet_setbuf(sht, (char *)ebx + ds_base, esi, edi, eax);
    make_window8((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
    sht->title = (char *)ecx + ds_base;  // titleをSHEETに入れる
    sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
    sheet_updown(sht, shtctl->top);  // マウスと同じ高さ
    reg[7] = (int)sht;               // EAXを書き換えて戻り値にする
  } else if (edx == 6) {
    // void api_putstrwin(int win, int x, int y, int col, int len, char *str);
    // EDX=6, EBX=window number, ESI=xpos, EDI=ypos, EAX=color number,
    // ECX=string length EBP=string
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *)ebp + ds_base);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
    }
  } else if (edx == 7) {
    // void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
    // EDX=7, EBX=window number, EAX=x0, ECX=y0, ESI=x1, EDI=y1, EBP=color
    // number
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
    // sprintf(s, "eax:%d,ecx:%d", eax, ecx);
    // putfonts8_asc(sht->buf, sht->bxsize, 28,28,0,s);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    }
  } else if (edx == 8) {
    // void api_initmalloc(void);
    memman_init((struct MEMMAN *)(ebx + ds_base));
    ecx &= 0xfffffff0;
    memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
  } else if (edx == 9) {
    // char * api_malloc(int size);
    ecx = (ecx + 0x0f) & 0xfffffff0;
    reg[7] = memman_alloc((struct MEMMAN *)(ebx + ds_base), ecx);
  } else if (edx == 10) {
    // void api_free(char *addr, int size);
    ecx = (ecx + 0x0f) & 0xffffff0;
    memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
  } else if (edx == 11) {
    //  void api_point(int win, int x, int y, int col);
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    sht->buf[sht->bxsize * edi + esi] = eax;
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
    }
  } else if (edx == 12) {
    sht = (struct SHEET *)ebx;
    sheet_refresh(sht, eax, ecx, esi, edi);
  } else if (edx == 13) {
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    }
  } else if (edx == 14) {
    sheet_free((struct SHEET *)ebx);
  } else if (edx == 15) {
    for (;;) {
      io_cli();
      if (fifo32_status(&task->fifo) == 0) {
        if (eax != 0) {
          task_sleep(task);
        } else {
          io_sti();
          reg[7] = -1;
          return 0;
        }
      }
      i = fifo32_get(&task->fifo);
      io_sti();
      if (i <= 1) {  // カーソル用タイマー
        // アプリ実行中はカーソルが出ないので、いつも次は表示用の１を注文しておく
        timer_init(cons->timer, &task->fifo, 1);  // 次は１
        timer_settime(cons->timer, 50);
      }
      if (i == 2) {  // カーソルON
        cons->cur_c = COL8_FFFFFF;
      }
      if (i == 3) {  // カーソルOFF
        cons->cur_c = -1;
      }
      if (i == 4) {  // コンソーだけを閉じる
        timer_cancel(cons->timer);
        io_cli();
        fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);
        cons->sht = 0;
        io_sti();
      }
      if (i >= 256) {  // キーボードデータ
        reg[7] = i - 256;
        return 0;
      }
    }
  } else if (edx == 16) {
    reg[7] = (int)timer_alloc();
    ((struct TIMER *)reg[7])->flags2 = 1;  // 自動キャンセル有効
  } else if (edx == 17) {
    timer_init((struct TIMER *)ebx, &task->fifo, eax + 256);
  } else if (edx == 18) {
    timer_settime((struct TIMER *)ebx, eax);
  } else if (edx == 19) {
    timer_free((struct TIMER *)ebx);
  } else if (edx == 20) {
    if (eax == 0) {
      i = io_in8(0x61);
      io_out8(0x61, i & 0x0d);
    } else {
      i = 1193180000 / eax;
      io_out8(0x43, 0xb6);
      io_out8(0x42, i & 0xff);
      io_out8(0x42, i >> 8);
      i = io_in8(0x61);
      io_out8(0x61, (i | 0x03) & 0x0f);
    }
  } else if (edx == 21) {
    // int api_fopen(char *fname);
    for (i = 0; i < 8; i++) {
      if (task->fhandle[i].buf == 0) {
        break;
      }
    }
    fh = &task->fhandle[i];
    reg[7] = 0;
    if (i < 8) {
      finfo = file_search((char *)ebx + ds_base, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
      if (finfo != 0) {
        reg[7] = (int)fh;
        fh->buf = (char *)memman_alloc_4k(memman, finfo->size);
        fh->size = finfo->size;
        fh->pos = 0;
        file_loadfile(finfo->clustno, finfo->size, fh->buf, task->fat, (char *)(ADR_DISKIMG + 0x003e00));
      }
    }
  } else if (edx == 22) {
    // void api_fclose(int fhandle);
    fh = (struct FILEHANDLE *)eax;
    memman_free_4k(memman, (int)fh->buf, fh->size);
    fh->buf = 0;
  } else if (edx == 23) {
    // void api_fseek(int fhandle, int offset, int mode);
    // 0:先頭、1:現在のアクセス位置、2:ファイルの終端
    fh = (struct FILEHANDLE *)eax;
    if (ecx == 0) {
      fh->pos = ebx;
    } else if (ecx == 1) {
      fh->pos += ebx;
    } else if (ecx == 2) {
      fh->pos = fh->size + ebx;
    }
    if (fh->pos < 0) {
      fh->pos = 0;
    }
    if (fh->pos > fh->size) {
      fh->pos = fh->size;
    }
  } else if (edx == 24) {
    // int api_fsize(int fhandle, int mode);
    // 0:ファイルサイズ、1:現のの読み込み位ままで、2:ファイル終端からみた現い位置までのバイト数
    fh = (struct FILEHANDLE *)eax;
    if (ecx == 0) {
      reg[7] = fh->size;
    } else if (ecx == 1) {
      reg[7] = fh->pos;
    } else if (ecx == 2) {
      reg[7] = fh->pos - fh->size;
    }
  } else if (edx == 25) {
    // int api_fread(char *buf, int maxsize, int fhandle);
    fh = (struct FILEHANDLE *)eax;
    for (i = 0; i < ecx; i++) {
      if (fh->pos == fh->size) {
        break;
      }
      *((char *)ebx + ds_base + i) = fh->buf[fh->pos];
      fh->pos++;
    }
    reg[7] = i;
  } else if (edx == 26) {
    // int api_cmdline(char *buf, int maxsize);
    i = 0;
    for (;;) {
      *((char *)ebx + ds_base + i) = task->cmdline[i];
      if (task->cmdline[i] == 0) {
        break;
      }
      if (i >= ecx) {
        break;
      }
      i++;
    }
    reg[7] = i;
  }

  return 0;
}

int *inthandler0d(int *esp) {
  struct TASK *task = task_now();
  struct CONSOLE *cons = task->cons;
  char s[30];
  cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
  sprintf(s, "EIP = %08x\n", esp[11]);
  cons_putstr0(cons, s);
  return &(task->tss.esp0);
}

int *inthandler0c(int *esp) {
  struct TASK *task = task_now();
  struct CONSOLE *cons = task->cons;
  char s[30];
  cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
  sprintf(s, "EIP = %08X\n", esp[11]);
  cons_putstr0(cons, s);
  return &(task->tss.esp0);
}

void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col) {
  int i, x, y, len, dx, dy;

  dx = x1 - x0;
  dy = y1 - y0;
  x = x0 << 10;
  y = y0 << 10;
  if (dx < 0) {
    dx = -dx;
  }
  if (dy < 0) {
    dy = -dy;
  }
  if (dx >= dy) {
    len = dx + 1;
    if (x0 > x1) {
      dx = -1024;
    } else {
      dx = 1024;
    }
    if (y0 <= y1) {
      dy = ((y1 - y0 + 1) << 10) / len;
    } else {
      dy = ((y1 - y0 - 1) << 10) / len;
    }
  } else {
    len = dy + 1;
    if (y0 > y1) {
      dy = -1024;
    } else {
      dy = 1024;
    }
    if (x0 <= x1) {
      dx = ((x1 - x0 + 1) << 10) / len;
    } else {
      dx = ((x1 - x0 - 1) << 10) / len;
    }
  }
  for (i = 0; i < len; i++) {
    sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
    x += dx;
    y += dy;
  }
  return;
}
