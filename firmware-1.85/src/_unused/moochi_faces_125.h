#pragma once
// ============================================================
//  moochi_faces_125.h
//  کتابخانه‌ی رسمیِ ۱۲۵ چهره‌ی موچی — تولیدشده به‌صورت خودکار
//  ۵ سبک چشم × ۵ پالت رنگ × ۵ چیدمان = ۱۲۵ چهره
//  مستقل از سخت‌افزار: فقط یک بافر RGB565 و تابع رندر.
// ============================================================
#include <stdint.h>
#include <math.h>
#include <string.h>

#ifndef MOOCHI_FB_W
#define MOOCHI_FB_W 320
#endif
#ifndef MOOCHI_FB_H
#define MOOCHI_FB_H 240
#endif

#define RGB565(r,g,b) ((uint16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3)))

static inline uint16_t face_swap16(uint16_t c){ return (c>>8)|(c<<8); }

// ---------- ۵ سبک چشم ----------
typedef enum {
  EYE_ROUND_GLOSSY = 0,  // چشم دایره‌ای براق
  EYE_ROUND_RECT   = 1,  // مستطیل گرد (کلاسیک موچی)
  EYE_ARC_BRUSH    = 2,  // قوس قلم‌مویی خوشحال
  EYE_HEART        = 3,  // قلب
  EYE_SLEEPY       = 4,  // پلک افتاده
} FaceEye;

// ---------- ۵ پالت رنگ ----------
typedef struct { uint16_t eye; uint16_t mouth; } FacePalette;
const FacePalette PAL_SOFT_VIOLET = { RGB565(198,188,244), RGB565( 62,232,200) }; // بنفش ملایم
const FacePalette PAL_PURE_WHITE = { RGB565(244,242,255), RGB565( 46,214,208) }; // سفید خالص
const FacePalette PAL_MINT_TEA = { RGB565( 62,232,200), RGB565(198,188,244) }; // نعنایی
const FacePalette PAL_WARM_GOLD = { RGB565(255,205, 60), RGB565(255,105,160) }; // طلایی گرم
const FacePalette PAL_SKY_BLUE = { RGB565(130,200,255), RGB565(255,150,190) }; // آبی آسمانی

// ---------- ۵ چیدمان ----------
typedef struct { int dx, ew, eh, pupR; } FaceLayout;
static const FaceLayout LAYOUTS_125[5] = {
  { 58, 64,  84, 14},  // NARROW
  { 78, 64,  84, 14},  // CLASSIC
  { 92, 64,  84, 14},  // WIDE
  { 78, 78, 100, 17},  // BIG
  { 78, 52,  68, 11},  // PETITE
};
static const char* LAYOUT_NAMES_125[5] =
  { "NARROW", "CLASSIC", "WIDE", "BIG", "PETITE" };
static const char* EYE_NAMES_125[5] =
  { "ROUND_GLOSSY", "ROUND_RECT", "ARC_BRUSH", "HEART_EYES", "SLEEPY_LID" };
static const char* PAL_NAMES_125[5] =
  { "SOFT_VIOLET", "PURE_WHITE", "MINT_TEA", "WARM_GOLD", "SKY_BLUE" };

