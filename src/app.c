/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026. Diamond Says for Q2 Pod.
 * Drawn the way Q2 Pod draws its own games (Snake, Vortex): the score and best along the top, a
 * flat board in a hairline frame, and the theme's pop-up (Q2_POPUP) for the title, the pause and
 * the game over. Integer arithmetic only.
 */
#include "app.h"
#include "cat_sprite.h"
#include "font.h"
enum { CENTRE=218,RETURN=170,WHEEL_STEP=13 };
/* Q2 Orange, Champagne Gold, Black and Silver; lit, each brightens. */
static const u32 colors[4]={0xfff07820u,0xffd9bd78u,0xff17191du,0xffaeb6c1u};
static const u32 bright[4]={0xffffc078u,0xffffe5a4u,0xff7a8290u,0xfff5f8ffu};
/* Quadrant order: top-left, top-right, bottom-right, bottom-left. */
enum {
 HUD_X=24, HUD_Y=6,                               /* as Q2 Pod's games */
 BOARD_X=24, BOARD_Y=38, BOARD_W=326, BOARD_H=228, BOARD_RADIUS=8,
 PAD_W=154, PAD_H=105, PAD_RADIUS=12,
 HUB_X=187, HUB_Y=152, HUB_RADIUS=52, HUB_MOAT=57,
 STATUS_Y=272, DOTS_Y=303, DOT=6, DOT_STEP=14,
 POP_W=281, POP_X=(375-281)/2, CAPS_TRACK=2, DOT_X=76, TEXT_X=92
};
static const int px[4]={30,190,190,30},py[4]={44,44,155,155};

/* ---- the game ---- */
static unsigned rand32(struct app *a){a->rng^=a->rng<<13;a->rng^=a->rng>>17;a->rng^=a->rng<<5;return a->rng;}
static void add(struct app *a){if(a->length<MAX_SEQUENCE)a->sequence[a->length++]=(unsigned char)(rand32(a)&3u);}
static void signal(struct app *a,int s){if(a->sounds){a->sound_event=s;a->sound_pending=1;}}
static void pulse(struct app *a,int n){a->shine[n]=255;signal(a,n+1);}
static unsigned tempo(struct app *a){
    if(a->length>=24)return 170;
    return 460-a->length*12;
}
static void begin(struct app *a){
 a->rng^=a->tick+0x9e3779b9u;if(!a->rng)a->rng=0x12345678u;
 a->length=a->shown=a->entered=a->score=0;a->selected=0;a->lit=-1;a->record=0;
 for(int i=0;i<4;++i)a->shine[i]=0;
 add(a);a->phase=SHOW;a->timer=500;
}
int app_init(struct app *a,unsigned long best){
 *a=(struct app){0};a->rng=0x92c347e1u;a->best=best<=MAX_SEQUENCE?best:0;
 a->phase=READY;a->lit=-1;a->sounds=1;a->dark_mode=1;app_theme(a,0);return 1;
}
static void choose(struct app *a,int n){
 if(a->phase!=INPUT||n<0||n>3)return;
 a->selected=n;pulse(a,n);
 if(a->sequence[a->entered]!=(unsigned)n){
  a->phase=LOST;signal(a,5);return;
 }
 ++a->entered;
 if(a->entered==a->length){
  a->score=a->length;
  if(a->score>a->best){a->best=a->score;a->save_pending=1;a->record=1;}
  a->phase=a->length==MAX_SEQUENCE?FINISHED:CELEBRATE;
  a->timer=900;signal(a,6);
 }
}
static void pause_menu(struct app *a){a->previous_phase=a->phase;a->phase=PAUSED;a->choice=MENU_CONTINUE;}
static void pick(struct app *a){
 if(a->choice==MENU_CONTINUE)a->phase=a->previous_phase;
 else if(a->choice==MENU_SOUND)a->sounds=!a->sounds;
 else a->quit=1;
}
static void action(struct app *a){
 if(a->phase==READY||a->phase==LOST||a->phase==FINISHED)begin(a);
 else if(a->phase==PAUSED)pick(a);
 else if(a->phase==INPUT)choose(a,a->selected);
}
static int number(const char **p){
 int v=0,neg=0;while(**p==' ')++*p;
 if(**p=='-'){neg=1;++*p;}
 if(**p<'0'||**p>'9')return 10000;
 while(**p>='0'&&**p<='9'){if(v>1000)return 10000;v=v*10+*(*p)++-'0';}
 return neg?-v:v;
}
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
/* Touch only the pads, never the cat. */
static int hit(int x,int y){
    int dx=x-HUB_X,dy=y-HUB_Y;
    if(dx*dx+dy*dy<=HUB_MOAT*HUB_MOAT)return -1;
    for(int i=0;i<4;++i)
        if(rounded_hit(x,y,px[i],py[i],PAD_W,PAD_H,PAD_RADIUS))return i;
    return -1;
}
/* The pause menu's box and its rows, as Q2 Pod's popup() places them. */
static int menu_top(const struct app *a){return a->theme.style==CLASSIC?POP_ROW+1:POP_ROW+6;}
static int menu_h(const struct app *a){return menu_top(a)+MENU_ROWS*POP_ROW+(a->theme.style==CLASSIC?0:8);}
static int menu_y(const struct app *a){return (HEIGHT-menu_h(a))/2;}
static int menu_row(const struct app *a,int x,int y){
    int r=y-menu_y(a)-menu_top(a);
    if(x<POP_X||x>=POP_X+POP_W||r<0||r>=MENU_ROWS*POP_ROW)return -1;
    return r/POP_ROW;
}

