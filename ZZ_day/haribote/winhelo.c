int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_end(void);
int api_getkey(int mode);
void api_closewin(int win);

char buf[150 * 50];

void HariMain(void) {
  int win, i;
  win = api_openwin(buf, 150, 50, -1, "hello");
  for (;;) {
    i = api_getkey(1);
  }
  api_closewin(win);
  api_end();
}