// ---------- توصیف‌گر چهره ----------
typedef struct { uint8_t id, eye, pal, layout; } FaceDef;
static const FaceDef FACES_125[125] = {
  {  1, (FaceEye)0, (FacePal)0, (FaceLayout)0}, // #001 ROUND_GLOSSY + SOFT_VIOLET + NARROW,
  {  2, (FaceEye)0, (FacePal)0, (FaceLayout)1}, // #002 ROUND_GLOSSY + SOFT_VIOLET + CLASSIC,
  {  3, (FaceEye)0, (FacePal)0, (FaceLayout)2}, // #003 ROUND_GLOSSY + SOFT_VIOLET + WIDE,
  {  4, (FaceEye)0, (FacePal)0, (FaceLayout)3}, // #004 ROUND_GLOSSY + SOFT_VIOLET + BIG,
  {  5, (FaceEye)0, (FacePal)0, (FaceLayout)4}, // #005 ROUND_GLOSSY + SOFT_VIOLET + PETITE,
  {  6, (FaceEye)0, (FacePal)1, (FaceLayout)0}, // #006 ROUND_GLOSSY + PURE_WHITE + NARROW,
  {  7, (FaceEye)0, (FacePal)1, (FaceLayout)1}, // #007 ROUND_GLOSSY + PURE_WHITE + CLASSIC,
  {  8, (FaceEye)0, (FacePal)1, (FaceLayout)2}, // #008 ROUND_GLOSSY + PURE_WHITE + WIDE,
  {  9, (FaceEye)0, (FacePal)1, (FaceLayout)3}, // #009 ROUND_GLOSSY + PURE_WHITE + BIG,
  { 10, (FaceEye)0, (FacePal)1, (FaceLayout)4}, // #010 ROUND_GLOSSY + PURE_WHITE + PETITE,
  { 11, (FaceEye)0, (FacePal)2, (FaceLayout)0}, // #011 ROUND_GLOSSY + MINT_TEA + NARROW,
  { 12, (FaceEye)0, (FacePal)2, (FaceLayout)1}, // #012 ROUND_GLOSSY + MINT_TEA + CLASSIC,
  { 13, (FaceEye)0, (FacePal)2, (FaceLayout)2}, // #013 ROUND_GLOSSY + MINT_TEA + WIDE,
  { 14, (FaceEye)0, (FacePal)2, (FaceLayout)3}, // #014 ROUND_GLOSSY + MINT_TEA + BIG,
  { 15, (FaceEye)0, (FacePal)2, (FaceLayout)4}, // #015 ROUND_GLOSSY + MINT_TEA + PETITE,
  { 16, (FaceEye)0, (FacePal)3, (FaceLayout)0}, // #016 ROUND_GLOSSY + WARM_GOLD + NARROW,
  { 17, (FaceEye)0, (FacePal)3, (FaceLayout)1}, // #017 ROUND_GLOSSY + WARM_GOLD + CLASSIC,
  { 18, (FaceEye)0, (FacePal)3, (FaceLayout)2}, // #018 ROUND_GLOSSY + WARM_GOLD + WIDE,
  { 19, (FaceEye)0, (FacePal)3, (FaceLayout)3}, // #019 ROUND_GLOSSY + WARM_GOLD + BIG,
  { 20, (FaceEye)0, (FacePal)3, (FaceLayout)4}, // #020 ROUND_GLOSSY + WARM_GOLD + PETITE,
  { 21, (FaceEye)0, (FacePal)4, (FaceLayout)0}, // #021 ROUND_GLOSSY + SKY_BLUE + NARROW,
  { 22, (FaceEye)0, (FacePal)4, (FaceLayout)1}, // #022 ROUND_GLOSSY + SKY_BLUE + CLASSIC,
  { 23, (FaceEye)0, (FacePal)4, (FaceLayout)2}, // #023 ROUND_GLOSSY + SKY_BLUE + WIDE,
  { 24, (FaceEye)0, (FacePal)4, (FaceLayout)3}, // #024 ROUND_GLOSSY + SKY_BLUE + BIG,
  { 25, (FaceEye)0, (FacePal)4, (FaceLayout)4}, // #025 ROUND_GLOSSY + SKY_BLUE + PETITE,
  { 26, (FaceEye)1, (FacePal)0, (FaceLayout)0}, // #026 ROUND_RECT + SOFT_VIOLET + NARROW,
  { 27, (FaceEye)1, (FacePal)0, (FaceLayout)1}, // #027 ROUND_RECT + SOFT_VIOLET + CLASSIC,
  { 28, (FaceEye)1, (FacePal)0, (FaceLayout)2}, // #028 ROUND_RECT + SOFT_VIOLET + WIDE,
  { 29, (FaceEye)1, (FacePal)0, (FaceLayout)3}, // #029 ROUND_RECT + SOFT_VIOLET + BIG,
  { 30, (FaceEye)1, (FacePal)0, (FaceLayout)4}, // #030 ROUND_RECT + SOFT_VIOLET + PETITE,
  { 31, (FaceEye)1, (FacePal)1, (FaceLayout)0}, // #031 ROUND_RECT + PURE_WHITE + NARROW,
  { 32, (FaceEye)1, (FacePal)1, (FaceLayout)1}, // #032 ROUND_RECT + PURE_WHITE + CLASSIC,
  { 33, (FaceEye)1, (FacePal)1, (FaceLayout)2}, // #033 ROUND_RECT + PURE_WHITE + WIDE,
  { 34, (FaceEye)1, (FacePal)1, (FaceLayout)3}, // #034 ROUND_RECT + PURE_WHITE + BIG,
  { 35, (FaceEye)1, (FacePal)1, (FaceLayout)4}, // #035 ROUND_RECT + PURE_WHITE + PETITE,
  { 36, (FaceEye)1, (FacePal)2, (FaceLayout)0}, // #036 ROUND_RECT + MINT_TEA + NARROW,
  { 37, (FaceEye)1, (FacePal)2, (FaceLayout)1}, // #037 ROUND_RECT + MINT_TEA + CLASSIC,
  { 38, (FaceEye)1, (FacePal)2, (FaceLayout)2}, // #038 ROUND_RECT + MINT_TEA + WIDE,
  { 39, (FaceEye)1, (FacePal)2, (FaceLayout)3}, // #039 ROUND_RECT + MINT_TEA + BIG,
  { 40, (FaceEye)1, (FacePal)2, (FaceLayout)4}, // #040 ROUND_RECT + MINT_TEA + PETITE,
  { 41, (FaceEye)1, (FacePal)3, (FaceLayout)0}, // #041 ROUND_RECT + WARM_GOLD + NARROW,
  { 42, (FaceEye)1, (FacePal)3, (FaceLayout)1}, // #042 ROUND_RECT + WARM_GOLD + CLASSIC,
  { 43, (FaceEye)1, (FacePal)3, (FaceLayout)2}, // #043 ROUND_RECT + WARM_GOLD + WIDE,
  { 44, (FaceEye)1, (FacePal)3, (FaceLayout)3}, // #044 ROUND_RECT + WARM_GOLD + BIG,
  { 45, (FaceEye)1, (FacePal)3, (FaceLayout)4}, // #045 ROUND_RECT + WARM_GOLD + PETITE,
  { 46, (FaceEye)1, (FacePal)4, (FaceLayout)0}, // #046 ROUND_RECT + SKY_BLUE + NARROW,
  { 47, (FaceEye)1, (FacePal)4, (FaceLayout)1}, // #047 ROUND_RECT + SKY_BLUE + CLASSIC,
  { 48, (FaceEye)1, (FacePal)4, (FaceLayout)2}, // #048 ROUND_RECT + SKY_BLUE + WIDE,
  { 49, (FaceEye)1, (FacePal)4, (FaceLayout)3}, // #049 ROUND_RECT + SKY_BLUE + BIG,
  { 50, (FaceEye)1, (FacePal)4, (FaceLayout)4}, // #050 ROUND_RECT + SKY_BLUE + PETITE,
  { 51, (FaceEye)2, (FacePal)0, (FaceLayout)0}, // #051 ARC_BRUSH + SOFT_VIOLET + NARROW,
  { 52, (FaceEye)2, (FacePal)0, (FaceLayout)1}, // #052 ARC_BRUSH + SOFT_VIOLET + CLASSIC,
  { 53, (FaceEye)2, (FacePal)0, (FaceLayout)2}, // #053 ARC_BRUSH + SOFT_VIOLET + WIDE,
  { 54, (FaceEye)2, (FacePal)0, (FaceLayout)3}, // #054 ARC_BRUSH + SOFT_VIOLET + BIG,
  { 55, (FaceEye)2, (FacePal)0, (FaceLayout)4}, // #055 ARC_BRUSH + SOFT_VIOLET + PETITE,
  { 56, (FaceEye)2, (FacePal)1, (FaceLayout)0}, // #056 ARC_BRUSH + PURE_WHITE + NARROW,
  { 57, (FaceEye)2, (FacePal)1, (FaceLayout)1}, // #057 ARC_BRUSH + PURE_WHITE + CLASSIC,
  { 58, (FaceEye)2, (FacePal)1, (FaceLayout)2}, // #058 ARC_BRUSH + PURE_WHITE + WIDE,
  { 59, (FaceEye)2, (FacePal)1, (FaceLayout)3}, // #059 ARC_BRUSH + PURE_WHITE + BIG,
  { 60, (FaceEye)2, (FacePal)1, (FaceLayout)4}, // #060 ARC_BRUSH + PURE_WHITE + PETITE,
  { 61, (FaceEye)2, (FacePal)2, (FaceLayout)0}, // #061 ARC_BRUSH + MINT_TEA + NARROW,
  { 62, (FaceEye)2, (FacePal)2, (FaceLayout)1}, // #062 ARC_BRUSH + MINT_TEA + CLASSIC,
  { 63, (FaceEye)2, (FacePal)2, (FaceLayout)2}, // #063 ARC_BRUSH + MINT_TEA + WIDE,
  { 64, (FaceEye)2, (FacePal)2, (FaceLayout)3}, // #064 ARC_BRUSH + MINT_TEA + BIG,
  { 65, (FaceEye)2, (FacePal)2, (FaceLayout)4}, // #065 ARC_BRUSH + MINT_TEA + PETITE,
  { 66, (FaceEye)2, (FacePal)3, (FaceLayout)0}, // #066 ARC_BRUSH + WARM_GOLD + NARROW,
  { 67, (FaceEye)2, (FacePal)3, (FaceLayout)1}, // #067 ARC_BRUSH + WARM_GOLD + CLASSIC,
  { 68, (FaceEye)2, (FacePal)3, (FaceLayout)2}, // #068 ARC_BRUSH + WARM_GOLD + WIDE,
  { 69, (FaceEye)2, (FacePal)3, (FaceLayout)3}, // #069 ARC_BRUSH + WARM_GOLD + BIG,
  { 70, (FaceEye)2, (FacePal)3, (FaceLayout)4}, // #070 ARC_BRUSH + WARM_GOLD + PETITE,
  { 71, (FaceEye)2, (FacePal)4, (FaceLayout)0}, // #071 ARC_BRUSH + SKY_BLUE + NARROW,
  { 72, (FaceEye)2, (FacePal)4, (FaceLayout)1}, // #072 ARC_BRUSH + SKY_BLUE + CLASSIC,
  { 73, (FaceEye)2, (FacePal)4, (FaceLayout)2}, // #073 ARC_BRUSH + SKY_BLUE + WIDE,
  { 74, (FaceEye)2, (FacePal)4, (FaceLayout)3}, // #074 ARC_BRUSH + SKY_BLUE + BIG,
  { 75, (FaceEye)2, (FacePal)4, (FaceLayout)4}, // #075 ARC_BRUSH + SKY_BLUE + PETITE,
  { 76, (FaceEye)3, (FacePal)0, (FaceLayout)0}, // #076 HEART_EYES + SOFT_VIOLET + NARROW,
  { 77, (FaceEye)3, (FacePal)0, (FaceLayout)1}, // #077 HEART_EYES + SOFT_VIOLET + CLASSIC,
  { 78, (FaceEye)3, (FacePal)0, (FaceLayout)2}, // #078 HEART_EYES + SOFT_VIOLET + WIDE,
  { 79, (FaceEye)3, (FacePal)0, (FaceLayout)3}, // #079 HEART_EYES + SOFT_VIOLET + BIG,
  { 80, (FaceEye)3, (FacePal)0, (FaceLayout)4}, // #080 HEART_EYES + SOFT_VIOLET + PETITE,
  { 81, (FaceEye)3, (FacePal)1, (FaceLayout)0}, // #081 HEART_EYES + PURE_WHITE + NARROW,
  { 82, (FaceEye)3, (FacePal)1, (FaceLayout)1}, // #082 HEART_EYES + PURE_WHITE + CLASSIC,
  { 83, (FaceEye)3, (FacePal)1, (FaceLayout)2}, // #083 HEART_EYES + PURE_WHITE + WIDE,
  { 84, (FaceEye)3, (FacePal)1, (FaceLayout)3}, // #084 HEART_EYES + PURE_WHITE + BIG,
  { 85, (FaceEye)3, (FacePal)1, (FaceLayout)4}, // #085 HEART_EYES + PURE_WHITE + PETITE,
  { 86, (FaceEye)3, (FacePal)2, (FaceLayout)0}, // #086 HEART_EYES + MINT_TEA + NARROW,
  { 87, (FaceEye)3, (FacePal)2, (FaceLayout)1}, // #087 HEART_EYES + MINT_TEA + CLASSIC,
  { 88, (FaceEye)3, (FacePal)2, (FaceLayout)2}, // #088 HEART_EYES + MINT_TEA + WIDE,
  { 89, (FaceEye)3, (FacePal)2, (FaceLayout)3}, // #089 HEART_EYES + MINT_TEA + BIG,
  { 90, (FaceEye)3, (FacePal)2, (FaceLayout)4}, // #090 HEART_EYES + MINT_TEA + PETITE,
  { 91, (FaceEye)3, (FacePal)3, (FaceLayout)0}, // #091 HEART_EYES + WARM_GOLD + NARROW,
  { 92, (FaceEye)3, (FacePal)3, (FaceLayout)1}, // #092 HEART_EYES + WARM_GOLD + CLASSIC,
  { 93, (FaceEye)3, (FacePal)3, (FaceLayout)2}, // #093 HEART_EYES + WARM_GOLD + WIDE,
  { 94, (FaceEye)3, (FacePal)3, (FaceLayout)3}, // #094 HEART_EYES + WARM_GOLD + BIG,
  { 95, (FaceEye)3, (FacePal)3, (FaceLayout)4}, // #095 HEART_EYES + WARM_GOLD + PETITE,
  { 96, (FaceEye)3, (FacePal)4, (FaceLayout)0}, // #096 HEART_EYES + SKY_BLUE + NARROW,
  { 97, (FaceEye)3, (FacePal)4, (FaceLayout)1}, // #097 HEART_EYES + SKY_BLUE + CLASSIC,
  { 98, (FaceEye)3, (FacePal)4, (FaceLayout)2}, // #098 HEART_EYES + SKY_BLUE + WIDE,
  { 99, (FaceEye)3, (FacePal)4, (FaceLayout)3}, // #099 HEART_EYES + SKY_BLUE + BIG,
  {100, (FaceEye)3, (FacePal)4, (FaceLayout)4}, // #100 HEART_EYES + SKY_BLUE + PETITE,
  {101, (FaceEye)4, (FacePal)0, (FaceLayout)0}, // #101 SLEEPY_LID + SOFT_VIOLET + NARROW,
  {102, (FaceEye)4, (FacePal)0, (FaceLayout)1}, // #102 SLEEPY_LID + SOFT_VIOLET + CLASSIC,
  {103, (FaceEye)4, (FacePal)0, (FaceLayout)2}, // #103 SLEEPY_LID + SOFT_VIOLET + WIDE,
  {104, (FaceEye)4, (FacePal)0, (FaceLayout)3}, // #104 SLEEPY_LID + SOFT_VIOLET + BIG,
  {105, (FaceEye)4, (FacePal)0, (FaceLayout)4}, // #105 SLEEPY_LID + SOFT_VIOLET + PETITE,
  {106, (FaceEye)4, (FacePal)1, (FaceLayout)0}, // #106 SLEEPY_LID + PURE_WHITE + NARROW,
  {107, (FaceEye)4, (FacePal)1, (FaceLayout)1}, // #107 SLEEPY_LID + PURE_WHITE + CLASSIC,
  {108, (FaceEye)4, (FacePal)1, (FaceLayout)2}, // #108 SLEEPY_LID + PURE_WHITE + WIDE,
  {109, (FaceEye)4, (FacePal)1, (FaceLayout)3}, // #109 SLEEPY_LID + PURE_WHITE + BIG,
  {110, (FaceEye)4, (FacePal)1, (FaceLayout)4}, // #110 SLEEPY_LID + PURE_WHITE + PETITE,
  {111, (FaceEye)4, (FacePal)2, (FaceLayout)0}, // #111 SLEEPY_LID + MINT_TEA + NARROW,
  {112, (FaceEye)4, (FacePal)2, (FaceLayout)1}, // #112 SLEEPY_LID + MINT_TEA + CLASSIC,
  {113, (FaceEye)4, (FacePal)2, (FaceLayout)2}, // #113 SLEEPY_LID + MINT_TEA + WIDE,
  {114, (FaceEye)4, (FacePal)2, (FaceLayout)3}, // #114 SLEEPY_LID + MINT_TEA + BIG,
  {115, (FaceEye)4, (FacePal)2, (FaceLayout)4}, // #115 SLEEPY_LID + MINT_TEA + PETITE,
  {116, (FaceEye)4, (FacePal)3, (FaceLayout)0}, // #116 SLEEPY_LID + WARM_GOLD + NARROW,
  {117, (FaceEye)4, (FacePal)3, (FaceLayout)1}, // #117 SLEEPY_LID + WARM_GOLD + CLASSIC,
  {118, (FaceEye)4, (FacePal)3, (FaceLayout)2}, // #118 SLEEPY_LID + WARM_GOLD + WIDE,
  {119, (FaceEye)4, (FacePal)3, (FaceLayout)3}, // #119 SLEEPY_LID + WARM_GOLD + BIG,
  {120, (FaceEye)4, (FacePal)3, (FaceLayout)4}, // #120 SLEEPY_LID + WARM_GOLD + PETITE,
  {121, (FaceEye)4, (FacePal)4, (FaceLayout)0}, // #121 SLEEPY_LID + SKY_BLUE + NARROW,
  {122, (FaceEye)4, (FacePal)4, (FaceLayout)1}, // #122 SLEEPY_LID + SKY_BLUE + CLASSIC,
  {123, (FaceEye)4, (FacePal)4, (FaceLayout)2}, // #123 SLEEPY_LID + SKY_BLUE + WIDE,
  {124, (FaceEye)4, (FacePal)4, (FaceLayout)3}, // #124 SLEEPY_LID + SKY_BLUE + BIG,
  {125, (FaceEye)4, (FacePal)4, (FaceLayout)4}, // #125 SLEEPY_LID + SKY_BLUE + PETITE
};