int app_input(struct app *a,const char *s){
 if(!s[0]||s[1]!=' ')return 0;
 if(s[0]=='w'){
  const char *p=s+2;int v=number(&p);
  if(v< -2000||v>2000)return 0;
  a->wheel-=v;int steps=a->wheel/WHEEL_STEP;a->wheel%=WHEEL_STEP;
  if(!steps)return 0;
  if(a->phase==INPUT)a->selected=(a->selected+steps%4+4)%4;
  else if(a->phase==PAUSED){
   int c=a->choice+steps;
   a->choice=c<0?0:c>=MENU_ROWS?MENU_ROWS-1:c;
  }
  return 1;
 }
 if(!s[2]||s[3]!=' ')return 0;
 const char *p=s+4;int x=number(&p);
 if(s[0]=='k'){
  if(s[2]!='d')return 0;
  if(x==CENTRE){action(a);return 1;}
  if(x==RETURN){
   if(a->phase==PAUSED)a->quit=1;
   else if(a->phase==READY||a->phase==LOST||a->phase==FINISHED)a->quit=1;
   else pause_menu(a);
   return 1;
  }
  return 0;
 }
 if(s[0]=='t'&&s[2]=='d'){
  int y=number(&p);if(x<0||x>=WIDTH||y<0||y>=HEIGHT)return 0;
  if(a->phase==PAUSED){
   int r=menu_row(a,x,y);if(r<0)return 0;
   a->choice=r;pick(a);return 1;
  }
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

/* ---- the theme ---- */
static u32 mix(u32 a,u32 b,unsigned v){
 if(v>255)v=255;
 unsigned u=255-v;
 unsigned r=(((a>>16)&255)*u+((b>>16)&255)*v)/255;
 unsigned g=(((a>>8)&255)*u+((b>>8)&255)*v)/255;
 unsigned blue=((a&255)*u+(b&255)*v)/255;
 return 0xff000000u|(r<<16)|(g<<8)|blue;
}
static unsigned luma(u32 c){return (((c>>16)&255)*3+((c>>8)&255)*6+(c&255))/10;}
static int word_is(const char *s,const char *w){
 while(*w&&*s==*w)++s,++w;
 return !*w&&(*s==' '||!*s);
}
static const char *hex(const char *s,u32 *c){ /* " RRGGBB" into c: past it, else 0 */
 unsigned v=0;
 if(*s++!=' ')return 0;
 for(int i=0;i<6;++i,++s){
  int d=*s>='0'&&*s<='9'?*s-'0':*s>='a'&&*s<='f'?*s-'a'+10:*s>='A'&&*s<='F'?*s-'A'+10:-1;
  if(d<0)return 0;
  v=v<<4|(unsigned)d;
 }
 *c=0xff000000u|v;return s;
}
/* "<style> <box> <frame> <title> <text> <pick> <pick_text>" and, for Classic, its title strip's
 * and chosen band's POP_ROW rows each (q2-pod docs/librespot.md). Anything short of a whole value
 * keeps the game's own look. Later firmware's further fields, after a space, are not ours. */
static int theme_read(struct theme *t,const char *s){
 int style=word_is(s,"minimal")?MINIMAL:word_is(s,"modern")?MODERN:word_is(s,"classic")?CLASSIC:0;
 if(!style)return 0;
 while(*s&&*s!=' ')++s;
 u32 *c[6]={&t->box,&t->frame,&t->title,&t->text,&t->pick,&t->pick_text};
 for(int i=0;i<6;++i)if(!(s=hex(s,c[i])))return 0;
 for(int i=0;style==CLASSIC&&i<2*POP_ROW;++i)
  if(!(s=hex(s,i<POP_ROW?&t->head[i]:&t->band[i-POP_ROW])))return 0;
 if(*s&&*s!=' ')return 0;
 t->style=style;return 1;
}
void app_theme(struct app *a,const char *value){
 struct theme *t=&a->theme,read={0};
 if(value&&theme_read(&read,value)){
  *t=read;
  /* Classic's page is its paper; the others' is black, white in Light Mode, under the ink. */
  t->bg=t->style==CLASSIC?t->box:luma(t->title)>127?0xff000000u:0xffffffffu;
  t->panel=t->box;
  a->dark_mode=luma(t->bg)<128;
 }else if(a->dark_mode){
  *t=(struct theme){OWN,0xff000000u,0xff111111u,0xff1c1c1cu,0xff3a3a3au,0xffffffffu,
   0xffaaaaaau,0,0xfff07820u,0xffffffffu,{0},{0}};
 }else{
  *t=(struct theme){OWN,0xffffffffu,0xffeeeeeeu,0xfff2f2f2u,0xffc8c8c8u,0xff000000u,
   0xff555555u,0,0xfff07820u,0xffffffffu,{0},{0}};
 }
 /* Q2 Pod's games caption their HUD in grey; Classic's rows are all ink, so grey it from them. */
 t->muted=t->text!=t->title?t->text:mix(t->title,t->bg,110);
}

/* ---- painting: the panel is portrait, so logical (x, y) lands at x * stride + HEIGHT-1-y ---- */
static void dot(u32 *p,unsigned st,int x,int y,u32 c){
 if(x>=0&&x<WIDTH&&y>=0&&y<HEIGHT)p[x*st+HEIGHT-1-y]=c;
}
static void blend(u32 *p,unsigned st,int x,int y,u32 c,unsigned alpha){
 if(x<0||x>=WIDTH||y<0||y>=HEIGHT||!alpha)return;
 u32 *q=&p[x*st+HEIGHT-1-y];
 *q=alpha>=255?c:mix(*q,c,alpha);
}
static void box(u32 *p,unsigned st,int x,int y,int w,int h,u32 c){
 for(int i=x;i<x+w;++i)for(int j=y;j<y+h;++j)dot(p,st,i,j,c);
}
/* A rounded box, its corners anti-aliased from four samples a pixel; row i is rows[i], or the last
 * of n past them. */
static unsigned corner(int x,int y,int l,int t,int w,int h,int r){
 unsigned n=0;
 for(int k=0;k<4;++k){
  int sx=4*x+1+(k&1)*2,sy=4*y+1+(k>>1)*2,L=4*l,T=4*t,R=4*r,W=4*w,H=4*h;
  int cx=sx<L+R?L+R:sx>L+W-R?L+W-R:sx,cy=sy<T+R?T+R:sy>T+H-R?T+H-R:sy;
  int dx=sx-cx,dy=sy-cy;
  n+=dx*dx+dy*dy<=R*R;
 }
 return n;
}
static void rrect_rows(u32 *p,unsigned st,int x,int y,int w,int h,int r,const u32 *rows,int n){
 for(int j=y;j<y+h;++j){
  u32 c=rows[j-y<n?j-y:n-1];
  for(int i=x;i<x+w;++i){
   int edge=(i<x+r||i>=x+w-r)&&(j<y+r||j>=y+h-r);
   unsigned k=edge?corner(i,j,x,y,w,h,r):4;
   if(k)blend(p,st,i,j,c,k==4?255:k*64);
  }
 }
}
static void rrect(u32 *p,unsigned st,int x,int y,int w,int h,int r,u32 c){rrect_rows(p,st,x,y,w,h,r,&c,1);}
/* A box in a hairline frame, as Q2 Pod's round_box and frame_rect draw a board. */
static void framed(u32 *p,unsigned st,int x,int y,int w,int h,int r,u32 fill,u32 line){
 rrect(p,st,x,y,w,h,r,line);
 rrect(p,st,x+1,y+1,w-2,h-2,r-1,fill);
}
static void disc(u32 *p,unsigned st,int x,int y,int radius,u32 c){
 rrect(p,st,x-radius,y-radius,2*radius,2*radius,radius,c);
}
static void dim(u32 *p,unsigned st){ /* black at half, as Q2 Pod dims a game under its pop-up */
 for(int x=0;x<WIDTH;++x)for(int y=0;y<HEIGHT;++y){
  u32 *q=&p[x*st+y];*q=0xff000000u|((*q>>1)&0x7f7f7fu);
 }
}

/* Q2 Pod's caption face; Minimal sets every word in capitals, CAPS_TRACK apart. */
static char shown(const struct app *a,char c){return a->theme.style==MINIMAL&&c>='a'&&c<='z'?(char)(c-32):c;}
static int text_w(const struct app *a,const char *s){
 int w=0,n=0;
 for(;*s;++s){char c=shown(a,*s);if(c>=32&&c<127)w+=font_adv[c-32],++n;}
 if(a->theme.style==MINIMAL&&n)w+=(n-1)*CAPS_TRACK;
 return w;
}
static int text(const struct app *a,u32 *p,unsigned st,int x,int y,const char *s,u32 c){
 int x0=x;
 for(;*s;++s){
  char ch=shown(a,*s);
  if(ch<32||ch>126)continue;
  int k=ch-32,w=font_adv[k];
  const unsigned char *g=font_px+font_off[k];
  for(int r=0;r<FONT_H;++r)for(int i=0;i<w;++i)blend(p,st,x+i,y+r,c,g[r*w+i]);
  x+=w+(a->theme.style==MINIMAL?CAPS_TRACK:0);
 }
 return x-x0;
}
static void centred(const struct app *a,u32 *p,unsigned st,int cx,int y,const char *s,u32 c){
 text(a,p,st,cx-text_w(a,s)/2,y,s,c);
}
static char *put(char *e,const char *s){while(*s)*e++=*s++;*e=0;return e;}
static char *put_n(char *e,unsigned v){
 char d[12];int n=0;
 do d[n++]=(char)('0'+v%10),v/=10;while(v);
 while(n)*e++=d[--n];
 *e=0;return e;
}

/* "Round 3" at the left and "Best 7" at the right, the names grey, as Q2 Pod's hud(). */
static void hud(struct app *a,u32 *p,unsigned st){
 const struct theme *t=&a->theme;
 char round[12],best[12];
 put_n(round,a->length?a->length:1);put_n(best,a->best);
 int x=HUD_X+text(a,p,st,HUD_X,HUD_Y,"Round",t->muted)+6;
 text(a,p,st,x,HUD_Y,round,t->title);
 x=WIDTH-HUD_X-text_w(a,best);
 text(a,p,st,x,HUD_Y,best,t->title);
 x-=6+text_w(a,"Best");
 text(a,p,st,x,HUD_Y,"Best",t->muted);
}

/* The source image is reduced to a compact indexed sprite once at build time.
 * Recolor its gray fills at runtime; black ink and pink ears remain consistent. */
static void cat(struct app *a,u32 *p,unsigned st){
 const int x0=HUB_X-CAT_SPRITE_W*CAT_SCALE/2,y0=HUB_Y-CAT_SPRITE_H*CAT_SCALE/2;
 const u32 palette[6]={
  0,
  0xff28282du,                   /* outlines */
  a->dark_mode?0xfff6f6f6u:0xffb0b0b0u,
  a->dark_mode?0xffffffffu:0xfff7f7f7u,
  0xffffabbdu,                   /* pink ears */
  a->dark_mode?0xffdddddfu:0xff777777u
 };
 unsigned offset=0;
 for(unsigned i=0;i<sizeof cat_sprite_rle;i+=2){
  unsigned count=cat_sprite_rle[i],index=cat_sprite_rle[i+1];
  while(count--){
   if(index){
    int x=x0+(int)(offset%CAT_SPRITE_W)*CAT_SCALE,y=y0+(int)(offset/CAT_SPRITE_W)*CAT_SCALE;
    box(p,st,x,y,CAT_SCALE,CAT_SCALE,palette[index]);
   }
   ++offset;
  }
 }
}
/* Flat pads on a flat board: a lit pad brightens, the wheel's pick is ringed in the theme's ink (its
 * accent can be a pad's own colour), and the cat sits in its own framed hub. No gloss, glow or gradient. */
static void board(struct app *a,u32 *p,unsigned st){
 const struct theme *t=&a->theme;
 framed(p,st,BOARD_X,BOARD_Y,BOARD_W,BOARD_H,BOARD_RADIUS,t->panel,t->frame);
 for(int i=0;i<4;++i){
  u32 base=i==2&&a->dark_mode?0xff33363du:colors[i]; /* Black stays clear of a dark board */
  if(a->phase==INPUT&&a->selected==i)
   rrect(p,st,px[i]-3,py[i]-3,PAD_W+6,PAD_H+6,PAD_RADIUS+3,t->title);
  rrect(p,st,px[i],py[i],PAD_W,PAD_H,PAD_RADIUS,mix(base,bright[i],a->shine[i]));
 }
 disc(p,st,HUB_X,HUB_Y,HUB_MOAT,t->panel);
 disc(p,st,HUB_X,HUB_Y,HUB_RADIUS,t->frame);
 disc(p,st,HUB_X,HUB_Y,HUB_RADIUS-1,mix(t->panel,t->frame,a->dark_mode?160:90));
 cat(a,p,st);
}
/* Under the board: what to do now, then a dot for each step of the round, Home's dots. */
static void status(struct app *a,u32 *p,unsigned st){
 const struct theme *t=&a->theme;
 const char *s=a->phase==SHOW?"Watch":a->phase==CELEBRATE?"Level up!":"Your turn";
 centred(a,p,st,WIDTH/2,STATUS_Y,s,a->phase==CELEBRATE?t->pick:t->muted);
 unsigned done=a->phase==SHOW?a->shown:a->phase==CELEBRATE?a->length:a->entered;
 unsigned slots=a->length<14?a->length:14;
 if(!slots)slots=1;
 int x=(WIDTH-(int)slots*DOT_STEP+DOT_STEP-DOT)/2;
 for(unsigned i=0;i<slots;++i){
  unsigned index=a->length<=14?i:i*a->length/slots;
  rrect(p,st,x+(int)i*DOT_STEP,DOTS_Y,DOT,DOT,DOT/2,index<done?t->title:t->frame);
 }
}

/* Q2 Pod's pop-up (popup() in its games.c): a title, then a hint, or the rows with the chosen one
 * as the theme draws its selection: Classic's band, Modern's accent bar, Minimal's dot. */
static void popup_box(struct app *a,u32 *p,unsigned st,int y,int h,int chosen){
 const struct theme *t=&a->theme;
 if(t->style==CLASSIC){
  u32 rows[HEIGHT];
  int band=chosen>=0?menu_top(a)+chosen*POP_ROW:h;
  for(int i=0;i<h;++i)
   rows[i]=i>=band&&i<band+POP_ROW?t->band[i-band]:i<POP_ROW?t->head[i]:i==POP_ROW?t->frame:t->box;
  rrect(p,st,POP_X-1,y-1,POP_W+2,h+2,13,t->frame);
  rrect_rows(p,st,POP_X,y,POP_W,h,12,rows,h);
 }else{
  int r=t->style==MINIMAL?6:12;
  framed(p,st,POP_X,y,POP_W,h,r,t->box,t->frame);
 }
}
static int title_y(const struct app *a,int y){return y+(a->theme.style==CLASSIC?0:4)+(POP_ROW-FONT_H)/2;}
static void popup_hint(struct app *a,u32 *p,unsigned st,const char *title,const char *hint){
 const struct theme *t=&a->theme;
 int h=POP_ROW+44,y=(HEIGHT-h)/2;
 dim(p,st);
 popup_box(a,p,st,y,h,-1);
 centred(a,p,st,WIDTH/2,title_y(a,y),title,t->title);
 int top=y+POP_ROW+(t->style==CLASSIC?1:2),room=h-(top-y);
 centred(a,p,st,WIDTH/2,top+(room-FONT_H)/2,hint,t->muted);
}
static void popup_menu(struct app *a,u32 *p,unsigned st){
 const struct theme *t=&a->theme;
 int y=menu_y(a),h=menu_h(a);
 char sound[16];put(put(sound,"Sound: "),a->sounds?"On":"Off");
 const char *rows[MENU_ROWS]={"Continue",sound,"Quit"};
 dim(p,st);
 popup_box(a,p,st,y,h,a->choice);
 centred(a,p,st,WIDTH/2,title_y(a,y),"Paused",t->title);
 for(int i=0;i<MENU_ROWS;++i){
  int ry=y+menu_top(a)+i*POP_ROW,on=i==a->choice,ty=ry+(POP_ROW-FONT_H)/2;
  u32 ink=on?t->pick_text:t->text;
  if(t->style==MINIMAL){
   if(on)rrect(p,st,POP_X+DOT_X,ry+(POP_ROW-DOT)/2,DOT,DOT,DOT/2,t->pick);
   text(a,p,st,POP_X+TEXT_X,ty,rows[i],ink);
  }else if(t->style==CLASSIC){
   text(a,p,st,POP_X+16,ty,rows[i],ink);
  }else{
   if(on)rrect(p,st,POP_X+10,ry+1,POP_W-20,POP_ROW-2,6,t->pick);
   centred(a,p,st,WIDTH/2,ty,rows[i],ink);
  }
 }
}

void app_frame(struct app *a,u32 *p,unsigned st,unsigned ms){
 const struct theme *t=&a->theme;
 if(a->phase!=PAUSED)step(a,ms);
 box(p,st,0,0,WIDTH,HEIGHT,t->bg);
 hud(a,p,st);
 board(a,p,st);
 if(a->phase==READY)popup_hint(a,p,st,"Diamond Says","Centre: play");
 else if(a->phase==LOST)popup_hint(a,p,st,a->record?"New best!":"Game over","Centre: play again");
 else if(a->phase==FINISHED)popup_hint(a,p,st,"You did it!","Centre: play again");
 else if(a->phase==PAUSED)popup_menu(a,p,st);
 else status(a,p,st);
}

int app_animated(const struct app *a){
 if(a->phase==PAUSED)return 0;
 if(a->phase==SHOW||a->phase==CELEBRATE)return 1;
 for(int i=0;i<4;++i)if(a->shine[i])return 1;
 return 0;
}
