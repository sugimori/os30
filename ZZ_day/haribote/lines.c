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
int api_getkey(int mode);

int rand(void);

void HariMain(void) {
  char *buf;
  int win, i;
  api_initmalloc();
  buf = api_malloc(160 * 100);
  win = api_openwin(buf, 160, 100, -1, "lines");
  for (i = 0; i < 8; i++) {
    api_linewin(win + 1, 8, 26, 77, i * 9 + 26, i);
    api_linewin(win + 1, 88, 26, i * 9 + 88, 89, i);
  }
  api_refreshwin(win, 6, 26, 154, 90);
  for (;;) {
    if (api_getkey(1) == 0x0a) {
      break;  // Enter
    }
  }
  api_closewin(win);
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