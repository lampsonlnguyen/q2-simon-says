#include "app.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 struct app a;
 assert(app_init(&a,0));assert(a.phase==READY);
 assert(app_input(&a,"k u 218")); assert(a.phase==SHOW&&a.length==1);
 unsigned pad=a.sequence[0];
 unsigned char initial=a.sequence[0];
 assert(initial<4);
 a.phase=INPUT;
 a.selected=pad;
 assert(app_input(&a,"k u 218"));
 assert(a.score==1&&a.length==2&&a.phase==SHOW&&a.save_pending);
 a.phase=INPUT;a.entered=0;
 a.selected=(a.sequence[0]+1)%4;
 assert(app_input(&a,"k u 218"));
 assert(a.phase==LOST);
 assert(app_input(&a,"k u 218")&&a.phase==SHOW);
 assert(app_input(&a,"k u 170")&&a.phase==PAUSED);
 assert(app_input(&a,"k u 170")&&a.phase==SHOW);
 puts("Diamond Says logic tests passed");
 return 0;
}
