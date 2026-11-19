#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include "moochi_gfx.h"     // DISPLAY_ROUND (از platformio.ini) و اندازه‌ي صفحه
#if DISPLAY_ROUND
#include <Adafruit_GC9A01A.h>
#ifndef ST77XX_BLACK
#define ST77XX_BLACK 0x0000
#endif
#else
#include <Adafruit_ST7789.h>
#endif
#include <Preferences.h>
#include <esp_heap_caps.h>
#include "boot_gif.h"       // گیف شروع (mahsa_world.gif)

// ============================================
//  موچی — test 08 : چهره‌ي کامل روی نمایشگر ۱٫۶۹ اینچ (+ چشم نئونی ، ۱۱ مهر)
// ============================================
// ۱۱ مهر ۱۴۰۵. موتور چهره‌ي کامل از firmware-1.85 آمد:
//   moochi_face.h     ابزار کشیدن ، چشم ، ابرو ، دهان ، دست ، اضافات
//   moochi_catalog.h  ۷۴ چهره‌ي کاتالوگ
//   moochi_states.h   ۲۲ حالت رفتاری + پلک + نگاه
//   moochi_gfx.h      لایه‌ي تازه: کشیدن نواری + نگاشت ۳۶۰ → ۲۴۰
// پیش‌نمایش همه‌ي چهره‌ها روی کامپیوتر: tools/host_render
//
// منطق حالت‌ها ساده شده (لمس و میکروفون هنوز وصل نیستند):
//   تکان شدید      → سرگیجه
//   حرکت ناگهانی    → جا خورد
//   کج کردن         → چشم‌ها و صورت به آن طرف می‌روند
//   ۶۰ ثانیه بی‌حرکت → خواب‌آلود ← خواب ؛ حرکت → بیدار
//   هر ۲۵ تا ۴۵ ثانیه → یک چهره‌ي تصادفی از کاتالوگ (زنده بودن)
//
// فرمان‌ها در مانیتور سریال (یک حرف بزن):
//   p = رژه روشن/خاموش (همه‌ي چهره‌ها پشت سر هم)
//   c = رژه‌ي کاتالوگ (۷۴) یا حالت‌ها (۲۲)
//   n = بعدی   b = قبلی
//
// سیم‌کشی (۱۱ آبان ۱۴۰۵ — docs/سیم‌کشی-ربات-پت):
//   نمایشگر ۱٫۶۹: SCL→G14  SDA→G13  DC→G26  CS→G15  RES,VCC,BLK→3V3  GND→زمین
//   شتاب‌سنج و فاصله‌سنج: SDA→G33  SCL→G32  VCC→3V3
//   آمپ: BCLK→G27  LRC→G25  DIN→G23
// ============================================

uint16_t *bandBuf = nullptr;       // در setup از heap گرفته می‌شود
int      bandH = 140, bandN = 2;
int      bandY0 = 0, vBandY0 = 0, vBandY1 = 0;
int16_t  vx2p[LCD_W], vy2p[LCD_H];
#include "moochi_states.h"  // خودش moochi_face.h و moochi_catalog.h را می‌آورد
#include "moochi_neon.h"    // ۱۱ مهر: چشم‌های نئونی صورتی (سبک تازه‌ي مهسن)
#include "moochi_neon_cat.h" // ۱۲ مهر: ۷۴ چهره‌ي کاتالوگ هم نئونی

// ---------- نمایشگر ----------
#if DISPLAY_ROUND
// ۱۲ مهر: نمایشگر گرد GC9A01 ، ۷ پایه (VCC GND SCL SDA DC CS RST) ، بدون نور پشت جدا.
// CS و RST وصل نمی‌شوند: پشت ماژول نوشته بدون آن‌ها کار می‌کند (CS با مقاومت R8 پایین
// کشیده شده) و کتابخانه برای RST=-1 «ریست نرم‌افزاری» می‌فرستد. پایه‌هایشان (G27 ، G25)
// برای صدا آزاد شد.
const int PIN_SCK = 14, PIN_MOSI = 13, PIN_DC = 26;
uint8_t SCREEN_ROTATION = 0;   // چرخش صفحه: با فرمان o در مانیتور عوض و ذخیره می‌شود
Adafruit_GC9A01A tft(-1, PIN_DC, -1);
#elif ST7789_CS15
// ۱۱ آبان: نمایشگر ۱٫۶۹ با CS روی G15 (بعد از بازبینی؛ بدون CS هر لرزش روی SCL تصویر را برای همیشه به هم می‌ریخت).
//   RES → 3V3 (کتابخانه SWRESET می‌فرستد). G27 و G25 مال صدا می‌مانند.
const int PIN_SCK = 14, PIN_MOSI = 13, PIN_DC = 26, PIN_CS = 15;
const uint8_t SCREEN_ROTATION = 2;
Adafruit_ST7789 tft(PIN_CS, PIN_DC, -1);
#elif ST7789_NO_CS
// ۱۹ مهر: نمایشگر ۱٫۶۹ بدون CS و RES تا G27 و G25 برای صدا آزاد شوند.
//   CS → ریل − (همیشه انتخاب‌شده — تنها دستگاه روی این باس SPI است)
//   RES → ریل + (کتابخانه در init اول «ریست نرم‌افزاری» SWRESET می‌فرستد — از Adafruit_ST7789.cpp چک شد)
const int PIN_SCK = 14, PIN_MOSI = 13, PIN_DC = 26;
const uint8_t SCREEN_ROTATION = 2;   // ۱۱ مهر: با 0 سر و ته بود
Adafruit_ST7789 tft(-1, PIN_DC, -1);
#else
const int PIN_SCK = 14, PIN_MOSI = 13, PIN_RST = 25, PIN_DC = 26, PIN_CS = 27;
const uint8_t SCREEN_ROTATION = 2;   // ۱۱ مهر: با 0 سر و ته بود
Adafruit_ST7789 tft(PIN_CS, PIN_DC, PIN_RST);
#endif
#ifndef ST7789_NO_CS
#define ST7789_NO_CS 0
#endif
#ifndef ST7789_CS15
#define ST7789_CS15 0
#endif
#define AUDIO_ENABLED (DISPLAY_ROUND || ST7789_NO_CS || ST7789_CS15)

// ---------- شتاب‌سنج ----------
const int PIN_SDA = 33, PIN_SCL = 32;
const uint8_t MPU_ADDR = 0x68;
bool imuOk = false;

