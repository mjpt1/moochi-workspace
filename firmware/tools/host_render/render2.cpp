#include "Arduino.h"
#include <chrono>
uint32_t g_millis = 100000;
#include "../../src/moochi_gfx.h"
static uint16_t bandMem[PHYS_W * 140];
uint16_t *bandBuf = bandMem;
int bandH = 140, bandN = 2, bandY0 = 0, vBandY0 = 0, vBandY1 = 0;
int16_t vx2p[LCD_W], vy2p[LCD_H];
#include "../../src/moochi_states.h"
static uint16_t frame[PHYS_W*PHYS_H];

template<class F> void renderAll(F draw){
  for(int b=0;b<bandN;b++){ gfxSetBand(b); draw();
    for(int i=0;i<PHYS_W*bandH;i++) frame[bandY0*PHYS_W+i]=bandBuf[i]; }
}
void savePPM(const char* fn){
  FILE*f=fopen(fn,"wb"); fprintf(f,"P6\n%d %d\n255\n",PHYS_W,PHYS_H);
  for(int i=0;i<PHYS_W*PHYS_H;i++){ uint16_t s=frame[i]; uint16_t c=(s>>8)|(s<<8);
    unsigned char r=((c>>11)&31)*255/31, g=((c>>5)&63)*255/63, b=(c&31)*255/31; fputc(r,f);fputc(g,f);fputc(b,f);} fclose(f);
}
void drawMood(Mood m, float t, int o){
  mood = m;
  switch(m){
    case M_IDLE: drawIdle(o); break; case M_HAPPY: drawHappy(o); break;
    case M_PET: drawPet(t); break; case M_LOVE: drawLove(t); break;
    case M_DIZZY: drawDizzy(t); break; case M_HURT: drawHurt(t,o); break;
    case M_SURPRISE: drawSurprise(t); break; case M_SLEEPY: drawSleepy(t); break;
    case M_ASLEEP: drawAsleep(t); break; case M_CURIOUS: drawCurious(t,o); break;
    case M_LISTEN: drawListen(t,o); break; case M_ANGRY: drawAngry(t,o); break;
    case M_KISS: drawKiss(t); break; case M_CRY: drawCry(t); break;
    case M_EYEROLL: drawEyeRoll(t); break; case M_HEARTHAND: drawHeartHand(t); break;
    case M_FEED: drawFeed(t); break; case M_RAINBOW: drawRainbow(t); break;
    case M_THINK: drawThink(t,o); break; case M_SHY: drawShy(t,o); break;
    case M_EXCITED: drawExcited(t); break; case M_YAWN: drawYawn(t); break;
  }
}
int main(){
  gfxInitTables();
  char fn[64];
  for(int m=0;m<22;m++){ renderAll([&]{ drawMood((Mood)m, 1.2f, 100); }); snprintf(fn,64,"out2/m%02d.ppm",m); savePPM(fn); }
  for(int i=0;i<CATALOG_N;i++){ renderAll([&]{ drawRecipe(CATALOG[i],0,0,0,0,100,1.2f);
      if (CATALOG[i].flags & FR_BOW) drawBow(CX - EYE_DX - 34, CY - 66, 0); });
    snprintf(fn,64,"out2/c%02d.ppm",i); savePPM(fn); }
  printf("ok %d catalog\n", CATALOG_N);
  auto T=[&](const char* nm, auto fn){ auto a=std::chrono::steady_clock::now(); for(int k=0;k<200;k++) renderAll(fn);
    auto b=std::chrono::steady_clock::now(); printf("%-12s %.3f ms/frame (host)\n", nm, std::chrono::duration<double,std::milli>(b-a).count()/200); };
  T("idle", [&]{ drawMood(M_IDLE,1.2f,100); });
  T("dizzy", [&]{ drawMood(M_DIZZY,1.2f,100); });
  T("hand40", [&]{ drawRecipe(CATALOG[39],0,0,0,0,100,1.2f); });
  T("wait63", [&]{ drawRecipe(CATALOG[62],0,0,0,0,100,1.2f); });
}
