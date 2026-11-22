#include "app.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static u32 fb[WIDTH*HEIGHT];
static u32 at(int x,int y){return fb[x*HEIGHT+HEIGHT-1-y];}
int main(void){
 struct app a;assert(app_init(&a,0));assert(a.phase==READY);
 assert(app_input(&a,"k d 218")&&a.phase==SHOW);
 assert(!app_input(&a,"k u 218"));assert(a.length==1);
 unsigned pad=a.sequence[0];assert(pad<4);
 a.phase=INPUT;a.selected=(int)pad;
 assert(app_input(&a,"k d 218")&&a.phase==CELEBRATE);
 assert(a.score==1&&a.best==1&&a.record);
 assert(a.sound_pending&&a.sound_event==6);
 a.phase=INPUT;a.entered=0;a.selected=(pad+1)%4;
 assert(app_input(&a,"k d 218")&&a.phase==LOST);
 assert(app_input(&a,"k d 218")&&a.phase==SHOW&&!a.record);
 a.phase=INPUT;
 /* Each pad's centre is tappable. */
 int xx[4]={107,267,267,107}, yy[4]={96,96,207,207};
 char msg[40];snprintf(msg,sizeof msg,"t d %d %d",xx[a.sequence[0]],yy[a.sequence[0]]);
 assert(app_input(&a,msg)&&a.phase==CELEBRATE);

 /* Return pauses on the theme's menu: the wheel picks a row, Centre or a tap takes it. */
 assert(app_input(&a,"k d 170")&&a.phase==PAUSED&&a.choice==MENU_CONTINUE);
 assert(app_input(&a,"w -13")&&a.choice==MENU_SOUND);
 assert(app_input(&a,"k d 218")&&!a.sounds&&a.phase==PAUSED);
 assert(app_input(&a,"w 13")&&a.choice==MENU_CONTINUE);
 assert(!app_input(&a,"w 13")||a.choice==MENU_CONTINUE); /* clamped at the top */
 assert(app_input(&a,"k d 218")&&a.phase==CELEBRATE);
 assert(app_input(&a,"k d 170")&&a.phase==PAUSED);
 int my=(HEIGHT-(38+3*32+8))/2+38;  /* the menu's first row, as popup() places it */
 snprintf(msg,sizeof msg,"t d 187 %d",my+32+16);
 assert(app_input(&a,msg)&&a.sounds);
 assert(!app_input(&a,"t d 5 5")&&a.phase==PAUSED); /* outside the box: nothing */
 assert(app_input(&a,"k d 170")&&a.quit==1);       /* a second Return still exits */

 /* The game's own look: black page, flat board in its hairline frame, the pads in their colours. */
 app_init(&a,0);a.phase=INPUT;
 app_frame(&a,fb,HEIGHT,0);
 assert(at(3,150)==0xff000000u);
 assert(at(24,152)==0xff3a3a3au&&at(40,40)==0xff111111u);
 assert(at(60,70)==0xfff07820u&&at(300,70)==0xffd9bd78u);
 assert(at(60,200)==0xffaeb6c1u&&at(300,200)==0xff33363du); /* Black lifted off the dark board */
 assert(at(187,152)!=at(60,70));                    /* the cat, not a pad */
 /* The wheel's pick is ringed in the theme's ink. */
 assert(at(28,100)==0xffffffffu);
 /* Pads stay tappable to their edges; the hub and board gaps are not. */
 assert(!app_input(&a,"t d 187 152")&&!app_input(&a,"t d 187 30")&&!app_input(&a,"t d 187 270"));
 assert(app_input(&a,"t d 32 100")||a.phase!=INPUT);

 /* A Modern Q2_POPUP: its accent is the pick, and the pause menu's bar is drawn in it. */
 app_init(&a,0);
 app_theme(&a,"modern 1c1c1c 3a3a3a ffffff aaaaaa 2f86f0 ffffff");
 assert(a.theme.style==MODERN&&a.theme.pick==0xff2f86f0u&&a.dark_mode&&a.theme.bg==0xff000000u);
 a.phase=PAUSED;a.previous_phase=INPUT;a.choice=MENU_QUIT;
 app_frame(&a,fb,HEIGHT,0);
 assert(at(70,my+2*32+4)==0xff2f86f0u);              /* Quit's bar */
 assert(at(3,150)==0xff000000u);                       /* the page under a dim stays black */
 assert(at(30,152)==0xff0e0e0eu);                      /* the board (its box), dimmed by half */
 /* Light Mode flips the ink, so the page turns white and the cat greys. */
 app_init(&a,0);
 app_theme(&a,"minimal ffffff b5b5b5 000000 737373 000000 000000");
 assert(a.theme.style==MINIMAL&&!a.dark_mode&&a.theme.bg==0xffffffffu);
 /* Classic: the band's 32 rows and the strip's are read; one missing keeps the game's own. */
 char v[16+2*32*7+6*7+1],*e=v+sprintf(v,"classic");
 for(int i=0;i<6+2*32;++i)e+=sprintf(e," %06x",i==6+32+16?0x0a64dc:0x808080);
 app_init(&a,0);app_theme(&a,v);
 assert(a.theme.style==CLASSIC&&a.theme.band[16]==0xff0a64dcu&&a.theme.bg==a.theme.box);
 v[strlen(v)-7]=0;app_init(&a,0);app_theme(&a,v);
 assert(a.theme.style==OWN);
 app_init(&a,0);app_theme(&a,"modern 1c1c1c 3a3a3a ffffff aaaaaa 2f86f0 ffffff more fields");
 assert(a.theme.style==MODERN);
 app_init(&a,0);app_theme(&a,"glass 1c1c1c 3a3a3a ffffff aaaaaa 2f86f0 ffffff");
 assert(a.theme.style==OWN);

 /* Dark mode recolors the indexed cat sprite without altering game state. */
 app_init(&a,0);a.phase=INPUT;a.dark_mode=0;app_theme(&a,0);
 app_frame(&a,fb,HEIGHT,0);
 u32 light_pixel=at(170,170);
 a.dark_mode=1;app_theme(&a,0);
 app_frame(&a,fb,HEIGHT,0);
 assert(light_pixel!=at(170,170)&&a.phase==INPUT);
 puts("Diamond Says theme, pop-ups, sprite and controls passed");return 0;
}