// کالیبره‌ي جهت (از test 07). برای تکرار: یک بار true ، آپلود ، بعد false.
const bool FORCE_CALIBRATION = false;
struct Calib { uint32_t magic; float g0[3], ex[3], ey[3]; };
const uint32_t CALIB_MAGIC = 0x4D4F4F31;   // "MOO1"
Calib cal;
bool calOk = false;
float restNow[3] = {0, 0, 1};

// ============================================
//  گیف شروع
// ============================================
static uint16_t lineBuf[240];

uint32_t drawGifFrame(int k) {
  uint32_t off = BOOT_GIF_IDX[k][0];
  int x = BOOT_GIF_IDX[k][1], y = BOOT_GIF_IDX[k][2];
  int w = BOOT_GIF_IDX[k][3], h = BOOT_GIF_IDX[k][4];
  uint32_t dur = BOOT_GIF_IDX[k][5];
  const uint8_t *p = BOOT_GIF_DATA + off;
  // ۱۲ مهر: گیف ۲۸۰ ردیف است ؛ روی صفحه‌ي ۲۴۰ ردیفی ، ۲۰ ردیف بالا و پایین بریده می‌شود
  const int GOY = (BOOT_GIF_H - PHYS_H) / 2;
  int ya = y - GOY, yb = y + h - 1 - GOY;
  if (ya < 0) ya = 0;
  if (yb > PHYS_H - 1) yb = PHYS_H - 1;
  tft.startWrite();
  if (yb >= ya) tft.setAddrWindow(x, ya, w, yb - ya + 1);
  uint32_t total = (uint32_t)w * h, done = 0;
  int col = 0, row = 0;
  while (done < total) {
    uint8_t n = pgm_read_byte(p++);
    uint16_t c = BOOT_GIF_PAL[pgm_read_byte(p++)];
    while (n-- && done < total) {
      lineBuf[col++] = c;
      done++;
      if (col == w) {
        int sy = y + row - GOY;
        if (sy >= ya && sy <= yb) tft.writePixels(lineBuf, w);
        col = 0; row++;
      }
    }
  }
  tft.endWrite();
  return dur;
}

void playBootAnimation(uint32_t holdLastMs) {
  for (int k = 0; k < BOOT_GIF_FRAMES; k++) {
    uint32_t t0 = millis();
    uint32_t dur = drawGifFrame(k);
    uint32_t dt = millis() - t0;
    if (dt < dur) delay(dur - dt);
  }
  delay(holdLastMs);
}

// ============================================
//  شتاب‌سنج (MPU6500)
// ============================================
bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg); Wire.write(val);
  return Wire.endTransmission() == 0;
}
int readReg(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)1) != 1) return -1;
  return Wire.read();
}
bool setupImu() {
  int who = readReg(0x75);
  if (who < 0) return false;
  Serial.print("شتاب‌سنج پیدا شد ، شناسه 0x");
  Serial.println(who, HEX);
  writeReg(0x6B, 0x80); delay(100);
  writeReg(0x6B, 0x01); delay(50);
  writeReg(0x1A, 0x02);                 // ۱۲ مهر: فیلتر ۹۲Hz (قبلا ۴۱) — برای دیدن ضربه‌ي تیز در نمونه‌برداری ۲۰۰Hz
  writeReg(0x1B, 0x08);                 // ±500 °/s
  writeReg(0x1C, 0x08);                 // ±4g
  if (who != 0x68) writeReg(0x1D, 0x02); // شتاب: فیلتر ۹۹Hz
  return true;
}
bool readImu(float &ax, float &ay, float &az, float &gmag) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14) != 14) return false;
  uint8_t b[14];
  for (int i = 0; i < 14; i++) b[i] = Wire.read();
  auto s16 = [&](int i) { return (int16_t)((b[i] << 8) | b[i + 1]); };
  ax = s16(0) / 8192.0f; ay = s16(2) / 8192.0f; az = s16(4) / 8192.0f;
  float gx = s16(8) / 65.5f, gy = s16(10) / 65.5f, gz = s16(12) / 65.5f;
  gmag = sqrtf(gx * gx + gy * gy + gz * gz);
  return true;
}

float dot3(const float *a, const float *b) { return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; }
void norm3(float *v) {
  float m = sqrtf(dot3(v, v));
  if (m > 1e-6f) { v[0] /= m; v[1] /= m; v[2] /= m; }
}
void cross3(const float *a, const float *b, float *o) {
  o[0] = a[1]*b[2] - a[2]*b[1];
  o[1] = a[2]*b[0] - a[0]*b[2];
  o[2] = a[0]*b[1] - a[1]*b[0];
}
bool readA(float *a, float &gmag) { return readImu(a[0], a[1], a[2], gmag); }

