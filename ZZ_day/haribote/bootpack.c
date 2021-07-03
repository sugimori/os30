#include "bootpack.h"

extern struct TIMERCTL timerctl;
extern struct TIMER *task_timer;
#define KEYCMD_LED 0xed
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);
struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal);
void close_console(struct SHEET *sht);

void HariMain(void) {
  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  struct FIFO32 fifo, keycmd;
  int fifobuf[128], keycmd_buf[32], *cons_fifo[2];
  char s[40], mcursor[256];
  int mx, my, i;
  int new_mx = -1, new_my = 0;          // マウスの新座標
  int new_wx = 0x7fffffff, new_wy = 0;  // ウインドのｎ新座標
  unsigned char mouse_dbuf[3], mouse_phase;
  struct MOUSE_DEC mdec;
  // memory関連宣言
  unsigned int memtotal;
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  // sheet関連
  struct SHTCTL *shtctl;
  struct SHEET *sht_back, *sht_mouse;
  unsigned char *buf_back, buf_mouse[256], *buf_cons[2];
  //マルチタスク
  struct TASK *task_a, *task_cons[2], *task;
  // コンソール
  struct CONSOLE *cons;
  // ウインドウ
  int j, x, y, mmx = -1, mmy = -1, mmx2 = 0;
  struct SHEET *sht = 0;
  struct SHEET *key_win;  // 入力状のコンソール

  init_gdtidt();
  init_pic();
  io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
  // FIFO初期化
  fifo32_init(&fifo, 32, fifobuf, 0);
  fifo32_init(&keycmd, 32, keycmd_buf, 0);
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);
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
  *((int *)0x0fe4) = (int)shtctl;
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 0);

  // sht_back
  sht_back = sheet_alloc(shtctl);
  buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);  // 透明色なし
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);                // 背景初期化

  // sht_cons
  key_win = open_console(shtctl, memtotal);

  // mouse
  sht_mouse = sheet_alloc(shtctl);
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);  // 透明番号99？
  init_mouse_cursor8(buf_mouse, 99);               // マウス初期化
  mx = (binfo->scrnx - 16) / 2;                    /* 画面中央になるように座標計算 */
  my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(sht_back, 0, 0);  // 背景の位置を設定
  sheet_slide(key_win, 56, 46);
  sheet_slide(sht_mouse, mx, my);
  sheet_updown(sht_back, 0);  // 背景は０固定？
  sheet_updown(key_win, 1);
  sheet_updown(sht_mouse, 2);
  keywin_on(key_win);

  // http://oswiki.osask.jp/?%28AT%29keyboard
  static char keytable0[0x80] = {
      0,   0,   '1', '2',  '3', '4', '5', '6', '7', '8', '9', '0', '-',  '^',  0x08, 0,    // 00-0F
      'Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0,    'A',  'S',  // 10-1F
      'D', 'F', 'G', 'H',  'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z',  'X',  'C',  'V',  // 20-2F
      'B', 'N', 'M', ',',  '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,    0,    0,    0,    // 30-3F
      0,   0,   0,   0,    0,   0,   0,   '7', '8', '9', '-', '4', '5',  '6',  '+',  '1',  // 40-4F
      '2', '3', '0', '.',  0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    // 50-5F
      0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    // 60-6F
      0,   0,   0,   0x5c, 0,   0,   0,   0,   0,   0,   0,   0,   0,    0x5c, 0,    0,    // 70-7F
  };
  static char keytable1[0x80] = {
      0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=',  '~', 0x08, 0,    // 00-0F
      'Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I', 'O',  'P', '`', '{', 0x0a, 0,   'A',  'S',  // 10-1F
      'D', 'F', 'G', 'H',  'J', 'K', 'L', '+', '*',  0,   0,   '}', 'Z',  'X', 'C',  'V',  // 20-2F
      'B', 'N', 'M', '<',  '>', '?', 0,   '*', 0,    ' ', 0,   0,   0,    0,   0,    0,    // 30-3F
      0,   0,   0,   0,    0,   0,   0,   '7', '8',  '9', '-', '4', '5',  '6', '+',  '1',  // 40-4F
      '2', '3', '0', '.',  0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,    0,    // 50-5F
      0,   0,   0,   0,    0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,    0,    // 60-6F
      0,   0,   0,   '_',  0,   0,   0,   0,   0,    0,   0,   0,   0,    '|', 0,    0,    // 70-7F
  };
  int key_shift = 0;
  int key_leds = (binfo->leds >> 4) & 7;  // 1: ScrollLock, 2: NumLock, 4: CapsLock
  int keycmd_wait = -1;

  fifo32_put(&keycmd, KEYCMD_LED);
  fifo32_put(&keycmd, key_leds);

  *((int *)0x0fec) = (int)&fifo;

  for (;;) {
    if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
      // キーボードコントローラーに送るものがあれば送る
      keycmd_wait = fifo32_get(&keycmd);
      wait_KBC_sendready();
      io_out8(PORT_KEYDAT, keycmd_wait);
    }
    io_cli();  // 割り込み禁止
    if (fifo32_status(&fifo) == 0) {
      // FIFOが空っぽになったので、保留している描画があれば実行する
      if (new_mx >= 0) {
        io_sti();
        sheet_slide(sht_mouse, new_mx, new_my);  // マウのの移動
        new_mx = -1;
      } else if (new_wx != 0x7fffffff) {
        io_sti();
        sheet_slide(sht, new_wx, new_wy);  // シーのの移動
        new_wx = 0x7fffffff;
      } else {
        task_sleep(task_a);
        io_sti();  // HLTを削除
      }
    } else {
      i = fifo32_get(&fifo);
      io_sti();                                   // 割り込み開始
      if (key_win != 0 && key_win->flags == 0) {  // 入うウインドウが閉じられた
        if (shtctl->top == 1) {                   // マウスと背景しかない
          key_win = 0;
        } else {
          key_win = shtctl->sheets[shtctl->top - 1];
          keywin_on(key_win);
        }
      }

      if (256 <= i && i <= 511) {  // キーボード
        if (i - 256 < 0x80) {      // キーコードを文字コードに変換
          if (key_shift == 0) {
            s[0] = keytable0[i - 256];
          } else {
            s[0] = keytable1[i - 256];
          }
        } else {
          s[0] = 0;
        }
        if ('A' <= s[0] && s[0] <= 'Z') {                    // 入力がアルファベット
          if (((key_leds & 4) == 0 && key_shift == 0)        // CapsLock=OFF & Shift=OFF
              || ((key_leds & 4) != 0 && key_shift != 0)) {  // CapsLock=ON & Shift=ON
            s[0] += 0x20;                                    // 大文字を小文字に変換
          }
        }

        if (s[0] != 0 && key_win != 0) {  // 通常文字、バックスペース、Enter
          fifo32_put(&key_win->task->fifo, s[0] + 256);
        }
        if (i - 256 == 0x0f && key_win != 0) {  // TAB
          keywin_off(key_win);
          j = key_win->height - 1;
          if (j == 0) {
            j = shtctl->top - 1;
          }
          key_win = shtctl->sheets[j];
          keywin_on(key_win);
        }
        if (i - 256 == 0x2a) {  // 左SHIFT ON
          key_shift |= 1;
        }
        if (i - 256 == 0x36) {  // 右SHIFT ON
          key_shift |= 2;
        }
        if (i - 256 == 0xaa) {  // 左SHIFT OFF
          key_shift &= ~1;
        }
        if (i - 256 == 0xb6) {  // 右SHIFT OFF
          key_shift &= ~2;
        }
        if (i - 256 == 0x3a) {  // CapsLock
          key_leds ^= 4;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        if (i - 256 == 0x45) {  // NumLock
          key_leds ^= 2;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        if (i - 256 == 0x46) {  // ScrollLock
          key_leds ^= 1;
          fifo32_put(&keycmd, KEYCMD_LED);
          fifo32_put(&keycmd, key_leds);
        }
        if (i - 256 == 0x57 && shtctl->top > 2) {  // F11
          sheet_updown(shtctl->sheets[1], shtctl->top - 1);
        }
        if (i - 256 == 0xfa) {  // キーボードが無事デーをう受け取った
          keycmd_wait = -1;
        }
        if (i - 256 == 0xfe) {  // キーボードが無事デーをう受け取れなかった
          wait_KBC_sendready();
          io_out8(PORT_KEYDAT, keycmd_wait);
        }
        if (i - 256 == 0x3b && key_shift != 0 && key_win != 0) {
          task = key_win->task;
          if (task != 0 && task->tss.ss0 != 0) {  // Shift+F1
            cons_putstr0(task->cons, "\nBreak(key) :\n");
            io_cli();  // 割り込み中止
            task->tss.eax = (int)&(task->tss.esp0);
            task->tss.eip = (int)asm_end_app;
            io_sti();
          }
        }
        if (i - 256 == 0x3c && key_shift != 0) {  // Shift-F2
          if (key_win != 0) {
            keywin_off(key_win);
          }
          key_win = open_console(shtctl, memtotal);
          sheet_slide(key_win, 32, 24);
          sheet_updown(key_win, shtctl->top);
          // 新しく作っこコンソールを入力選択状ににする
          keywin_on(key_win);
        }
      } else if (512 <= i && i <= 767) {  // マウス
        if (mouse_decode(&mdec, i - 512) != 0) {
          // マウスカーソルの移動
          mx += mdec.x;
          my += mdec.y;
          if (mx < 0) mx = 0;
          if (my < 0) my = 0;
          if (mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
          if (my > binfo->scrny - 1) my = binfo->scrny - 1;
          // sprintf(s, "(%d, %d)", mx, my);
          // putfonts8_asc_sht(sht_back,0,0,COL8_FFFFFF,COL8_008484,s,12);
          // sheet_slide(sht_mouse, mx, my);
          new_mx = mx;
          new_my = my;
          if ((mdec.btn & 0x01) != 0) {
            // 左ボタンを押している
            if (mmx < 0) {
              // 通常モード
              // 上の下敷きから順番にマウスが指している下敷きを探す
              for (j = shtctl->top - 1; j > 0; j--) {
                sht = shtctl->sheets[j];
                x = mx - sht->vx0;  // シートからの相対座標
                y = my - sht->vy0;  // シートからの相ざ座標
                if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
                  if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                    sheet_updown(sht, shtctl->top - 1);
                    if (sht != key_win) {
                      keywin_off(key_win);
                      key_win = sht;
                      keywin_on(key_win);
                    }
                    if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {  // タイトル部分
                      mmx = mx;                                               // ウインドウ移動モード
                      mmy = my;
                      mmx2 = sht->vx0;
                      new_wy = sht->vy0;
                    }
                    if (3 + 10 <= x && x < 3 + 10 + 15 && 5 <= y && y < 5 + 15) {  // バツボタン
                      if ((sht->flags & 0x10) != 0) {  // アプリが作ったウインドかか？
                        task = sht->task;
                        cons_putstr0(task->cons, "\nBreak(mouse) :\n");
                        io_cli();
                        task->tss.eax = (int)&(task->tss.esp0);
                        task->tss.eip = (int)asm_end_app;
                        io_sti();
                      }
                    }
                    break;
                  }
                }
              }
            } else {
              // ウインドウ移動モード
              x = mx - mmx;
              y = my - mmy;
              new_wx = (mmx2 + x + 2) & ~3;
              new_wy = new_wy + y;
              mmy = my;
            }
          } else {
            // 左ボタンを押していない
            mmx = -1;
            if (new_wx != 0x7fffffff) {
              sheet_slide(sht, new_wx, new_wy);
              new_wx = 0x7fffffff;
            }
          }
        }
      } else if (768 <= i && i <= 1023) {  // コンソール終了処理
        close_console(shtctl->sheets0 + (i - 768));
      }
    }
  }
  return;
}

