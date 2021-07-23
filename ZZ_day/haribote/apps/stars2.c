#include "../apilib/apilib.h"

int rand(void);

void HariMain(void) {
  char *buf;
  int win, i, x, y;

  api_initmalloc();
  buf = api_malloc(150 * 100);

  win = api_openwin(buf, 150, 100, -1, "stars2");
  api_boxfilwin(win + 1, 6, 26, 143, 93, 0 /* black */);
  for (i = 0; i < 50; i++) {
    x = (rand() % 137) + 6;
    y = (rand() % 67) + 26;
    api_point(win + 1, x, y, 3 /* yellow */);
  }
  api_refreshwin(win, 6, 26, 144, 94);
  for (;;) {
    if (api_getkey(1) == 0x0a) {
      break;
    }
  }
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