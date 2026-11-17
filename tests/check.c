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
 a.phase=INPUT;a.entered=0;a.selected=(pad+1)%4;
 assert(app_input(&a,"k d 218")&&a.phase==LOST);
 assert(app_input(&a,"k d 218")&&a.phase==SHOW);
 a.phase=INPUT;
 int xx[4]={187,257,187,117}, yy[4]={110,173,236,173};
 char msg[40];snprintf(msg,sizeof msg,"t d %d %d",xx[a.sequence[0]],yy[a.sequence[0]]);
 assert(app_input(&a,msg)&&a.phase==CELEBRATE);
 assert(app_input(&a,"t d 345 42")&&!a.sounds);
 assert(app_input(&a,"k d 170")&&a.phase==PAUSED);
 assert(app_input(&a,"k d 170")&&a.phase==CELEBRATE);
 static u32 fb[WIDTH*HEIGHT];app_frame(&a,fb,HEIGHT,17);
 puts("Diamond Says effects and controls passed");return 0;
}