void keywin_off(struct SHEET *key_win) {
  change_wtitle8(key_win, 0);
  if ((key_win->flags & 0x20) != 0) {
    fifo32_put(&key_win->task->fifo, 3);  // コンソールのカーソルOFF
  }
  return;
}

void keywin_on(struct SHEET *key_win) {
  change_wtitle8(key_win, 1);
  if ((key_win->flags & 0x20) != 0) {
    fifo32_put(&key_win->task->fifo, 2);  // コンソーののカーソルON
  }
  return;
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

  struct SHEET *sht = sheet_alloc(shtctl);
  unsigned char *buf = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
  struct TASK *task = task_alloc();
  int *cons_fifo = (int *)memman_alloc_4k(memman, 128 * 4);
  sheet_setbuf(sht, buf, 256, 165, -1);  // 透明色なし
  sht->title = "console";
  make_window8(buf, 256, 165, sht->title, 0);
  make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
  task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
  task->tss.esp = task->cons_stack + 64 * 1024 - 12;
  task->tss.eip = (int)&console_task;
  task->tss.es = 1 * 8;
  task->tss.cs = 2 * 8;
  task->tss.ss = 1 * 8;
  task->tss.ds = 1 * 8;
  task->tss.fs = 1 * 8;
  task->tss.gs = 1 * 8;
  *((int *)(task->tss.esp + 4)) = (int)sht;
  *((int *)(task->tss.esp + 8)) = memtotal;
  task_run(task, 2, 2);  // level=2, priority=2
  sht->task = task;
  sht->flags |= 0x20;  // カーソルあり
  fifo32_init(&task->fifo, 128, cons_fifo, task);
  return sht;
}

void close_constask(struct TASK *task) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  task_sleep(task);
  memman_free_4k(memman, task->cons_stack, 64 * 1024);
  memman_free_4k(memman, (int)task->fifo.buf, 128 * 4);
  task->flags = 0;  // task_free(task)の代わり
  return;
}

void close_console(struct SHEET *sht) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct TASK *task = sht->task;
  memman_free_4k(memman, (int)sht->buf, 256 * 165);
  sheet_free(sht);
  close_constask(task);
  return;
}