// ---------- صفحه‌های کالیبره (متن انگلیسی ، فونت نمایشگر فارسی ندارد) ----------
// ۱۲ مهر: روی صفحه‌ي گرد گوشه‌ها دیده نمی‌شوند ، پس متن‌ها کوچک‌تر و پایین‌تر و نوار بالاتر
#if DISPLAY_ROUND
const int UI_TS = 2, UI_TY = 46, UI_SS = 1, UI_SY = 76, UI_BY = 192, UI_DY = -18;
#else
const int UI_TS = 3, UI_TY = 22, UI_SS = 2, UI_SY = 60, UI_BY = 250, UI_DY = 0;
#endif
void promptScreen(const char *title, const char *sub, int arrow, uint16_t col) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(col);
  tft.setTextSize(UI_TS);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, UI_TY);
  tft.print(title);
  tft.setTextSize(UI_SS);
  tft.getTextBounds(sub, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, UI_SY);
  tft.print(sub);
  if (arrow == 0) {
    tft.fillRect(40, 128 + UI_DY, 100, 28, col);
    tft.fillTriangle(140, 100 + UI_DY, 140, 184 + UI_DY, 200, 142 + UI_DY, col);
  } else if (arrow == 1) {
#if DISPLAY_ROUND
    tft.fillRect(106, 92, 28, 50, col);
    tft.fillTriangle(82, 142, 158, 142, 120, 180, col);
#else
    tft.fillRect(106, 90, 28, 90, col);
    tft.fillTriangle(78, 180, 162, 180, 120, 236, col);
#endif
  } else if (arrow == -1) {
    tft.fillRect(50, 140 + UI_DY, 140, 6, col);
    tft.fillCircle(120, 143 + UI_DY, 10, col);
  }
  tft.drawRect(30, UI_BY, 180, 14, col);
}
void progressBar(float p, uint16_t col) {
  if (p < 0) p = 0;
  if (p > 1) p = 1;
  int w = (int)(176 * p);
  tft.fillRect(32, UI_BY + 2, w, 10, col);
  tft.fillRect(32 + w, UI_BY + 2, 176 - w, 10, ST77XX_BLACK);
}
void waitStill(float *out, uint16_t col) {
  float acc[3] = {0, 0, 0}; int n = 0;
  while (n < 30) {
    float a[3], g;
    if (readA(a, g)) {
      float m = sqrtf(dot3(a, a));
      if (g < 20 && fabsf(m - 1) < 0.15f) { for (int i = 0; i < 3; i++) acc[i] += a[i]; n++; }
      else { acc[0] = acc[1] = acc[2] = 0; n = 0; }
    }
    progressBar(n / 30.0f, col);
    delay(20);
  }
  for (int i = 0; i < 3; i++) out[i] = acc[i] / n;
}
void waitTilt(const float *g0, float *out, uint16_t col) {
  float acc[3] = {0, 0, 0}; int n = 0;
  while (n < 25) {
    float a[3], g;
    if (readA(a, g)) {
      float d[3] = {a[0]-g0[0], a[1]-g0[1], a[2]-g0[2]};
      if (sqrtf(dot3(d, d)) > 0.35f && g < 40) { for (int i = 0; i < 3; i++) acc[i] += a[i]; n++; }
      else { acc[0] = acc[1] = acc[2] = 0; n = 0; }
    }
    progressBar(n / 25.0f, col);
    delay(20);
  }
  for (int i = 0; i < 3; i++) out[i] = acc[i] / n;
}
void waitLevel(const float *g0) {
  int n = 0;
  while (n < 15) {
    float a[3], g;
    if (readA(a, g)) {
      float d[3] = {a[0]-g0[0], a[1]-g0[1], a[2]-g0[2]};
      n = (sqrtf(dot3(d, d)) < 0.15f) ? n + 1 : 0;
    }
    delay(20);
  }
}
void runCalibration() {
  uint16_t cY = tft.color565(255, 210, 60), cG = tft.color565(80, 230, 120);
  Serial.println("کالیبره: ۱) ربات را صاف و ساکن نگه دار");
  promptScreen("HOLD STILL", "keep it level", -1, cY);
  waitStill(cal.g0, cY);
  Serial.println("کالیبره: ۲) طرف راستِ تصویر را پایین ببر و نگه دار");
  promptScreen("TILT RIGHT", "lower this side", 0, cY);
  float aR[3]; waitTilt(cal.g0, aR, cY);
  promptScreen("OK!", "now back to level", -1, cG);
  waitLevel(cal.g0);
  Serial.println("کالیبره: ۳) طرف پایینِ تصویر را پایین ببر و نگه دار");
  promptScreen("TILT DOWN", "lower this side", 1, cY);
  float aD[3]; waitTilt(cal.g0, aD, cY);

  float gh[3] = {cal.g0[0], cal.g0[1], cal.g0[2]}; norm3(gh);
  float dR[3] = {aR[0]-cal.g0[0], aR[1]-cal.g0[1], aR[2]-cal.g0[2]};
  float k = dot3(dR, gh);
  for (int i = 0; i < 3; i++) cal.ex[i] = dR[i] - k * gh[i];
  norm3(cal.ex);
  cross3(gh, cal.ex, cal.ey); norm3(cal.ey);
  float dD[3] = {aD[0]-cal.g0[0], aD[1]-cal.g0[1], aD[2]-cal.g0[2]};
  if (dot3(cal.ey, dD) < 0) for (int i = 0; i < 3; i++) cal.ey[i] = -cal.ey[i];
  cal.magic = CALIB_MAGIC;

  Preferences pr;
  pr.begin("moochi", false);
  pr.putBytes("calib", &cal, sizeof(cal));
  pr.end();
  calOk = true;
  promptScreen("DONE", "saved", -1, cG);
  progressBar(1, cG);
  Serial.println("کالیبره تمام شد و ذخیره شد ✅");
  waitLevel(cal.g0);
  delay(400);
}
bool loadCalibration() {
  Preferences pr;
  if (!pr.begin("moochi", true)) return false;     // بار اول هنوز چیزی ذخیره نشده
  size_t n = pr.getBytes("calib", &cal, sizeof(cal));
  pr.end();
  return n == sizeof(cal) && cal.magic == CALIB_MAGIC;
}

#include "moochi_motion.h"   // ۱۲ مهر: خواندن ۲۰۰Hz روی هسته‌ي ۰ + یاد دادن حرکت‌ها
#include "moochi_audio.h"    // ۱۲ مهر: صدا از MAX98357 (فقط با نمایشگر گرد روشن می‌شود)

// ============================================
//  کشیدن نواری: برای هر نوار کل چهره کشیده و همان نوار فرستاده می‌شود
// ============================================
struct FrameState { Mood show; float t; int o; float shutter; bool cat; };

// کدام حالت‌ها سبک نئونی تازه را دارند (بقیه هنوز سبک قبلی)
bool neonKindFor(Mood m, NeonKind &k) {
  switch (m) {
    case M_IDLE:     k = NK_NORMAL;   return true;
    case M_HAPPY:    k = NK_HAPPY;    return true;
    case M_SURPRISE: k = NK_SURPRISE; return true;
    case M_SLEEPY:   k = NK_SLEEPY;   return true;
    case M_ASLEEP:   k = NK_ASLEEP;   return true;
    // ۱۲ مهر: واکنش‌های تازه هم نئونی شدند
    case M_DIZZY:    k = NK_DIZZY;    return true;
    case M_HURT:     k = NK_SAD;      return true;
    case M_CRY:      k = NK_CRY;      return true;
    case M_ANGRY:    k = NK_ANGRY;    return true;
    case M_CURIOUS:  k = NK_CURIOUS;  return true;
    // ۱۲ مهر (دوم): بقیه‌ي حالت‌ها
    case M_EXCITED:  k = NK_EXCITED;  return true;
    case M_PET:      k = NK_PET;      return true;
    case M_LOVE:     k = NK_LOVE;     return true;
    case M_THINK:    k = NK_THINK;    return true;
    case M_SHY:      k = NK_SHY;      return true;
    case M_YAWN:     k = NK_YAWN;     return true;
    case M_LISTEN:   k = NK_LISTEN;   return true;
    case M_KISS:     k = NK_KISS;     return true;
    case M_EYEROLL:  k = NK_EYEROLL;  return true;
    case M_HEARTHAND:k = NK_HEARTHAND;return true;
    case M_FEED:     k = NK_FEED;     return true;
    case M_RAINBOW:  k = NK_RAINBOW;  return true;
    default:         return false;
  }
}

