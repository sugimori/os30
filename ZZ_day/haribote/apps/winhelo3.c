#include "../apilib/apilib.h"

void HariMain(void) {
  char *buf;
  int win, i;

  api_initmalloc();
  buf = api_malloc(150 * 50);
  win = api_openwin(buf, 150, 50, -1, "hello2");
  api_boxfilwin(win, 8, 36, 141, 43, 6 /* yellow */);
  api_putstrwin(win, 28, 28, 0 /* black */, 13, "Hello, world2");
  for (;;) {
    i = api_getkey(1);
  }
  api_closewin(win);
  api_end();
}