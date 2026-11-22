/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DIAMOND_SAYS_H
#define DIAMOND_SAYS_H
enum { WIDTH=375, HEIGHT=320, MAX_SEQUENCE=128, POP_ROW=32 };
typedef unsigned int u32;
enum phase { READY, SHOW, INPUT, LOST, PAUSED, FINISHED, CELEBRATE };
/* Q2_POPUP's styles; OWN is the game's look on firmware that sends none. */
enum style { OWN, MINIMAL, MODERN, CLASSIC };
enum { MENU_CONTINUE, MENU_SOUND, MENU_QUIT, MENU_ROWS };
struct theme {
 int style;
 u32 bg,panel,box,frame,title,text,muted,pick,pick_text,head[POP_ROW],band[POP_ROW];
};
struct app {
 unsigned char sequence[MAX_SEQUENCE], shine[4];
 unsigned rng,timer,length,shown,entered,score,best,tick;
 int phase,previous_phase,selected,lit,wheel,quit,save_pending,record,choice;
 int sound_event,sound_pending,sounds,dark_mode;
 struct theme theme;
};
int app_init(struct app *,unsigned long);
/* The theme from Q2_POPUP's value (0: the game's own, by dark_mode). */
void app_theme(struct app *,const char *);
int app_input(struct app *,const char *);
void app_frame(struct app *,u32 *,unsigned,unsigned);
int app_animated(const struct app *);
#endif
