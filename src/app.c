/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026. Diamond Says for Q2 Pod.
 * Framebuffer effects use fixed point / integer arithmetic.
 */
#include "app.h"
enum { CENTRE=218,RETURN=170,WHEEL_STEP=13 };
static const u32 colors[4]={0xfff07820u,0xffd9bd78u,0xff17191du,0xffaeb6c1u};
static const u32 bright[4]={0xffffc078u,0xffffe5a4u,0xff7a8290u,0xfff5f8ffu};
/* Quadrant order: top-left, top-right, bottom-right, bottom-left. */
static const int cx[4]={132,243,243,132},cy[4]={130,130,218,218};
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
/* Touch only the visible colored areas, never the cat or the dividers. */
static int hit(int x,int y){
    int left=74, top=83, right=301, bottom=267, corner=31;
    if(x<left+5||x>=right-5||y<top+5||y>=bottom-5)return -1;
    int dx=0,dy=0;
    if(x<left+corner)dx=left+corner-x;
    else if(x>=right-corner)dx=x-(right-corner-1);
    if(y<top+corner)dy=top+corner-y;
    else if(y>=bottom-corner)dy=y-(bottom-corner-1);
    if(dx*dx+dy*dy>corner*corner)return -1;
    dx=x-187;dy=y-175;
    if(dx*dx+dy*dy<44*44)return -1;
    if(x>=184&&x<=190)return -1;
    if(y>=172&&y<=178)return -1;
    return (y<175)?(x<187?0:1):(x<187?3:2);
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
  if(y>=299){a->sounds=!a->sounds;return 1;}
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
/* Small integer drawing helpers: no PNG decoder or heap allocation. */
static void dot(u32 *p,unsigned stride,int x,int y,u32 color){
    if(x>=0&&x<WIDTH&&y>=0&&y<HEIGHT)
        p[x*stride+HEIGHT-1-y]=color;
}
static void disc(u32 *p,unsigned stride,int x,int y,int radius,u32 color){
    for(int dy=-radius;dy<=radius;++dy)
        for(int dx=-radius;dx<=radius;++dx)
            if(dx*dx+dy*dy<=radius*radius)
                dot(p,stride,x+dx,y+dy,color);
}
static void oval(u32 *p,unsigned stride,int x,int y,int rx,int ry,u32 color){
    for(int dy=-ry;dy<=ry;++dy)
        for(int dx=-rx;dx<=rx;++dx)
            if(dx*dx*ry*ry+dy*dy*rx*rx<=rx*rx*ry*ry)
                dot(p,stride,x+dx,y+dy,color);
}
static void segment(u32 *p,unsigned stride,int x0,int y0,int x1,int y1,u32 color){
    int dx=x1-x0,dy=y1-y0;
    int adx=dx<0?-dx:dx,ady=dy<0?-dy:dy;
    int n=adx>ady?adx:ady;
    if(!n){dot(p,stride,x0,y0,color);return;}
    for(int i=0;i<=n;++i)
        dot(p,stride,x0+dx*i/n,y0+dy*i/n,color);
}
static void ears(u32 *p,unsigned stride,int x,int y,int side,u32 outer,u32 inside){
    int tip=x+side*14;
    for(int k=0;k<20;++k){
        int half=2+k/2;
        int center=tip+(x-tip)*k/20;
        box(p,stride,center-half,y+k,2*half+1,1,outer);
    }
    for(int k=5;k<17;++k){
        int half=(k-3)/3;
        int center=tip+(x-tip)*k/20;
        box(p,stride,center-half,y+k,2*half+1,1,inside);
    }
}
static void cat(u32 *p,unsigned stride){
    const u32 line=0xff242632u, fur=0xffb8b3b1u, pale=0xffeee9e4u;
    const u32 pink=0xffffa9b2u, green=0xff739e72u;
    /* Central charcoal hub gives the mascot space and separates all four pads. */
    disc(p,stride,187,175,45,0xff292c35u);
    disc(p,stride,187,175,41,0xff41424bu);
    oval(p,stride,187,199,28,16,line);
    oval(p,stride,187,198,25,13,fur);
    ears(p,stride,170,137,-1,line,pink);
    ears(p,stride,204,137, 1,line,pink);
    oval(p,stride,187,174,31,30,line);
    oval(p,stride,187,174,28,27,fur);
    oval(p,stride,187,188,20,13,pale);
    /* A few tabby stripes and bright, readable eyes at 375x320. */
    segment(p,stride,181,148,178,155,0xff686462u);
    segment(p,stride,188,146,188,153,0xff686462u);
    segment(p,stride,196,149,199,156,0xff686462u);
    segment(p,stride,163,171,170,174,0xff77716eu);
    segment(p,stride,206,173,212,170,0xff77716eu);
    oval(p,stride,177,172,8,10,line);
    oval(p,stride,197,172,8,10,line);
    oval(p,stride,177,172,6,8,green);
    oval(p,stride,197,172,6,8,green);
    disc(p,stride,178,173,3,0xff111b1cu);
    disc(p,stride,198,173,3,0xff111b1cu);
    disc(p,stride,179,169,2,0xffffffffu);
    disc(p,stride,199,169,2,0xffffffffu);
    oval(p,stride,187,185,5,3,pink);
    segment(p,stride,187,187,187,191,line);
    segment(p,stride,187,191,183,193,line);
    segment(p,stride,187,191,191,193,line);
    segment(p,stride,178,188,163,184,0xffe9e5dfu);
    segment(p,stride,178,191,163,192,0xffe9e5dfu);
    segment(p,stride,196,188,211,184,0xffe9e5dfu);
    segment(p,stride,196,191,211,192,0xffe9e5dfu);
    oval(p,stride,176,205,10,6,pale);
    oval(p,stride,198,205,10,6,pale);
}
/* The four chunky, soft-corner buttons share a rounded-square silhouette.
 * Integer masks replace the former sharp diamond outlines.
 */
static void render_pads(struct app *a,u32 *p,unsigned stride){
    const int left=74,top=83,right=301,bottom=267,corner=32;
    const u32 surround=0xff292c35u;
    for(int y=top;y<bottom;++y){
        for(int x=left;x<right;++x){
            int dx=0,dy=0;
            if(x<left+corner)dx=left+corner-x;
            else if(x>=right-corner)dx=x-(right-corner-1);
            if(y<top+corner)dy=top+corner-y;
            else if(y>=bottom-corner)dy=y-(bottom-corner-1);
            if(dx*dx+dy*dy>corner*corner)continue;
            int i=(y<175)?(x<187?0:1):(x<187?3:2);
            int inner= x<left+6||x>=right-6||y<top+6||y>=bottom-6;
            int gap=(x>=183&&x<=191)||(y>=171&&y<=179);
            int cdx=x-187,cdy=y-175;
            int central=cdx*cdx+cdy*cdy<45*45;
            if(inner||gap||central){dot(p,stride,x,y,surround);continue;}
            unsigned shine=a->shine[i];
            if(a->phase==INPUT&&a->selected==i&&shine<58)shine=58;
            u32 color=mix(colors[i],bright[i],shine);
            /* A gentle top-edge highlight gives the buttons some depth. */
            if(y<cy[i]-18&&y>top+10)
                color=mix(color,0xffffffffu,24+shine/9);
            if(i==2||i==3){
                if(y>bottom-24)color=mix(color,0xff12151cu,32);
            }
            dot(p,stride,x,y,color);
        }
    }
    /* Restrained glow on the active quadrant, without jagged geometry. */
    if(a->ripple_age&&a->ripple_pad>=0&&a->ripple_pad<4){
        int i=a->ripple_pad;
        unsigned age=a->ripple_age;
        int radius=18+(int)age/13;
        u32 glow=mix(colors[i],bright[i],200*(430-age)/430);
        for(int k=0;k<72;++k){
            /* Circle sampled with integer midpoint-free coordinates. */
            int x=cx[i]-radius+k*radius/36;
            int dd=x-cx[i];
            int rr=radius*radius-dd*dd;
            if(rr<0)continue;
            int y=0;
            while((y+1)*(y+1)<=rr)++y;
            int yy=cy[i]-y;
            if(hit(x,yy)==i)dot(p,stride,x,yy,glow);
            yy=cy[i]+y;
            if(hit(x,yy)==i)dot(p,stride,x,yy,glow);
        }
    }
    cat(p,stride);
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
void app_frame(struct app *a,u32 *p,unsigned stride,unsigned ms){
    if(a->phase!=PAUSED)step(a,ms);
    box(p,stride,0,0,WIDTH,HEIGHT,0xfff7f1e9u);
    /* Compact title leaves room for a round label that is always readable. */
    label(p,stride,"DIAMOND SAYS",16,2,0xff3a333cu);
    unsigned round=a->length?a->length:1;
    char text[11]={'R','O','U','N','D',' ','0','0','0',0};
    if(round<10){
        text[6]=(char)('0'+round);
        text[7]=0;
    }else if(round<100){
        text[6]=(char)('0'+round/10);
        text[7]=(char)('0'+round%10);
        text[8]=0;
    }else{
        text[6]=(char)('0'+round/100);
        text[7]=(char)('0'+round/10%10);
        text[8]=(char)('0'+round%10);
    }
    /* Soft pill backdrop. */
    for(int y=46;y<76;++y){
        int inset=0;
        if(y<55)inset=55-y;
        else if(y>66)inset=y-66;
        box(p,stride,129+inset,y,117-2*inset,1,0xffddd3ecu);
    }
    label(p,stride,text,53,2,0xff403b56u);
    render_pads(a,p,stride);
    particles(a,p,stride);
    if(a->phase==CELEBRATE)label(p,stride,"LEVEL UP",157,2,0xffffe0a0u);
    if(a->phase==LOST)label(p,stride,"TRY AGAIN",157,2,0xffffb1a5u);
    if(a->phase==PAUSED)label(p,stride,"PAUSED",157,2,0xfff0dfb5u);
    const char *status=a->phase==READY?"CENTER TO START":
        a->phase==SHOW?"WATCH THE PATTERN":
        a->phase==INPUT?"YOUR TURN":
        a->phase==CELEBRATE?"NICE WORK":
        a->phase==LOST?"TRY AGAIN":
        a->phase==PAUSED?"CENTER RESUME":"YOU DID IT";
    label(p,stride,status,277,2,0xff453c50u);
    unsigned done=a->phase==SHOW?a->shown:a->entered;
    unsigned slots=a->length<14?a->length:14;
    if(!slots)slots=1;
    int start=(WIDTH-(int)slots*12)/2;
    for(unsigned i=0;i<slots;++i){
        unsigned index=a->length<=14?i:i*a->length/slots;
        box(p,stride,start+(int)i*12,297,7,4,
            index<done?0xffb494d8u:0xffd7cbd8u);
    }
    label(p,stride,a->sounds?"SOUND ON":"SOUND OFF",309,1,0xff766d78u);
}

int app_animated(const struct app *a){
 if(a->phase==PAUSED)return 0;
 if(a->phase==SHOW||a->phase==CELEBRATE||a->spark_age||a->ripple_age||a->flash_age)return 1;
 for(int i=0;i<4;++i)if(a->shine[i])return 1;
 return 0;
}
