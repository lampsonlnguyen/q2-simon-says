/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DIAMOND_SAYS_H
#define DIAMOND_SAYS_H
enum { WIDTH=375, HEIGHT=320, MAX_SEQUENCE=128 };
typedef unsigned int u32;
enum phase { READY, SHOW, INPUT, LOST, PAUSED, FINISHED };
struct app {
    unsigned char sequence[MAX_SEQUENCE];
    unsigned rng, timer, length, shown, entered, score, best;
    int phase, previous_phase, selected, lit, wheel, quit, save_pending;
    int touch_x, touch_y, touching;
};
int app_init(struct app *a, unsigned long best);
int app_input(struct app *a, const char *message);
void app_frame(struct app *a, u32 *page, unsigned stride, unsigned elapsed_ms);
int app_animated(const struct app *a);
#endif
