#include "apilib.h"

int rand(void);

void HariMain(void) {
  int i, timer;
  timer = api_alloctimer();
  api_inittimer(timer, 128);
  for (i = 20000000; i >= 20000; i -= i / 100) {
    // 20KHz〜20Hz
    api_beep(i);
    api_settimer(timer, 1);
    if (api_getkey(1) != 128) {
      break;
    }
  }
  api_beep(0);
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