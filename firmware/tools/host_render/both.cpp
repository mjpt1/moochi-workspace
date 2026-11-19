#include "Arduino.h"
uint32_t g_millis = 100000;
#include "../../src/moochi_gfx.h"
static uint16_t bandMem[PHYS_W * 140];
uint16_t *bandBuf = bandMem;
int bandH = 140, bandN = 2, bandY0 = 0, vBandY0 = 0, vBandY1 = 0;
int16_t vx2p[LCD_W], vy2p[LCD_H];
#include "../../src/moochi_states.h"
#include "../../src/moochi_neon.h"
int main(){ gfxInitTables(); gfxSetBand(0); NeonParams P; neonDraw(P); drawFloaters(); drawShutter(0.3f); printf("both ok\n"); }
