/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler0d(void);
void asm_inthandler0c(void);
void asm_cons_putchar(void);
void asm_hrb_api(void);
void asm_end_app(void);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void taskswitch4(void);
void taskswitch3(void);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);

/* asmhead.nas */
struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
};
#define ADR_BOOTINFO 0x00000ff0
#define ADR_DISKIMG 0x00100000

/* sprintf.c */
int sprintf(char *str, char *fmt, ...);
/* strcmp.c */
int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, int n);
int strlen(char *s);

/* dsctbl.c */
struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;    // 2 2
  char base_mid, access_right;  // 1 1
  char limit_high, base_high;   // 1 1
};

struct GATE_DESCRIPTOR {
  short offset_low, selector;   // 2 2
  char dw_count, access_right;  // 1 1
  short offset_high;            // 2
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_TSS32 0x0089
#define AR_INTGATE32 0x008e

/* int.c */
void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

/* fifo.c */
struct FIFO8 {
  unsigned char *buf;
  int p, q, size, free, flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);
struct FIFO32 {
  int *buf;
  int p, q, size, free, flags;
  struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/* keyboard.c */

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);
void inthandler21(int *esp);

/* mouse.c */
struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void inthandler2c(int *esp);

/* memory.c */

#define MEMMAN_ADDR 0x003c0000
#define MEMMAN_FREES 4090
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

struct FREEINFO {
  /* data */
  unsigned int addr, size;
};

struct MEMMAN {
  /* data */
  int frees, maxfrees, lostsize, losts;
  struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* sheet.c */
struct SHEET {
  unsigned char *buf;
  int bxsize, bysize, vx0, vy0, col_inv, height, flags;
  struct SHTCTL *ctl;
  struct TASK *task;
  char *title;
};

#define MAX_SHEETS 256

struct SHTCTL {
  unsigned char *vram, *map;
  int xsize, ysize, top;
  struct SHEET *sheets[MAX_SHEETS];
  struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(unsigned char *vram, int xsize, int ysize);
void putfont8(unsigned char *vram, int xsize, int x, int y, char color, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char color, unsigned char *s);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int color, int backcolor, char *s, int length);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
void init_mouse_cursor8(char *mouse, char bc);

/* カラーパレットの定義 */
#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

/* timer.c */
void init_pit(void);
void inthandler20(int *esp);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
int timer_cancel(struct TIMER *timer);
void timer_cancelall(struct FIFO32 *fifo);

#define MAX_TIMER 500

struct TIMER {
  struct TIMER *next_timer;
  unsigned int timeout;
  char flags, flags2;
  struct FIFO32 *fifo;
  int data;
};
struct TIMERCTL {
  unsigned int count, next_time, using;
  struct TIMER *t0;  // 使用中
  struct TIMER timers0[MAX_TIMER];
};

#define MAX_TASKS 1000  // 最大タスク数
#define TASK_GDT0 3     // TSSをGDTの何番から割り当てるか
#define MAX_TASKS_LV 100
#define MAX_TASKLEVELS 10
struct TSS32 {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
  int sel;  // GDTの番号
  int flags;
  int level, priority;
  struct FIFO32 fifo;
  struct TSS32 tss;
  struct CONSOLE *cons;
  int ds_base;
};

struct TASKLEVEL {
  int running;  // 動作しているタスクの数
  int now;      // 現在動作しているタスク
  struct TASK *tasks[MAX_TASKS_LV];
};

struct TASKCTL {
  int now_lv;      // 現在動作中のレベル
  char lv_change;  // 次回タスクスイッチのときに、レベルも変えた方がいいかどうか
  struct TASKLEVEL level[MAX_TASKLEVELS];
  struct TASK task0[MAX_TASKS];
};

/* mtask.c */
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int level, int priority);
void task_switch(void);
void task_sleep(struct TASK *task);
struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchsub(void);
void task_idle(void);

/* file.c */
struct FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
};
void file_raedfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);

/* console.c */
struct CONSOLE {
  struct SHEET *sht;
  int cur_x, cur_y, cur_c;
  struct TIMER *timer;
};
void console_task(struct SHEET *sheet, unsigned int memtotal);
void cons_putstr0(struct CONSOLE *cons, char *s);

/* window.c */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void change_wtitle8(struct SHEET *sht, char act);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
