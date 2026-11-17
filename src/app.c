/* SPDX-License-Identifier: GPL-3.0-only
 * Diamond Says: original four-pad memory game for Q2 Pod.
 * Copyright (C) 2026 - GPL-3.0-only.
 */
#include "app.h"
enum { CENTRE=218, RETURN=170, WHEEL_STEP=13 };
static const u32 pad_colors[4]={0xfff07820u,0xffd9bd78u,0xff17191du,0xffaeb6c1u};
static const u32 flash_colors[4]={0xffffbd69u,0xffffe4a1u,0xff6d7480u,0xfff1f5ffu};
static const char *names[4]={"ORANGE","GOLD","BLACK","SILVER"};
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
static unsigned next(struct app *a){a->rng ^= a->rng<<13; a->rng ^= a->rng>>17; a->rng ^= a->rng<<5; return a->rng;}
static void add(struct app *a){if(a->length<MAX_SEQUENCE)a->sequence[a->length++]=(unsigned char)(next(a)&3u);}
static void begin(struct app *a){a->length=a->shown=a->entered=a->score=0; a->selected=0; a->lit=-1; add(a); a->phase=SHOW; a->timer=540;}
int app_init(struct app *a, unsigned long best){
    *a=(struct app){0}; a->rng=0x92c347e1u; a->best=best<=MAX_SEQUENCE?best:0;
    a->phase=READY; a->lit=-1; return 1;
}
static void choose(struct app *a,int pad){
    if(a->phase!=INPUT || pad<0 || pad>3)return;
    a->selected=pad; a->lit=pad; a->timer=210;
    if(a->sequence[a->entered]!=(unsigned)pad){a->phase=LOST;return;}
    ++a->entered;
    if(a->entered==a->length){
        a->score=a->length;
        if(a->score>a->best){a->best=a->score;a->save_pending=1;}
        if(a->length==MAX_SEQUENCE){a->phase=FINISHED;return;}
        add(a);a->shown=0;a->entered=0;a->phase=SHOW;a->timer=850;
    }
}
static void action(struct app *a){
    if(a->phase==READY || a->phase==LOST || a->phase==FINISHED)begin(a);
    else if(a->phase==PAUSED)a->phase=a->previous_phase;
    else if(a->phase==INPUT)choose(a,a->selected);
}
static int number(const char **p){
    int v=0, neg=0; while(**p==' ')++*p;
    if(**p=='-'){neg=1;++*p;}
    if(**p<'0'||**p>'9')return 10000;
    while(**p>='0'&&**p<='9'){if(v>1000)return 10000;v=v*10+*(*p)++-'0';}
    return neg?-v:v;
}
/* Q2 sends key-down and touch-down immediately; do not wait for release. */
int app_input(struct app *a,const char *s){
    if(!s[0]||s[1]!=' ')return 0;
    if(s[0]=='w'){
        const char *p=s+2;
        int value=number(&p);
        if(value < -2000 || value > 2000)return 0;
        a->wheel-=value;
        int steps=a->wheel/WHEEL_STEP;
        a->wheel%=WHEEL_STEP;
        if(a->phase==INPUT && steps)
            a->selected=(a->selected+steps%4+4)%4;
        return steps!=0;
    }
    if(!s[2]||s[3]!=' ')return 0;
    const char *p=s+4;
    int x=number(&p);
    if(s[0]=='k'){
        if(s[2]!='d')return 0; /* ignore key-up to avoid repeated actions */
        if(x==CENTRE){action(a);return 1;}
        if(x==RETURN){
            if(a->phase==PAUSED)a->phase=a->previous_phase;
            else if(a->phase==READY||a->phase==LOST||a->phase==FINISHED)a->quit=1;
            else {a->previous_phase=a->phase;a->phase=PAUSED;}
            return 1;
        }
        return 0;
    }
    if(s[0]=='t'){
        if(s[2]!='d')return 0;
        int y=number(&p);
        if(x<0||x>=WIDTH||y<0||y>=HEIGHT)return 0;
        if(a->phase!=INPUT){action(a);return 1;}
        /* Match the two rows of pads painted at x=67/190, y=116/186. */
        for(int pad=0;pad<4;++pad){
            int px=67+(pad%2)*123, py=116+(pad/2)*70;
            if(x>=px && x<px+111 && y>=py && y<py+58){
                choose(a,pad);
                return 1;
            }
        }
    }
    return 0;
}

