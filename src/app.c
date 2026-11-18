/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026. Diamond Says for Q2 Pod.
 * Framebuffer effects use fixed point / integer arithmetic.
 */
#include "app.h"
enum { CENTRE=218,RETURN=170,WHEEL_STEP=13 };
static const u32 colors[4]={0xfff07820u,0xffd9bd78u,0xff17191du,0xffaeb6c1u};
static const u32 bright[4]={0xffffc078u,0xffffe5a4u,0xff7a8290u,0xfff5f8ffu};
/* Quadrant order: top-left, top-right, bottom-right, bottom-left. */
/* Wide 151 x 72 pads fill the Q2 display, leaving a separate status bar. */
enum {
 BOARD_X=27, BOARD_Y=96, BOARD_W=321, BOARD_H=162,
 PAD_W=151, PAD_H=72, PAD_RADIUS=18, HUB_X=187, HUB_Y=177, HUB_RADIUS=54
};
static const int px[4]={34,190,190,34},py[4]={102,102,180,180};
static const int cx[4]={109,265,265,109},cy[4]={138,138,216,216};
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
static unsigned tempo(struct app *a){
    if(a->length>=24)return 170;
    return 460-a->length*12;
}
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
/* The same rounded-rectangle mask is used for paint and touch. */
static int rounded_hit(int x,int y,int left,int top,int width,int height,int radius){
    int rx=x-left,ry=y-top;
    if(rx<0||ry<0||rx>=width||ry>=height)return 0;
    int dx=0,dy=0;
    if(rx<radius)dx=radius-1-rx;
    else if(rx>=width-radius)dx=rx-(width-radius);
    if(ry<radius)dy=radius-1-ry;
    else if(ry>=height-radius)dy=ry-(height-radius);
    return dx*dx+dy*dy<radius*radius;
}
static int hit(int x,int y){
    int dx=x-HUB_X,dy=y-HUB_Y;
    if(dx*dx+dy*dy<=HUB_RADIUS*HUB_RADIUS)return -1;
    for(int i=0;i<4;++i)
        if(rounded_hit(x,y,px[i],py[i],PAD_W,PAD_H,PAD_RADIUS))return i;
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
/* Rounded rectangle shared by the board, pads and status strip. */
static void rounded_rect(u32 *p,unsigned stride,int x,int y,int width,int height,
                         int radius,u32 color){
    for(int yy=y;yy<y+height;++yy)
        for(int xx=x;xx<x+width;++xx)
            if(rounded_hit(xx,yy,x,y,width,height,radius))
                dot(p,stride,xx,yy,color);
}
static void stroke(u32 *p,unsigned stride,int x0,int y0,int x1,int y1,int width,u32 c){
    int dx=x1-x0,dy=y1-y0;
    int ax=dx<0?-dx:dx,ay=dy<0?-dy:dy;
    int n=ax>ay?ax:ay;
    if(!n){disc(p,stride,x0,y0,width,c);return;}
    for(int i=0;i<=n;++i)
        disc(p,stride,x0+dx*i/n,y0+dy*i/n,width,c);
}
/* Three-point faceted polygons are tiny and avoid PNG assets or dependencies. */
static int edge(int ax,int ay,int bx,int by,int x,int y){
    return (x-ax)*(by-ay)-(y-ay)*(bx-ax);
}
static void triangle(u32 *p,unsigned stride,
                     int ax,int ay,int bx,int by,int cx0,int cy0,u32 color){
    int minx=ax<bx?ax:bx,maxx=ax>bx?ax:bx;
    int miny=ay<by?ay:by,maxy=ay>by?ay:by;
    if(cx0<minx)minx=cx0;
    if(cx0>maxx)maxx=cx0;
    if(cy0<miny)miny=cy0;
    if(cy0>maxy)maxy=cy0;
    int winding=edge(ax,ay,bx,by,cx0,cy0);
    if(!winding)return;
    for(int y=miny;y<=maxy;++y)
        for(int x=minx;x<=maxx;++x){
            int e0=edge(ax,ay,bx,by,x,y);
            int e1=edge(bx,by,cx0,cy0,x,y);
            int e2=edge(cx0,cy0,ax,ay,x,y);
            if(winding>0?(e0>=0&&e1>=0&&e2>=0):(e0<=0&&e1<=0&&e2<=0))
                dot(p,stride,x,y,color);
        }
}
static void sparkle(u32 *p,unsigned stride,int x,int y,u32 color){
    stroke(p,stride,x-3,y,x+3,y,0,color);
    stroke(p,stride,x,y-3,x,y+3,0,color);
    dot(p,stride,x,y,0xffffffffu);
}
/* Gray tabby holding a little faceted diamond, as in the approved mockup.
 * All shapes are rasterized directly to the 375x320 framebuffer.
 * The medallion is deliberately small enough to leave useful touch targets.
 */
static void cat(u32 *p,unsigned stride){
    const u32 outline=0xff252630u, gray=0xffb7b2b0u;
    const u32 stripe=0xff746e6cu, white=0xfffffaf8u;
    const u32 pink=0xffffb0bcu, eye=0xff396d45u;
    const u32 ice=0xffc8f2ffu, blue=0xff80ceeau;
    /* Nested round medallion with a warm pastel interior. */
    disc(p,stride,187,177,54,outline);
    disc(p,stride,187,177,51,0xfff8edf6u);
    disc(p,stride,187,177,46,0xfff5e8f2u);
    /* Body and chest behind the head, with a resting left paw. */
    oval(p,stride,181,202,29,18,outline);
    oval(p,stride,181,201,26,16,gray);
    oval(p,stride,191,203,15,14,white);
    stroke(p,stride,160,200,167,205,2,stripe);
    stroke(p,stride,163,207,170,210,2,stripe);
    /* Ear triangles, outlined with contrasting pink inner ears. */
    triangle(p,stride,157,161,157,135,180,152,outline);
    triangle(p,stride,160,157,160,141,175,153,pink);
    triangle(p,stride,194,150,214,137,213,165,outline);
    triangle(p,stride,198,151,211,143,210,157,pink);
    /* Large rounded gray face and white muzzle. */
    oval(p,stride,184,171,33,27,outline);
    oval(p,stride,184,170,30,24,gray);
    oval(p,stride,197,185,19,12,white);
    /* Six curved tabby streaks use short rounded strokes. */
    stroke(p,stride,174,149,180,157,2,stripe);
    stroke(p,stride,183,147,190,156,2,stripe);
    stroke(p,stride,191,149,197,156,2,stripe);
    stroke(p,stride,157,172,166,171,2,stripe);
    stroke(p,stride,157,181,167,177,2,stripe);
    stroke(p,stride,165,192,173,195,1,stripe);
    /* Asymmetric eyes give a curious, thoughtful expression. */
    oval(p,stride,180,174,8,10,outline);
    oval(p,stride,180,174,6,8,eye);
    oval(p,stride,204,172,5,8,outline);
    oval(p,stride,204,172,3,6,eye);
    disc(p,stride,182,170,2,white);
    disc(p,stride,205,169,1,white);
    /* Pink nose, two tiny mouth curves and whiskers. */
    triangle(p,stride,194,183,202,183,198,189,pink);
    stroke(p,stride,198,189,197,193,0,outline);
    stroke(p,stride,197,193,193,195,1,outline);
    stroke(p,stride,198,193,201,195,1,outline);
    stroke(p,stride,192,186,178,184,0,white);
    stroke(p,stride,192,190,178,191,0,white);
    stroke(p,stride,206,186,220,183,0,white);
    stroke(p,stride,206,191,220,191,0,white);
    /* A diamond with six independently shaded facets. */
    triangle(p,stride,176,194,185,187,210,192,outline);
    triangle(p,stride,176,194,210,192,204,215,outline);
    triangle(p,stride,176,194,185,190,192,198,ice);
    triangle(p,stride,185,190,202,190,192,198,0xffedfcffu);
    triangle(p,stride,202,190,210,194,192,198,blue);
    triangle(p,stride,176,194,192,198,187,212,0xff93d9f4u);
    triangle(p,stride,192,198,210,194,204,213,0xff65b7e4u);
    triangle(p,stride,192,198,204,213,187,212,ice);
    stroke(p,stride,176,194,185,187,0,outline);
    stroke(p,stride,185,187,210,192,0,outline);
    stroke(p,stride,210,192,204,215,0,outline);
    stroke(p,stride,204,215,187,212,0,outline);
    stroke(p,stride,187,212,176,194,0,outline);
    /* One white paw tucked below the chin and wrapped around the gem. */
    oval(p,stride,207,198,10,12,outline);
    oval(p,stride,206,197,8,10,white);
    stroke(p,stride,204,190,202,193,0,outline);
    stroke(p,stride,209,191,207,194,0,outline);
    /* The other paw rests naturally on the lower gem facet. */
    oval(p,stride,177,211,13,7,outline);
    oval(p,stride,177,210,11,5,white);
    stroke(p,stride,178,208,179,212,0,outline);
    sparkle(p,stride,172,190,blue);
    sparkle(p,stride,214,211,0xffe4faffu);
}
static void render_pads(struct app *a,u32 *p,unsigned stride){
    const u32 rim=0xff55505bu;
    /* Cream surround and lavender border match the approved layout. */
    rounded_rect(p,stride,BOARD_X-4,BOARD_Y-4,BOARD_W+8,BOARD_H+8,
                 28,0xff514957u);
    rounded_rect(p,stride,BOARD_X-2,BOARD_Y-2,BOARD_W+4,BOARD_H+4,
                 26,0xfffff7f5u);
    /* Pads are independently rounded, so the outer shell clips none of them. */
    rounded_rect(p,stride,BOARD_X,BOARD_Y,BOARD_W,BOARD_H,25,rim);
    for(int i=0;i<4;++i){
        unsigned shine=a->shine[i];
        if(a->phase==INPUT&&a->selected==i&&shine<60)shine=60;
        u32 base=mix(colors[i],bright[i],shine);
        rounded_rect(p,stride,px[i],py[i],PAD_W,PAD_H,PAD_RADIUS,base);
        /* Simple highlights within the pad itself, preserving uniform shape. */
        for(int yy=py[i]+8;yy<py[i]+13;++yy)
            for(int xx=px[i]+20;xx<px[i]+PAD_W-20;++xx)
                if(rounded_hit(xx,yy,px[i],py[i],PAD_W,PAD_H,PAD_RADIUS))
                    dot(p,stride,xx,yy,mix(base,0xffffffffu,38+shine/7));
        if(a->ripple_pad==i&&a->ripple_age){
            unsigned age=a->ripple_age;
            int radius=14+(int)age/12;
            u32 ring=mix(base,bright[i],170*(430-age)/430);
            for(int xx=cx[i]-radius;xx<=cx[i]+radius;++xx){
                int dx=xx-cx[i],v=radius*radius-dx*dx;
                int dy=0;
                while((dy+1)*(dy+1)<=v)++dy;
                if(hit(xx,cy[i]-dy)==i)dot(p,stride,xx,cy[i]-dy,ring);
                if(hit(xx,cy[i]+dy)==i)dot(p,stride,xx,cy[i]+dy,ring);
            }
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
    /* Status has its own pill below the board; never obscure the mascot. */
    const char *status=a->phase==READY?"CENTER TO START":
        a->phase==SHOW?"WATCH THE PATTERN":
        a->phase==INPUT?"YOUR TURN":
        a->phase==CELEBRATE?"LEVEL UP":
        a->phase==LOST?"TRY AGAIN":
        a->phase==PAUSED?"CENTER RESUME":"YOU DID IT";
    u32 bg=a->phase==LOST?0xffa83847u:0xffe7d6eeu;
    u32 fg=a->phase==LOST?0xffffffffu:0xff403447u;
    rounded_rect(p,stride,69,270,237,23,11,bg);
    label(p,stride,status,275,2,fg);
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