int feedFood = -1;              // غذای فعلی (هر بار که «غذا» شروع شود ، بعدی)

NeonParams neonParamsFor(const FrameState &F, NeonKind nk) {
  NeonParams P;
  P.kind = nk;
  P.gx = gazeX * 0.45f;           // نگاه: مجازی ±۲۴ ← واقعی حدود ±۱۱
  P.gy = gazeY * 0.40f;
  P.fx = faceX * 0.667f;          // تکیه‌ي صورت: مجازی ← واقعی
  P.fy = faceY * 0.667f;
  P.open = F.o / 100.0f;          // پلک
  P.t = F.t;
  P.food = feedFood;
  return P;
}

void drawContent(const FrameState &F) {
  NeonKind nk;
  if (!F.cat && neonKindFor(F.show, nk)) {
    NeonParams P = neonParamsFor(F, nk);
    neonDraw(P);
    if (F.show == M_ASLEEP) drawFloaters();    // Zها
    drawShutter(F.shutter);
    return;
  }
  if (F.cat) {
    neonDrawCatalog(CATALOG[catFace], neonParamsFor(F, NK_NORMAL));
  } else {
    switch (F.show) {
      case M_IDLE:     drawIdle(F.o);        break;
      case M_HAPPY:    drawHappy(F.o);       break;
      case M_PET:      drawPet(F.t);         break;
      case M_LOVE:     drawLove(F.t);        break;
      case M_DIZZY:    drawDizzy(F.t);       break;
      case M_HURT:     drawHurt(F.t, F.o);   break;
      case M_SURPRISE: drawSurprise(F.t);    break;
      case M_SLEEPY:   drawSleepy(F.t);      break;
      case M_ASLEEP:   drawAsleep(F.t);      break;
      case M_CURIOUS:  drawCurious(F.t, F.o);break;
      case M_LISTEN:   drawListen(F.t, F.o); break;
      case M_ANGRY:    drawAngry(F.t, F.o);  break;
      case M_KISS:     drawKiss(F.t);        break;
      case M_CRY:      drawCry(F.t);         break;
      case M_EYEROLL:  drawEyeRoll(F.t);     break;
      case M_HEARTHAND:drawHeartHand(F.t);   break;
      case M_FEED:     drawFeed(F.t);        break;
      case M_RAINBOW:  drawRainbow(F.t);     break;
      case M_THINK:    drawThink(F.t, F.o);  break;
      case M_SHY:      drawShy(F.t, F.o);    break;
      case M_EXCITED:  drawExcited(F.t);     break;
      case M_YAWN:     drawYawn(F.t);        break;
    }
  }
  drawShutter(F.shutter);
}

int bandAlloc = 140;            // ظرفیت بافر نوار (ردیف)

// ردیف‌های y0..y1 را در تکه‌هایی به اندازه‌ي بافر بکش و بفرست
void renderRows(const FrameState &F, int y0, int y1) {
  for (int y = y0; y <= y1; y += bandAlloc) {
    int h = y1 - y + 1;
    if (h > bandAlloc) h = bandAlloc;
    gfxSetBandRows(y, h);
    drawContent(F);
    tft.startWrite();
    tft.setAddrWindow(0, bandY0, PHYS_W, bandH);
    tft.writePixels(bandBuf, PHYS_W * bandH, true, true);   // داده از قبل big-endian است (swap16)
    tft.endWrite();
  }
}

// ۱۲ مهر: در حالت‌های نئونی بیرون از صورت همیشه مشکی است ، پس فقط ردیف‌های
// خود صورت (و جای فریم قبلی) فرستاده می‌شود — ارسال SPI تقریبا نصف می‌شود.
bool outsideClean = false;      // آیا بیرون از ناحیه‌ي صورت الان مشکی است؟
int  prevNY0 = 0, prevNY1 = PHYS_H - 1;

void renderFrame(const FrameState &F) {
  NeonKind nk;
  bool neon = !F.cat && neonKindFor(F.show, nk) && F.shutter <= 0;
  if (neon) {
    int a, b;
    neonRowRange(neonParamsFor(F, nk), a, b);
    if (outsideClean) {
      renderRows(F, a < prevNY0 ? a : prevNY0, b > prevNY1 ? b : prevNY1);
    } else {
      renderRows(F, 0, PHYS_H - 1);
      outsideClean = true;
    }
    prevNY0 = a; prevNY1 = b;
  } else {
    renderRows(F, 0, PHYS_H - 1);
    outsideClean = false;
  }
}

// بافر نوار: اول ۲ نوار ۱۴۰ ردیفی (۶۷ KB) ، اگر جا نشد ۴ نوار ۷۰ ردیفی (۳۴ KB)
void allocBands() {
  Serial.printf("بزرگ‌ترین تکه‌ي آزاد حافظه: %u KB\n",
                (unsigned)(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) / 1024));
  const int tryH[2] = {140, 70};
  for (int i = 0; i < 2; i++) {
    bandBuf = (uint16_t *)heap_caps_malloc(PHYS_W * tryH[i] * 2, MALLOC_CAP_8BIT);
    if (bandBuf) {
      bandH = tryH[i];
      bandAlloc = bandH;
      bandN = PHYS_H / bandH;
      Serial.printf("کشیدن نواری: %d نوار %d ردیفی (%u KB)\n", bandN, bandH,
                    (unsigned)(PHYS_W * bandH * 2 / 1024));
      return;
    }
  }
  Serial.println("☠️ حافظه برای بافر نوار نیست!");
  while (true) delay(1000);
}

// ============================================
//  منطق ساده‌ي حالت‌ها (حواس: فقط شتاب‌سنج)
// ============================================
float tiltX = 0, tiltY = 0;       // -1..1 (نرم‌شده)
bool  tilted = false;
uint32_t nextLivelyAt = 0, lastZ = 0, lastSurprise = 0;

uint32_t shakeTimes[3] = {0, 0, 0};   // سه تکان آخر ← عصبانی
int      shakeIdx = 0;
uint32_t upsideSince = 0;             // از کی وارونه است
uint32_t tiltSince = 0;               // از کی کج و ساکن نگه داشته شده
uint32_t rockSince = 0;               // از کی آرام تاب می‌خورد
uint32_t lastStrongEv = 0;            // آخرین رویداد غیر «آرام»
uint32_t lastShakeEv = 0;             // آخرین رویداد تکان (برای یکی حساب کردن تکان طولانی)

