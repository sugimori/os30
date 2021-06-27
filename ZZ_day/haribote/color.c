int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_putstr0(char *s);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_closewin(int win);
void api_end(void);
void api_freetimer(int timer);
void api_settimer(int timer, int time);
void api_inittimer(int timer, int data);
int api_alloctimer(void);
int api_getkey(int mode);
int sprintf(char *str, char *fmt, ...);
void api_beep(int tone);

int rand(void);

void HariMain(void) {
  char *buf;
  int win, x, y, r, g, b;
  api_initmalloc();
  buf = api_malloc(144 * 164);
  win = api_openwin(buf, 144, 164, -1, "color");
  for (y = 0; y < 128; y++) {
    for (x = 0; x < 128; x++) {
      r = x * 2;
      g = y * 2;
      b = 0;
      buf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
    }
  }
  api_refreshwin(win, 8, 28, 136, 156);
  api_getkey(1);
  api_end();
}

static int X = 1;

int rand(void) {
  //
  int A = 1664525;
  int B = 1013904223;
  int M = 2147483647;

  X = (A * X + B) % M;
  return X;
}