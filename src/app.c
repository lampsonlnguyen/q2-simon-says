/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026. Diamond Says for Q2 Pod.
 * Framebuffer effects use fixed point / integer arithmetic.
 */
#include "app.h"
enum { CENTRE=218,RETURN=170,WHEEL_STEP=13 };
static const u32 colors[4]={0xfff07820u,0xffd9bd78u,0xff17191du,0xffaeb6c1u};
static const u32 bright[4]={0xffffc078u,0xffffe5a4u,0xff7a8290u,0xfff5f8ffu};
static const int cx[4]={187,257,187,117},cy[4]={110,173,236,173};
static const unsigned char font[36][7]={
{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
{1,1,1,1,17,17,14},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
{17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,30,1,1,17,14},
{6,8,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
{14,17,17,15,1,2,12}
};

static unsigned rand32(struct app *a){a->rng^=a->rng<<13;a->rng^=a->rng>>17;a->rng^=a->rng<<5;return a->rng;}
static void add(struct app *a){if(a->length<MAX_SEQUENCE)a->sequence[a->length++]=(unsigned char)(rand32(a)&3u);}
static void signal(struct app *a,int s){if(a->sounds){a->sound_event=s;a->sound_pending=1;}}
static void pulse(struct app *a,int n){a->shine[n]=255;a->ripple_pad=n;a->ripple_age=1;signal(a,n+1);}
static unsigned tempo(struct app *a){unsigned t=460-a->length*12;return t<170?170:t;}
static void begin(struct app *a){
 a->rng^=a->tick+0x9e3779b9u;if(!a->rng)a->rng=0x12345678u;
 a->length=a->shown=a->entered=a->score=0;a->selected=0;a->lit=-1;
 a->ripple_age=a->spark_age=a->flash_age=0;
 for(int i=0;i<4;++i)a->shine[i]=0;
 add(a);a->phase=SHOW;a->timer=500;
}
int app_init(struct app *a,unsigned long best){
 *a=(struct app){0};a->rng=0x92c347e1u;a->best=best<=MAX_SEQUENCE?best:0;
 a->phase=READY;a->lit=-1;a->ripple_pad=-1;a->sounds=1;return 1;
}
static void choose(struct app *a,int n){
 if(a->phase!=INPUT||n<0||n>3)return;
 a->selected=n;pulse(a,n);
 if(a->sequence[a->entered]!=(unsigned)n){
  a->phase=LOST;a->flash_age=620;signal(a,5);return;
 }
 ++a->entered;
 if(a->entered==a->length){
  a->score=a->length;if(a->score>a->best){a->best=a->score;a->save_pending=1;}
  a->phase=a->length==MAX_SEQUENCE?FINISHED:CELEBRATE;
  a->timer=900;a->spark_age=850;a->flash_age=500;signal(a,6);
 }
}
static void action(struct app *a){
 if(a->phase==READY||a->phase==LOST||a->phase==FINISHED)begin(a);
 else if(a->phase==PAUSED)a->phase=a->previous_phase;
 else if(a->phase==INPUT)choose(a,a->selected);
}
static int number(const char **p){
 int v=0,neg=0;while(**p==' ')++*p;
 if(**p=='-'){neg=1;++*p;}
 if(**p<'0'||**p>'9')return 10000;
 while(**p>='0'&&**p<='9'){if(v>1000)return 10000;v=v*10+*(*p)++-'0';}
 return neg?-v:v;
}
static int hit(int x,int y){
 for(int i=0;i<4;++i){
  int dx=x-cx[i],dy=y-cy[i];if(dx<0)dx=-dx;if(dy<0)dy=-dy;
  if(dx*39+dy*49<=49*39)return i;
 }
 return -1;
}
int app_input(struct app *a,const char *s){
 if(!s[0]||s[1]!=' ')return 0;
 if(s[0]=='w'){
  const char *p=s+2;int v=number(&p);
  if(v< -2000||v>2000)return 0;
  a->wheel-=v;int steps=a->wheel/WHEEL_STEP;a->wheel%=WHEEL_STEP;
  if(a->phase==INPUT&&steps)a->selected=(a->selected+steps%4+4)%4;
  return steps!=0;
 }
 if(!s[2]||s[3]!=' ')return 0;
 const char *p=s+4;int x=number(&p);
 if(s[0]=='k'){
  if(s[2]!='d')return 0;
  if(x==CENTRE){action(a);return 1;}
  if(x==RETURN){
   if(a->phase==PAUSED)a->phase=a->previous_phase;
   else if(a->phase==READY||a->phase==LOST||a->phase==FINISHED)a->quit=1;
   else {a->previous_phase=a->phase;a->phase=PAUSED;}
   return 1;
  }
  return 0;
 }
 if(s[0]=='t'&&s[2]=='d'){
  int y=number(&p);if(x<0||x>=WIDTH||y<0||y>=HEIGHT)return 0;
  if(x>=305&&y<65){a->sounds=!a->sounds;return 1;}
  if(a->phase!=INPUT){action(a);return 1;}
  int n=hit(x,y);if(n>=0){choose(a,n);return 1;}
 }
 return 0;
}
static void step(struct app *a,unsigned dt){
 a->tick+=dt;
 for(int i=0;i<4;++i){
  unsigned f=dt*255/360;a->shine[i]=(unsigned char)(f>=a->shine[i]?0:a->shine[i]-f);
 }
 if(a->ripple_age){a->ripple_age+=dt;if(a->ripple_age>430)a->ripple_age=0;}
 if(a->spark_age)a->spark_age=dt>=a->spark_age?0:a->spark_age-dt;
 if(a->flash_age)a->flash_age=dt>=a->flash_age?0:a->flash_age-dt;
 if(a->phase!=SHOW&&a->phase!=CELEBRATE)return;
 while(dt>=a->timer){
  dt-=a->timer;a->timer=0;
  if(a->phase==CELEBRATE){
   add(a);a->shown=a->entered=0;a->phase=SHOW;a->lit=-1;a->timer=420;break;
  }
  if(a->lit>=0){a->lit=-1;a->timer=165;++a->shown;}
  else if(a->shown<a->length){a->lit=a->sequence[a->shown];pulse(a,a->lit);a->timer=tempo(a);}
  else {a->phase=INPUT;a->lit=-1;return;}
 }
 if(a->timer>dt)a->timer-=dt;
}
static u32 mix(u32 a,u32 b,unsigned v){
 if(v>255)v=255;
 unsigned u=255-v;
 unsigned r=(((a>>16)&255)*u+((b>>16)&255)*v)/255;
 unsigned g=(((a>>8)&255)*u+((b>>8)&255)*v)/255;
 unsigned blue=((a&255)*u+(b&255)*v)/255;
 return 0xff000000u|(r<<16)|(g<<8)|blue;
}
static void box(u32 *p,unsigned st,int x,int y,int w,int h,u32 c){
 if(x<0||y<0||x+w>WIDTH||y+h>HEIGHT)return;
 for(int i=x;i<x+w;++i)for(int j=y;j<y+h;++j)p[i*st+HEIGHT-1-j]=c;
}
static void label(u32 *p,unsigned st,const char *s,int y,int z,u32 c){
 int n=0;while(s[n])++n;int x=(WIDTH-(n*6-1)*z)/2;
 for(;*s;++s,x+=6*z){
  int g=(*s>='A'&&*s<='Z')?*s-'A':(*s>='0'&&*s<='9')?*s-'0'+26:-1;
  if(g<0)continue;
  for(int r=0;r<7;++r)for(int col=0;col<5;++col)
   if(font[g][r]&(16u>>col))box(p,st,x+col*z,y+r*z,z,z,c);
 }
}
static void numeral(u32 *p,unsigned st,unsigned v,int y,int z){
 char buf[4],rev[4];int n=0;
 do{rev[n++]=(char)('0'+v%10);v/=10;}while(v&&n<3);
 for(int i=0;i<n;++i)buf[i]=rev[n-1-i];
 buf[n]=0;
 label(p,st,buf,y,z,0xfff0dfb5u);
}
static void diamond(u32 *p,unsigned st,int x,int y,int w,int h,u32 c){
 for(int dy=-h;dy<=h;++dy){
  int ad=dy<0?-dy:dy,half=w*(h-ad)/h;
  box(p,st,x-half,y+dy,2*half+1,1,c);
 }
}
static void render_pads(struct app *a,u32 *p,unsigned st){
 for(int i=0;i<4;++i){
  unsigned f=a->shine[i];
  if(a->phase==INPUT&&a->selected==i&&f<65)f=65;
  u32 base=mix(colors[i],bright[i],f);
  diamond(p,st,cx[i],cy[i],56,47,mix(0xff0e121au,bright[i],18+f/3));
  diamond(p,st,cx[i],cy[i],50,41,0xff505a6bu);
  diamond(p,st,cx[i],cy[i],46,37,base);
  for(int k=2;k<17;++k)box(p,st,cx[i]-k,cy[i]-29+k/3,2*k+1,1,
                          mix(base,0xffffffffu,18+f/5));
  if(a->ripple_pad==i&&a->ripple_age){
   int w=50+(int)a->ripple_age/19,h=42+(int)a->ripple_age/25;
   if(w<74&&h<63){
    u32 line=mix(0xff0e121au,bright[i],170*(430-a->ripple_age)/430);
    for(int dy=-h;dy<=h;dy+=2){
     int ad=dy<0?-dy:dy,dx=w*(h-ad)/h;
     if(cx[i]-dx>0&&cx[i]+dx<WIDTH&&cy[i]+dy>0&&cy[i]+dy<HEIGHT){
      box(p,st,cx[i]-dx,cy[i]+dy,2,2,line);
      box(p,st,cx[i]+dx-1,cy[i]+dy,2,2,line);
     }
    }
   }
  }
 }
 diamond(p,st,187,173,25,22,0xffd9bd78u);
 diamond(p,st,187,173,20,17,0xff1e2838u);
 diamond(p,st,187,172,12,11,0xff52617bu);
 diamond(p,st,187,170,6,5,0xfff9e5b0u);
}
static void particles(struct app *a,u32 *p,unsigned st){
 if(!a->spark_age)return;
 unsigned age=850-a->spark_age;
 for(int k=0;k<22;++k){
  unsigned seed=(unsigned)k*1664525u+1013904223u;
  int dx=(int)(seed%231)-115,dy=(int)((seed>>11)%151)-75;
  int x=187+dx*(int)age/700,y=174+dy*(int)age/700;
  if(x>20&&x<355&&y>65&&y<265)
   box(p,st,x,y,age<500?3:2,age<500?3:2,k%3?0xffd9bd78u:0xffffffffu);
 }
}
void app_frame(struct app *a,u32 *p,unsigned st,unsigned ms){
 if(a->phase!=PAUSED)step(a,ms);
 box(p,st,0,0,WIDTH,HEIGHT,0xff0e121au);
 label(p,st,"DIAMOND SAYS",23,3,0xfff0dfb5u);
 label(p,st,"ROUND",57,1,0xffb5bfce);numeral(p,st,a->length,68,2);
 render_pads(a,p,st);particles(a,p,st);
 if(a->phase==CELEBRATE)label(p,st,"LEVEL UP",159,3,0xffffe0a0u);
 if(a->phase==LOST)label(p,st,"GAME OVER",159,2,0xffffa7a0u);
 if(a->phase==PAUSED)label(p,st,"PAUSED",159,3,0xfff0dfb5u);
 const char *status=a->phase==READY?"CENTER TO START":a->phase==SHOW?"WATCH CLOSELY":
 a->phase==INPUT?"YOUR TURN":a->phase==CELEBRATE?"NICE WORK":
 a->phase==LOST?"TRY AGAIN":a->phase==PAUSED?"CENTER RESUME":"YOU DID IT";
 label(p,st,status,268,2,0xffe9dfc8u);
 unsigned done=a->phase==SHOW?a->shown:a->entered;
 unsigned slots=a->length<14?a->length:14;
 if(!slots)slots=1;
 int start=(WIDTH-(int)slots*12)/2;
 for(unsigned i=0;i<slots;++i){
  unsigned index=a->length<=14?i:i*a->length/slots;
  box(p,st,start+(int)i*12,292,7,5,index<done?0xffd9bd78u:0xff465165u);
 }
 label(p,st,a->sounds?"SOUND ON":"SOUND OFF",305,1,0xff8693a6u);
 box(p,st,337,20,18,4,a->sounds?0xffd9bd78u:0xff6b7485u);
}
int app_animated(const struct app *a){
 if(a->phase==PAUSED)return 0;
 if(a->phase==SHOW||a->phase==CELEBRATE||a->spark_age||a->ripple_age||a->flash_age)return 1;
 for(int i=0;i<4;++i)if(a->shine[i])return 1;
 return 0;
}
