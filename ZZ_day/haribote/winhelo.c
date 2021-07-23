#include "apilib.h"

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