void reactTo(int c, uint32_t now) {
  bool sleeping = (mood == M_ASLEEP || mood == M_SLEEPY);
  if (c != MC_GENTLE) lastStrongEv = now;
  if (sleeping) {
    if (c != MC_GENTLE) {                 // جابه‌جا کردن آرام (یا تاب) بیدارش نمی‌کند
      Serial.println("بیدار شد!");
      setMood(M_SURPRISE, T_SURPRISE);
      lastSurprise = now;
    }
    return;
  }
  if (mood == M_CRY && upsideSince) return;   // وارونه است و گریه می‌کند — اول برش گردان
  switch (c) {
    case MC_SHAKE: {
      // ۱۲ مهر: تکانِ بیشتر از ۲٫۵ ثانیه دو رویداد پشت سر هم می‌شد و دو تکان حساب می‌شد
      // (در لاگ: یک تکان طولانی + یکی دیگر = «سه بار» ← عصبانی). حالا رویداد تکانی که
      // کمتر از ۳ ثانیه بعد از تکان قبلی بیاید ادامه‌ي همان است.
      bool newShake = !lastShakeEv || now - lastShakeEv > 3000;
      lastShakeEv = now;
      if (!newShake) break;
      shakeTimes[shakeIdx] = now; shakeIdx = (shakeIdx + 1) % 3;
      bool three = true;
      for (int i = 0; i < 3; i++) if (!shakeTimes[i] || now - shakeTimes[i] > 20000) three = false;
      if (three) {
        Serial.println("عصبانی شدم! (سه بار تکان)");
        setMood(M_ANGRY, T_ANGRY);
        shakeTimes[0] = shakeTimes[1] = shakeTimes[2] = 0;
      } else if (mood != M_DIZZY && mood != M_ANGRY) {
        Serial.println("گیج شدم!");
        setMood(M_DIZZY, T_DIZZY);
      }
      break;
    }
    case MC_KNOCK:
      if (mood != M_HURT && mood != M_ANGRY) {
        Serial.println("آخ! (ضربه)");
        setMood(M_HURT, T_HURT);
      }
      break;
    case MC_MOVE:
      if (mood != M_DIZZY && mood != M_HURT && mood != M_ANGRY && mood != M_SURPRISE
          && now - lastSurprise > 2500) {
        Serial.println("جا خوردم!");
        setMood(M_SURPRISE, T_SURPRISE);
        lastSurprise = now;
      }
      break;
    default: break;                       // آرام: واکنشی ندارد
  }
}

// دست جلوی فاصله‌سنج:
//   نزدیک شدن سریع (از دور به زیر ۱۵ سانت در کمتر از نیم ثانیه)  → جا می‌خورد
//   دست ۳ تا ۱۵ سانتی نگه داشته شود (۱ ثانیه)                   → «نوازش»: لوس می‌شود
//   نوازش پیوسته‌ي ۶ ثانیه                                        → عاشق
//   در خواب ، دست نزدیک (زیر ۲۰ سانت)                           → آرام بیدار می‌شود (خمیازه)
uint32_t nearSince = 0, farAt = 0, petSince = 0;
void senseDistance(const ImuSnap &S, uint32_t now) {
  if (now - S.distAt > 400) return;                     // اندازه‌ي تازه نیست
  uint16_t d = S.distMm;
  bool far = (d == 0 || d > 350);
  bool near = (d > 0 && d < 150);
  if (far) { farAt = now; nearSince = 0; petSince = 0; return; }
  if (near) lastTouchOrMove = now;
  if (mood == M_ASLEEP || mood == M_SLEEPY) {
    if (d < 200) { Serial.printf("[فاصله] %u mm — بیدار شد\n", d); setMood(M_YAWN, T_YAWN); }
    return;
  }
  if (near && now - farAt < 500 && mood != M_SURPRISE && now - lastSurprise > 2500) {
    Serial.printf("[فاصله] %u mm — یک‌دفعه نزدیک شد!\n", d);
    setMood(M_SURPRISE, T_SURPRISE); lastSurprise = now; farAt = 0;
    return;
  }
  if (d > 30 && d < 150) {
    if (!nearSince) nearSince = now;
    if (now - nearSince > 1000) {
      if (!petSince) petSince = now;
      if (now - petSince > T_PET_TO_LOVE && mood != M_LOVE) {
        Serial.println("[فاصله] نوازش طولانی — عاشق شد");
        setMood(M_LOVE, T_LOVE);
      } else if (mood != M_PET && mood != M_LOVE && mood != M_DIZZY && mood != M_ANGRY && mood != M_HURT) {
        Serial.printf("[فاصله] %u mm — نوازش\n", d);
        setMood(M_PET, T_PET_LINGER);
      } else if (mood == M_PET) {
        moodUntil = now + T_PET_LINGER;                 // تا وقتی دست هست ، لوس بماند
      }
    }
  } else { nearSince = 0; petSince = 0; }
}

