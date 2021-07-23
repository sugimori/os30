#include "../apilib/apilib.h"

void HariMain(void) {
  char *buf;
  int win, i;

  api_initmalloc();
  buf = api_malloc(150 * 100);

  win = api_openwin(buf, 150, 100, -1, "star1");
  api_boxfilwin(win, 6, 26, 143, 93, 0 /* black */);
  api_point(win, 75, 59, 3 /* yellow */);
  for (;;) {
    i = api_getkey(1);
  }
  api_closewin(win);
  api_end();
}