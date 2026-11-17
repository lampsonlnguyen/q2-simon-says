#include "app.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 struct app a;
 assert(app_init(&a,0));assert(a.phase==READY);
 /* Press triggers on key-down and release does not repeat it. */
 assert(app_input(&a,"k d 218"));assert(a.phase==SHOW&&a.length==1);
 assert(!app_input(&a,"k u 218"));assert(a.length==1);
 unsigned pad=a.sequence[0];
 assert(pad<4);
 a.phase=INPUT;a.selected=(int)pad;
 assert(app_input(&a,"k d 218"));
 assert(a.score==1&&a.length==2&&a.phase==SHOW&&a.save_pending);
 assert(!app_input(&a,"k u 218"));
 /* Wheel selects a pad; Center confirms the choice. */
 a.phase=INPUT;a.entered=0;a.selected=(int)((a.sequence[0]+1)%4);
 assert(app_input(&a,"w 13"));assert(a.selected==(int)a.sequence[0]);
 assert(app_input(&a,"k d 218"));assert(a.score==2);
 /* A wrong selection ends the run. */
 a.phase=INPUT;a.entered=0;
 a.selected=(a.sequence[0]+1)%4;
 assert(app_input(&a,"k d 218"));assert(a.phase==LOST);
 /* Touch-down can restart, without needing touch-up. */
 assert(app_input(&a,"t d 120 140"));assert(a.phase==SHOW);
 assert(!app_input(&a,"t u 120 140"));
 a.phase=INPUT;a.selected=0;
 /* Touch directly selects the correct pad, not the wheel selection. */
 int x=67+(a.sequence[0]%2)*123+25,y=116+(a.sequence[0]/2)*70+25;
 char msg[32];snprintf(msg,sizeof msg,"t d %d %d",x,y);
 assert(app_input(&a,msg));assert(a.score==1);
 /* Touch in the gap between pads is ignored. */
 a.phase=INPUT;a.entered=0;
 assert(!app_input(&a,"t d 184 180"));
 assert(app_input(&a,"k d 170")&&a.phase==PAUSED);
 assert(!app_input(&a,"k u 170"));
 assert(app_input(&a,"k d 170")&&a.phase==INPUT);
 puts("Diamond Says input and game integration tests passed");
 return 0;
}