void senseImu() {
  if (!imuOk) return;
  ImuSnap S; getImuSnap(S);
  uint32_t now = millis();
  const float *a = S.a;

  if (calOk) {
    float d[3] = {a[0] - restNow[0], a[1] - restNow[1], a[2] - restNow[2]};
    float rx = constrain(dot3(d, cal.ex) * 1.8f, -1.0f, 1.0f);
    float ry = constrain(dot3(d, cal.ey) * 1.8f, -1.0f, 1.0f);
    tiltX += (rx - tiltX) * 0.3f;
    tiltY += (ry - tiltY) * 0.3f;
    float tm = fmaxf(fabsf(tiltX), fabsf(tiltY));
    if (!tilted && tm > 0.18f) tilted = true;
    else if (tilted && tm < 0.10f) tilted = false;
  }
  bool movingNow = now - S.lastActive < 300;
  if (movingNow) lastTouchOrMove = now;

  // ---------- رویدادهای حرکت ----------
  MotionEvent e;
  while (xQueueReceive(motionQ, &e, 0) == pdTRUE) {
    float dist[MC_N];
    int c = motionClassify(e, dist);
    printEvent(MC_FA[c], e, dist);
    reactTo(c, now);
  }

  // ---------- وارونه: اول جا می‌خورد ، اگر وارونه بماند گریه می‌کند ----------
  float an[3] = {a[0], a[1], a[2]}, rn[3] = {restNow[0], restNow[1], restNow[2]};
  norm3(an); norm3(rn);
  bool upside = dot3(an, rn) < -0.5f;
  if (upside) {
    if (!upsideSince) upsideSince = now;
    uint32_t held = now - upsideSince;
    if (held > 600 && held < 1600 && mood != M_SURPRISE && mood != M_CRY) {
      Serial.println("وای ، وارونه‌ام!"); setMood(M_SURPRISE, 1400); lastSurprise = now;
    } else if (held >= 1600 && mood != M_CRY) {
      Serial.println("گریه — برم گردون!"); setMood(M_CRY, 0);
    }
  } else {
    if (upsideSince && mood == M_CRY && moodUntil == 0) setMood(M_CRY, 1500);
    upsideSince = 0;
  }

  // ---------- فاصله‌سنج (۱۲ مهر) ----------
  if (tofOk) senseDistance(S, now);

  // ---------- کج و ساکن نگه داشتن: کنجکاو ----------
  // ۱۲ مهر: قبلا وسط جابه‌جا کردن هم فعال می‌شد. حالا باید کج باشد و ۲٫۵ ثانیه
  // هیچ رویداد حرکتی نباشد (یعنی واقعا کج «نگه داشته» شده).
  float tm = fmaxf(fabsf(tiltX), fabsf(tiltY));
  if (tm > 0.45f && !upside && !movingNow) { if (!tiltSince) tiltSince = now; }
  else tiltSince = 0;
  if (tiltSince && now - tiltSince > 2500 && mood == M_IDLE) {
    Serial.println("کنجکاو شدم (کج نگه داشتی)");
    setMood(M_CURIOUS, T_CURIOUS);
    tiltSince = 0;
  }

  // ---------- تاب دادن آرام (لالایی): خواب‌آلود ----------
  // باید رفت‌وبرگشت باشد (برگشت جهت چرخش) ، در بازه‌ي یادگرفته ، و ۴ ثانیه هیچ رویداد قوی نباشد
  bool rocking = S.rockE > mm.rockLo && S.rockE < mm.rockHi && S.rockRevs >= mm.rockRev
                 && !upside && now - lastStrongEv > 4000;
  if (rocking) { if (!rockSince) rockSince = now; }
  else rockSince = 0;
  if (rockSince && now - rockSince > 4000 && mood == M_IDLE) {
    Serial.println("لالایی... خوابم میاد");
    setMood(M_SLEEPY, 0);
    rockSince = 0;
  }
}

void idleBehavior(uint32_t now) {
  struct Opt { int w; int kind; };     // kind: شماره‌ي Mood ، یا −۱ = چهره‌ي کاتالوگ
  static const Opt OPTS[] = {
    {24, M_HAPPY}, {12, M_THINK}, {10, M_CURIOUS}, {10, M_LISTEN}, {18, -1},
    { 8, M_FEED}, { 6, M_YAWN}, { 3, M_EYEROLL}, { 3, M_RAINBOW}, { 3, M_SHY}, {3, M_EXCITED},
  };
  int total = 0;
  for (auto &o : OPTS) total += o.w;
  int r = random(total);
  int k = M_HAPPY;
  for (auto &o : OPTS) { if (r < o.w) { k = o.kind; break; } r -= o.w; }
  // بعد از مدتی بی‌کاری ، خمیازه بیشتر
  if (now - lastTouchOrMove > 40000 && random(3) == 0) k = M_YAWN;
  Serial.print("[خودش] ");
  switch (k) {
    case -1:        SHOW_SET(SET_HAPPY, 3200); break;
    case M_LISTEN:  setMood(M_LISTEN, 8000); break;
    case M_HAPPY:   setMood(M_HAPPY, 2500); break;
    case M_THINK:   setMood(M_THINK, T_THINK); break;
    case M_CURIOUS: setMood(M_CURIOUS, T_CURIOUS); break;
    case M_FEED:    setMood(M_FEED, T_FEED); break;
    case M_YAWN:    setMood(M_YAWN, T_YAWN); break;
    case M_EYEROLL: setMood(M_EYEROLL, T_EYEROLL); break;
    case M_RAINBOW: setMood(M_RAINBOW, T_RAINBOW); break;
    case M_SHY:     setMood(M_SHY, T_SHY); break;
    default:        setMood(M_EXCITED, T_EXCITED); break;
  }
}

// ---------- صدای هر حالت (وقتی حالت تازه شروع می‌شود) ----------
int voiceForMood(Mood m) {
  switch (m) {
    case M_HAPPY: return V_HAPPY;      case M_PET: return V_GIGGLE;     case M_LOVE: return V_LOVE;
    case M_DIZZY: return V_DIZZY;      case M_HURT: return V_HURT;      case M_SURPRISE: return V_SURPRISE;
    case M_SLEEPY: return V_YAWN;      case M_ASLEEP: return V_SLEEP;   case M_CURIOUS: return V_CURIOUS;
    case M_THINK: return V_QUESTION;   case M_SHY: return V_GIGGLE;     case M_EXCITED: return V_EXCITED;
    case M_YAWN: return V_YAWN;        case M_LISTEN: return V_LISTEN;  case M_ANGRY: return V_ANGRY;
    case M_KISS: return V_LOVE;        case M_CRY: return V_CRY;        case M_EYEROLL: return V_GRUMPY;
    case M_HEARTHAND: return V_LOVE;   case M_FEED: return V_EAT;       case M_RAINBOW: return V_HAPPY;
    default: return V_NONE;
  }
}

void moodStepSimple() {
  uint32_t now = millis();
  if (moodUntil && now >= moodUntil) setMood(M_IDLE, 0);

  // بی‌حرکتی ← خواب‌آلود ← خواب
  if (mood == M_IDLE && catFace < 0 && now - lastTouchOrMove > T_SLEEPY_AFTER) {
    Serial.println("خوابم میاد...");
    setMood(M_SLEEPY, 0);
  }
  if (mood == M_SLEEPY && now - moodStart > T_SLEEP_AFTER) setMood(M_ASLEEP, 0);
  if (mood == M_ASLEEP && now - lastZ > 1600) { lastZ = now; spawnFloater(CX + 70, CY - 60, 1); }

  // ---------- زنده بودن (۱۲ مهر): وقتی بی‌کار است ، خودش کاری می‌کند ----------
  // هر ۱۸ تا ۳۵ ثانیه یکی از این‌ها ، با شانس‌های متفاوت. کاتالوگ حالا نئونی است ،
  // پس چهره‌های «دل‌بسته»ی کاتالوگ هم در قرعه‌اند.
  if (mood == M_IDLE && catFace < 0 && now > nextLivelyAt) {
    idleBehavior(now);
    nextLivelyAt = now + random(18000, 35000);
  }
}

