#include "Arduino.h"
uint32_t g_millis = 100000;
#include "../../src/moochi_gfx.h"
static uint16_t bandMem[PHYS_W * 140];
uint16_t *bandBuf = bandMem;
int bandH = 140, bandN = 2, bandY0 = 0, vBandY0 = 0, vBandY1 = 0;
int16_t vx2p[LCD_W], vy2p[LCD_H];
#include "../../src/moochi_neon.h"
#include <chrono>
static uint16_t frame[PHYS_W*PHYS_H];
void render(const NeonParams& P){ for(int b=0;b<bandN;b++){ gfxSetBand(b); neonDraw(P);
  for(int i=0;i<PHYS_W*bandH;i++) frame[bandY0*PHYS_W+i]=bandBuf[i]; } }
void save(const char* fn){ FILE*f=fopen(fn,"wb"); fprintf(f,"P6\n%d %d\n255\n",PHYS_W,PHYS_H);
  for(int i=0;i<PHYS_W*PHYS_H;i++){ uint16_t s=frame[i]; uint16_t c=(s>>8)|(s<<8);
  fputc(((c>>11)&31)*255/31,f); fputc(((c>>5)&63)*255/63,f); fputc((c&31)*255/31,f);} fclose(f); }
int main(){ gfxInitTables(); NeonParams P; char fn[64];
  const char* nm[]={"normal","happy","surprise","sleepy","asleep","dizzy","sad","cry","angry","curious"};
  for(int k=0;k<10;k++){ P=NeonParams(); P.kind=(NeonKind)k; P.t=0.6f; render(P); snprintf(fn,64,"out4/n_%02d_%s.ppm",k,nm[k]); save(fn);}
  P=NeonParams(); P.open=0.4f; render(P); save("out4/n_10_blink.ppm");
  P=NeonParams(); P.gx=12; P.gy=6; render(P); save("out4/n_11_look.ppm");
  auto a=std::chrono::steady_clock::now(); for(int i=0;i<300;i++){P=NeonParams(); render(P);} auto b=std::chrono::steady_clock::now();
  printf("neon %.3f ms/frame host\n", std::chrono::duration<double,std::milli>(b-a).count()/300);
  for(int k=5;k<10;k++){ auto a2=std::chrono::steady_clock::now(); for(int i=0;i<200;i++){P=NeonParams(); P.kind=(NeonKind)k; P.t=0.6f; render(P);} auto b2=std::chrono::steady_clock::now();
    printf("%s %.3f ms\n", nm[k], std::chrono::duration<double,std::milli>(b2-a2).count()/200); }
}