// ============================================================
//  رندرکننده — روی یک بافر RGB565 می‌کشد
// ============================================================
static uint16_t *face_fb = 0;

static inline void fpx(int x, int y, uint16_t c){
  if (x<0||x>=MOOCHI_FB_W||y<0||y>=MOOCHI_FB_H) return;
  face_fb[y*MOOCHI_FB_W+x] = face_swap16(c);
}
static void faceFill(uint16_t c){
  uint16_t s = face_swap16(c);
  for (int i=0;i<MOOCHI_FB_W*MOOCHI_FB_H;i++) face_fb[i]=s;
}
static void fCircle(int cx,int cy,int r,uint16_t c){
  uint16_t s=face_swap16(c); if(r<1)r=1;
  for(int y=cy-r;y<=cy+r;y++){int dy=y-cy;int w2=r*r-dy*dy;if(w2<0)continue;
    int w=(int)sqrtf((float)w2);
    for(int x=cx-w;x<=cx+w;x++)fpx(x,y,s);}
}
static void fRoundRect(int x,int y,int w,int h,int r,uint16_t c){
  uint16_t s=face_swap16(c); if(w<1||h<1)return;
  if(r>w/2)r=w/2; if(r>h/2)r=h/2; if(r<0)r=0;
  for(int j=0;j<h;j++){int ins=0;
    if(r>0){ if(j<r){int d=r-j; ins=r-(int)sqrtf((float)(r*r-d*d));}
             else if(j>=h-r){int d=j-(h-r-1); ins=r-(int)sqrtf((float)(r*r-d*d));}}
    for(int i=ins;i<w-ins;i++)fpx(x+i,y+j,s);}
}
static void fCurve(int cx,int cy,int halfW,int depth,int thick,uint16_t c){
  if(halfW<1)halfW=1;
  for(int x=cx-halfW;x<=cx+halfW;x++){
    float t=(float)(x-cx)/halfW;
    fCircle(x, cy+(int)(depth*(1.0f-t*t)), thick, c);
  }
}
static void fHeart(int cx,int cy,int size,uint16_t c){
  uint16_t s=face_swap16(c); if(size<6)size=6;
  float sc=size/2.4f; int span=(int)(1.3f*sc)+2;
  for(int y=cy-span;y<=cy+span;y++)for(int x=cx-span;x<=cx+span;x++){
    float nx=(x-cx)/sc, ny=-(float)(y-cy)/sc-0.2f;
    float a=nx*nx+ny*ny-1.0f;
    if(a*a*a-nx*nx*ny*ny*ny<=0) fpx(x,y,s);
  }
}

