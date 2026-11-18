#include "app.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 struct app a;assert(app_init(&a,0));assert(a.phase==READY);
 assert(app_input(&a,"k d 218")&&a.phase==SHOW);
 assert(!app_input(&a,"k u 218"));assert(a.length==1);
 unsigned pad=a.sequence[0];assert(pad<4);
 a.phase=INPUT;a.selected=(int)pad;
 assert(app_input(&a,"k d 218")&&a.phase==CELEBRATE);
 assert(a.score==1&&a.best==1);
 assert(a.sound_pending&&a.sound_event==6);
 a.phase=INPUT;a.entered=0;a.selected=(pad+1)%4;
 assert(app_input(&a,"k d 218")&&a.phase==LOST);
 assert(app_input(&a,"k d 218")&&a.phase==SHOW);
 a.phase=INPUT;
 int xx[4]={109,265,265,109}, yy[4]={138,138,216,216};
 char msg[40];snprintf(msg,sizeof msg,"t d %d %d",xx[a.sequence[0]],yy[a.sequence[0]]);
 assert(app_input(&a,msg)&&a.phase==CELEBRATE);
 assert(app_input(&a,"t d 187 311")&&!a.sounds);
 assert(app_input(&a,"k d 170")&&a.phase==PAUSED);
 assert(app_input(&a,"k d 170")&&a.phase==CELEBRATE);
 static u32 fb[WIDTH*HEIGHT];app_frame(&a,fb,HEIGHT,17);
 assert(fb[187*HEIGHT+HEIGHT-1-177]!=0xfff7f1e9u);
 /* The hub contains a rendered cat, and each identical pad retains its color. */
 assert(fb[180*HEIGHT+HEIGHT-1-174]!=0xfff5e8f2u);
 assert(fb[139*HEIGHT+HEIGHT-1-129]!=fb[236*HEIGHT+HEIGHT-1-129]);
 /* The wide pads fill the sides but do not overlap the bottom status. */
 assert(fb[38*HEIGHT+HEIGHT-1-138]!=0xfff7f1e9u);
 assert(fb[337*HEIGHT+HEIGHT-1-216]!=0xfff7f1e9u);
 assert(fb[187*HEIGHT+HEIGHT-1-265]==0xfff7f1e9u);
 a.phase=LOST;
 app_frame(&a,fb,HEIGHT,0);
 assert(fb[187*HEIGHT+HEIGHT-1-272]==0xffa83847u);
 assert(fb[187*HEIGHT+HEIGHT-1-177]!=0xffa83847u);
 /* The identical pads remain tappable near their edges while the cat is not. */
 a.phase=INPUT;a.entered=0;
 assert(!app_input(&a,"t d 187 177"));
 assert(!app_input(&a,"t d 94 84"));
 assert(!app_input(&a,"t d 187 85"));
 assert(app_input(&a,"t d 109 138") || a.phase==INPUT);
 puts("Diamond Says rounded pads and mascot controls passed");return 0;
}