static void step(struct app *a,unsigned elapsed){
    if(a->phase!=SHOW && a->phase!=INPUT && a->phase!=LOST)return;
    if(elapsed>a->timer)elapsed=a->timer;
    a->timer-=elapsed;
    if(a->timer)return;
    if(a->phase==SHOW){
        if(a->lit>=0){a->lit=-1; a->timer=180; ++a->shown;}
        else if(a->shown<a->length){a->lit=a->sequence[a->shown];a->timer=460;}
        else {a->phase=INPUT;a->lit=-1;a->timer=0;}
    }else a->lit=-1;
}
static void rect(u32 *p,unsigned stride,int x,int y,int w,int h,u32 color){
    if(x<0||y<0||x+w>WIDTH||y+h>HEIGHT)return;
    for(int xx=x;xx<x+w;++xx)for(int yy=y;yy<y+h;++yy)p[xx*stride+HEIGHT-1-yy]=color;
}
static void label(u32 *p,unsigned stride,const char *s,int y,int scale,u32 color){
    int n=0;while(s[n])++n;
    int x=(WIDTH-(n*6-1)*scale)/2;
    for(;*s;++s,x+=6*scale){
        int g=(*s>='A'&&*s<='Z')?*s-'A':(*s>='0'&&*s<='9')?*s-'0'+26:-1;
        if(g<0)continue;
        for(int r=0;r<7;++r)for(int c=0;c<5;++c)
            if(font[g][r]&(16u>>c))rect(p,stride,x+c*scale,y+r*scale,scale,scale,color);
    }
}
static void number_label(u32 *p,unsigned stride,unsigned value,int y){
    char digits[4];int len=0;
    do{digits[len++]=(char)('0'+value%10);value/=10;}while(value&&len<3);
    char text[4];for(int j=0;j<len;++j)text[j]=digits[len-j-1];text[len]=0;
    label(p,stride,text,y,2,0xffeeeeeeu);
}
void app_frame(struct app *a,u32 *page,unsigned stride,unsigned elapsed){
    if(a->phase!=PAUSED)step(a,elapsed);
    rect(page,stride,0,0,WIDTH,HEIGHT,0xff0e121au);
    label(page,stride,"DIAMOND SAYS",28,3,0xfff0dfb5u);
    label(page,stride,"ROUND",62,2,0xffb7bfccu);
    number_label(page,stride,a->length,84);
    for(int i=0;i<4;++i){
        int x=67+(i%2)*123,y=116+(i/2)*70;
        int active=(a->lit==i)||(a->phase==INPUT&&a->selected==i);
        u32 border=active?0xffffffffu:0xff636c79u;
        rect(page,stride,x-3,y-3,117,64,border);
        rect(page,stride,x,y,111,58,active?flash_colors[i]:pad_colors[i]);
        label(page,stride,names[i],y+23,1,i==2?0xfff3f3f3u:0xff121722u);
    }
    const char *status=a->phase==READY?"PRESS CENTER":a->phase==SHOW?"WATCH THE PATTERN":
        a->phase==INPUT?"REPEAT THE PATTERN":a->phase==LOST?"WRONG PATTERN":
        a->phase==FINISHED?"PERFECT MEMORY":"PAUSED";
    label(page,stride,status,267,2,0xfff0dfb5u);
    label(page,stride,a->phase==INPUT?"WHEEL SELECT CENTER PLAY":
        a->phase==SHOW?"MEMORIZE THE COLORS":
        a->phase==PAUSED?"CENTER RESUME":
        "CENTER START RETURN EXIT",289,1,0xffbac5d0u);
}
int app_animated(const struct app *a){return a->phase==SHOW||(a->lit>=0&&a->timer>0);}