// ---------- رژه ----------
bool parade = false, paradeCat = true;
int paradeIdx = 0;
uint32_t paradeAt = 0;
const uint32_t PARADE_MS = 2600;

void paradeShow(int i) {
  int n = paradeCat ? CATALOG_N : 22;
  paradeIdx = (i % n + n) % n;
  paradeAt = millis();
  if (paradeCat) showFace(paradeIdx, 0);
  else { setMood((Mood)paradeIdx, 0); Serial.printf("[حالت %d/22] %s\n", paradeIdx + 1, moodName((Mood)paradeIdx)); }
}

void serialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'p' || c == 'P') {
      parade = !parade;
      Serial.println(parade ? "[رژه] روشن — n بعدی · b قبلی · c نوع · p خاموش" : "[رژه] خاموش");
      if (parade) paradeShow(0); else { hideFace(); setMood(M_IDLE, 0); }
    }
    if (c == 'c' || c == 'C') {
      paradeCat = !paradeCat;
      Serial.println(paradeCat ? "[رژه] چهره‌های کاتالوگ (۷۴)" : "[رژه] حالت‌های رفتاری (۲۲)");
      if (parade) paradeShow(0);
    }
    if (parade && (c == 'n' || c == 'N')) paradeShow(paradeIdx + 1);
    if (parade && (c == 'b' || c == 'B')) paradeShow(paradeIdx - 1);
    // ۱۲ مهر: پریدن مستقیم — عدد بزن و Enter (در رژه‌ي روشن ، همان شماره‌ای که چاپ می‌شود)
    static int numBuf = 0; static bool numHas = false;
    if (c >= '0' && c <= '9') { numBuf = numBuf * 10 + (c - '0'); numHas = true; if (numBuf > 999) numBuf = 0; }
    if ((c == '\r' || c == '\n') && numHas) {
      if (!parade) { parade = true; Serial.println("[رژه] روشن"); }
      paradeShow(numBuf - 1);
      numBuf = 0; numHas = false;
    }
#if DISPLAY_ROUND
    if (c == 'o' || c == 'O') {                   // چرخاندن تصویر ۹۰ درجه (ذخیره می‌شود)
      SCREEN_ROTATION = (SCREEN_ROTATION + 1) & 3;
      tft.setRotation(SCREEN_ROTATION);
      Preferences pr; pr.begin("moochi", false); pr.putUChar("rot", SCREEN_ROTATION); pr.end();
      outsideClean = false;
      Serial.printf("[صفحه] چرخش %d (ذخیره شد)\n", SCREEN_ROTATION);
    }
#endif
    if (c == 's' || c == 'S') {                   // آزمایش صدا: آوای بعدی
      static int vi = -1; vi = (vi + 1) % VOICE_N;
      voiceSay(vi);
      Serial.printf("[صدا] %d/%d %s%s\n", vi + 1, VOICE_N, VOICE[vi].name, audioOk ? "" : "  (⚠️ صدا روشن نیست)");
    }
    if (c == 'v' || c == 'V') {                   // بلندی: ۰ بی‌صدا … ۳ بلند
      volIdx = (volIdx + 1) % 4;
      Serial.printf("[صدا] بلندی %d از ۳\n", (int)volIdx);
      voiceSay(V_CHIRP);
    }
    if (c == 'd' || c == 'D') {                   // فاصله‌سنج: عدد فعلی
      ImuSnap S; getImuSnap(S);
      if (!tofOk) Serial.println("[فاصله] سنسور پیدا نشده");
      else Serial.printf("[فاصله] %u mm (۰ = چیزی جلویش نیست)\n", S.distMm);
    }
    // میان‌بُر حالت‌ها (۸ ثانیه)
    if (c == 'f' || c == 'F') { parade = false; hideFace(); setMood(M_FEED, 3000);  Serial.println("[آزمایش] غذا"); }
    if (c == 'l' || c == 'L') { parade = false; hideFace(); setMood(M_LISTEN, 8000); Serial.println("[آزمایش] گوش دادن به موسیقی"); }
    if (c == 'r' || c == 'R') {                   // آزمایش: رنگین‌کمان ۶ ثانیه
      parade = false; hideFace();
      setMood(M_RAINBOW, 6000);
      Serial.println("[آزمایش] رنگین‌کمان ۶ ثانیه");
    }
    if ((c == 't' || c == 'T') && imuOk) {        // ۱۲ مهر: یاد دادن دوباره‌ي حرکت‌ها
      parade = false; hideFace();
      runMotionTeach();
      setMood(M_IDLE, 0); outsideClean = false; lastStrongEv = millis();
    }
    if ((c == 'k' || c == 'K') && imuOk) {        // کالیبره‌ي دوباره‌ي جهت (کج شدن)
      parade = false; hideFace();
      imuHold(true); runCalibration(); imuHold(false);
      drainMotion();
      setMood(M_IDLE, 0); outsideClean = false;
    }
  }
}

// ============================================
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  موچی - test 08 : چهره‌ي کامل");
  Serial.println("==============================");

  gfxInitTables();
  allocBands();
#if DISPLAY_ROUND
  SPI.begin(PIN_SCK, -1, PIN_MOSI, -1);
  tft.begin(40000000);                      // ۴۰MHz ؛ بدون RST ، کتابخانه ریست نرم‌افزاری می‌فرستد
  { Preferences pr; if (pr.begin("moochi", true)) { SCREEN_ROTATION = pr.getUChar("rot", 0) & 3; pr.end(); } }
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(ST77XX_BLACK);
  Serial.println("نمایشگر: گرد GC9A01 ۲۴۰×۲۴۰");
#elif ST7789_CS15
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
  tft.init(240, 280);
  tft.setSPISpeed(40000000);
  tft.setRotation(SCREEN_ROTATION);
  Serial.println("نمایشگر: ST7789 ۲۴۰×۲۸۰ (CS روی G15)");
#elif ST7789_NO_CS
  SPI.begin(PIN_SCK, -1, PIN_MOSI, -1);
  // ⚠️ بدون CS ، ماژول‌های ST7789 معمولا با SPI mode 3 کار می‌کنند (گزارش‌های رایج ، روی برد ما تست نشده).
  //    اگر صفحه سیاه ماند ، خط بعد را به SPI_MODE0 عوض کن.
  tft.init(240, 280, SPI_MODE3);
  tft.setSPISpeed(40000000);
  tft.setRotation(SCREEN_ROTATION);
  Serial.println("نمایشگر: ST7789 ۲۴۰×۲۸۰ (بدون CS/RES ، SPI mode 3)");
