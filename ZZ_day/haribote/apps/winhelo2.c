#include "../apilib/apilib.h"

void HariMain(void) {
  int win, i;
  char buf[150 * 50];
  api_putstr0("Making Window...");

  win = api_openwin(buf, 150, 50, -1, "hello");
  api_boxfilwin(win, 8, 36, 141, 43, 3 /* yellow */);
  api_putstrwin(win, 28, 28, 0 /* black */, 12, "Hello, world");
  for (;;) {
    if (api_getkey(1) == 0x0a) {
      break;
    }
  }
  api_end();
}