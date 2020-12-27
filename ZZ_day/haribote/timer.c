#include "bootpack.h"
#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040
struct TIMERCTL timerctl;
#define TIMER_FLAGS_ALLOC   1 /* 確保済 */
#define TIMER_FLAGS_USING   2 /* タイマ作動中 */

void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    timerctl.count = 0;
    timerctl.next_time = 0xffffffff;
    timerctl.using = 0;
    for(i =0;i<MAX_TIMER;i++) {
        timerctl.timers0[i].flags = 0; /* 未使用 */
    }
    return;
}

struct TIMER *timer_alloc(void)
{
    int i;
    for(i=0;i<MAX_TIMER;i++) {
        if(timerctl.timers0[i].flags == 0) {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer) 
{
    timer->flags = 0;
    return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data) 
{
    timer->fifo = fifo;
    timer->data = data;
    return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    int e;
    struct TIMER *next_t, *prev_t;
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    e = io_load_eflags();
    io_cli();
    timerctl.using++;
    if(timerctl.using == 1) {
        // 動作中のタイマは１つ
        timerctl.t0 = timer;
        timer->next_timer = 0; // 次はない
        timerctl.next_time = timer->timeout;
        io_store_eflags(e);
        return;
    }
    next_t = timerctl.t0;
    if(timer->timeout <= next_t->timeout) {
        // 先頭に入れる場合
        timerctl.t0 = timer;
        timer->next_timer = next_t; // 次はt
        timerctl.next_time = timer->timeout;
        io_store_eflags(e);
        return;
    }
    // どこに入れればいいか探す
    for(;;) {
        prev_t = next_t;
        next_t = next_t->next_timer;
        if(next_t== 0) {
            break; // 一番うしろ
        }
        if(timer->timeout <= next_t->timeout) {
            // sとtの間に入れる
            prev_t->next_timer = timer; // sの次はtimer
            timer->next_timer = next_t; // timerの次はt
            io_store_eflags(e);
            return;
        }        
    }
    // 一番うしろ
    prev_t->next_timer = timer;
    timer->next_timer = 0; // ない
    io_store_eflags(e);
    return;
}

void inthandler20(int *esp)
{
    int i;
    struct TIMER *timer;
    io_out8(PIC0_OCW2, 0x60); /* IRQ-00受付完了をPICに通知 */
    timerctl.count++;
    if(timerctl.next_time > timerctl.count) {
        return;
    }
    timer = timerctl.t0;
    for(i=0;i<timerctl.using;i++) {
        if(timer->timeout > timerctl.count) {
            break;
        }
        // タイムアウト
        timer->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo,timer->data);
        timer = timer->next_timer; // 次のタイマーをtimer に代入
    }
    timerctl.using -= 1;
    // ずらし
    timerctl.t0 = timer; // 残りのタイマの先頭をtimersの最初にセット
    // timerctl.nextの設定
    if(timerctl.using > 0) {
        timerctl.next_time = timerctl.t0->timeout;
    } else {
        timerctl.next_time = 0xffffffff;
    }
    return;
}