#else
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
  tft.init(240, 280);
  tft.setSPISpeed(40000000);
  tft.setRotation(SCREEN_ROTATION);
  Serial.println("نمایشگر: ST7789 ۲۴۰×۲۸۰");
#endif

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  imuOk = setupImu();
  if (!imuOk) Serial.println("⚠️ شتاب‌سنج جواب نداد — چهره بدون حس حرکت کار می‌کند.");

  playBootAnimation(2000);

  if (imuOk) {
    calOk = !FORCE_CALIBRATION && loadCalibration();
    if (calOk) Serial.println("کالیبره‌ي ذخیره‌شده خوانده شد.");
    else runCalibration();
    float acc[3] = {0, 0, 0}; int n = 0;
    for (int i = 0; i < 25; i++) {
      float a[3], g;
      if (readA(a, g) && g < 20) { for (int j = 0; j < 3; j++) acc[j] += a[j]; n++; }
      delay(15);
    }
    if (n >= 20) { for (int j = 0; j < 3; j++) restNow[j] = acc[j] / n; }
    else         { for (int j = 0; j < 3; j++) restNow[j] = cal.g0[j]; }
    tofBegin();                    // فاصله‌سنج (اگر وصل باشد) — قبل از شروع کار هسته‌ي ۰
    motionBegin();                 // از این‌جا به بعد شتاب‌سنج فقط در کارِ هسته‌ي ۰ خوانده می‌شود
    if (!mm.trained) runMotionTeach();
  }

  randomSeed(esp_random());
#if AUDIO_ENABLED
  audioBegin();                  // صدا (MAX98357): BCLK G27 ، LRC G25 ، DIN G23
  voiceSay(V_BOOT);
#endif
  blinkAt = millis() + 1500;
  lastTouchOrMove = millis();
  nextLivelyAt = millis() + 20000;
  setMood(M_IDLE, 0);
  Serial.println("موچی بیدار شد. فرمان‌ها: p رژه · c نوع رژه · n بعدی · b قبلی · عدد+Enter پرش · f غذا · l موسیقی · r رنگین‌کمان · s صدا · v بلندی · d فاصله · o چرخش صفحه · t یاد دادن حرکت‌ها · k کالیبره‌ي جهت");
}

void loop() {
  uint32_t t0 = millis();
  serialCommands();

  if (parade) {
    if (millis() - paradeAt > PARADE_MS) paradeShow(paradeIdx + 1);
  } else {
    senseImu();
    moodStepSimple();
  }

  // هر بار که حالت «غذا» شروع شود ، غذای بعدی (۶ نوع ، به نوبت)
  // صدای حالت تازه (هر شروع تازه‌ي حالت ، نه هر فریم)
  static uint32_t lastVoiceStart = 0;
  if (moodStart != lastVoiceStart) {
    lastVoiceStart = moodStart;
    if (catFace >= 0) voiceSay(V_CHIRP);
    else { int v = voiceForMood(mood); if (v != V_NONE) voiceSay(v); }
  }

  // ۱۲ مهر: قبلا فقط «تغییر» به حالت غذا حساب می‌شد ، پس f پشت سر هم همیشه همان سیب بود.
  // حالا هر «شروع تازه»ی غذا (moodStart عوض شود) غذای بعدی است.
  static uint32_t lastFeedStart = 0;
  if (mood == M_FEED && moodStart != lastFeedStart) {
    lastFeedStart = moodStart;
    feedFood = (feedFood + 1) % 6;
    Serial.printf("[غذا] %s\n", FOOD_FA[feedFood]);
  }

  bowPhase += 0.10f;
  if (bowPhase > 62.8f) bowPhase -= 62.8f;

  // نگاه: اگر کج شده ، به همان طرف ؛ وگرنه نگاه آزاد
  if (mood == M_IDLE || mood == M_HAPPY || mood == M_CURIOUS) {
    if (tilted) gazeTowards(tiltX * 24, tiltY * 16);
    else        gazeIdle();
  } else if (mood != M_PET) gazeTowards(0, 0);
  // نرم کردن نگاه و صورت در کد اصلی «هر فریم» یک گام است و برای حدود ۳۵ فریم
  // در ثانیه تنظیم شده بود. این‌جا فریم‌ها کندترند ، پس به اندازه‌ي زمانِ گذشته
  // چند گام برمی‌داریم تا سرعت حرکت چشم همان بماند.
  static uint32_t lastStepAt = 0;
  uint32_t nowMs = millis();
  int nSteps = lastStepAt ? (int)((nowMs - lastStepAt + 14) / 28) : 1;
  if (nSteps < 1) nSteps = 1;
  if (nSteps > 5) nSteps = 5;
  lastStepAt = nowMs;

  // صورت هم کمی به طرف کج شدن می‌رود (تکیه دادن)
  faceTX = tilted ? tiltX * 10 : 0;
  faceTY = tilted ? tiltY * 8 : 0;
  for (int i = 0; i < nSteps; i++) { gazeStep(); faceStep(); }

  setBrowGaze((int)gazeX, (int)gazeY);

  // ---------- گذر نرم (مثل اصل) ----------
  FrameState F;
  F.t = moodT();
  F.o = eyeOpen();
  F.show = mood;
  F.shutter = 0;
  uint32_t td = millis() - transStart;
  if (td < TRANS_MS) {
    float h = (float)td / TRANS_MS;
    if (h < 0.5f) { F.show = prevMood; F.shutter = h * 2.0f; F.t = (millis() - prevMoodStart) / 1000.0f; }
    else          { F.shutter = (1.0f - h) * 2.0f; }
  }
  F.cat = false;
  if (catFace >= 0) {
    if (catUntil && millis() >= catUntil) hideFace();
    else F.cat = true;
  }

  renderFrame(F);

  static uint32_t lastReport = 0;
  if (millis() - lastReport > 4000) {
    lastReport = millis();
    Serial.print("فریم: ");
    Serial.print(millis() - t0);
    Serial.print(" ms   حالت: ");
    Serial.print(F.cat ? CATALOG[catFace].name : moodName(mood));
    Serial.printf("   حافظه‌ي آزاد (بزرگ‌ترین تکه): %u KB\n",
                  (unsigned)(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) / 1024));
  }
}
