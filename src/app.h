/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DIAMOND_SAYS_H
#define DIAMOND_SAYS_H
enum { WIDTH=375, HEIGHT=320, MAX_SEQUENCE=128 };
typedef unsigned int u32;
enum phase { READY, SHOW, INPUT, LOST, PAUSED, FINISHED, CELEBRATE };
struct app {
 unsigned char sequence[MAX_SEQUENCE], shine[4];
 unsigned rng,timer,length,shown,entered,score,best,tick,ripple_age,spark_age,flash_age;
 int phase,previous_phase,selected,lit,wheel,quit,save_pending,ripple_pad;
 int sound_event,sound_pending,sounds,dark_mode;
};
int app_init(struct app *,unsigned long);
int app_input(struct app *,const char *);
void app_frame(struct app *,u32 *,unsigned,unsigned);
int app_animated(const struct app *);
#endif