// ---------- کشیدن یک چهره‌ی مشخص ----------
// id چهره: 1..125
static void drawMoochiFace125(int faceId, int gazeX=0, int gazeY=0,
                              int open=100, uint16_t *fb=0){
  if (fb) face_fb = fb;
  if (!face_fb) return;
  if (faceId < 1 || faceId > 125) faceId = 1;
  const FaceDef &F  = FACES_125[faceId-1];
  const FaceLayout &L = LAYOUTS_125[F.layout];
  const FacePalette &P = (&PAL_SOFT_VIOLET)[F.pal];
  int cx = MOOCHI_FB_W/2, cy = MOOCHI_FB_H/2 - 10;
  faceFill(RGB565(0,0,0));
  int ew=L.ew, eh=(L.eh*open)/100; if(eh<4)eh=4;
  int pr=(L.pupR*open)/100; if(pr<1)pr=1;

  for (int sd=-1; sd<=1; sd+=2){
    int ex = cx + sd*L.dx + gazeX;
    int ey = cy + gazeY;
    switch ((FaceEye)F.eye){
      case EYE_ROUND_GLOSSY:
        fCircle(ex, ey, ew/2, P.eye);
        fCircle(ex, ey, pr, RGB565(0,0,0));
        if (open>45) fCircle(ex-pr/3, ey-pr/2, pr/3, P.eye);
        break;
      case EYE_ROUND_RECT:
        fRoundRect(ex-ew/2, ey-eh/2, ew, eh, 18, P.eye);
        fCircle(ex+gazeX/2, ey+gazeY/2, pr, RGB565(0,0,0));
        break;
      case EYE_ARC_BRUSH:
        fCurve(ex, ey+6, ew/2, 14, 9, P.eye);
        break;
      case EYE_HEART:
        fHeart(ex, ey, pr*3, P.eye);
        break;
      case EYE_SLEEPY:
        fRoundRect(ex-ew/2, ey-eh/4, ew, eh/2, 12, P.eye);
        fCurve(ex, ey+eh/4, ew/2, -6, 4, P.eye);
        break;
    }
  }
  // دهان — همیشه هم‌رنگ پالت
  fCurve(cx, cy+52, 30, 12, 5, P.mouth);
}

// نام قابل‌چاپ چهره (برای لاگ)
static inline void moochiFaceName125(int id, char *buf, int bufLen){
  if (id<1||id>125) id=1;
  const FaceDef &F = FACES_125[id-1];
  snprintf(buf, bufLen, "#%03d %s / %s / %s",
           id, EYE_NAMES_125[F.eye], PAL_NAMES_125[F.pal], LAYOUT_NAMES_125[F.layout]);
}
