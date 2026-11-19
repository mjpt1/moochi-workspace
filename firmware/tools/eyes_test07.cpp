#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>     // حافظه‌ي ماندگار برای ذخیره‌ي کالیبره
#include "boot_gif.h"     // گیف شروع (mahsa_world.gif) — با tools/gif_to_anim.py ساخته شد

// ============================================
//  موچی — test 07 : انیمیشن شروع + چشم‌هایی که با شتاب‌سنج حرکت می‌کنند
// ============================================
// تست‌های قبلی در firmware\tools\ نگه داشته شده‌اند.
//
// بخش ۱ — انیمیشن شروع (مثل «لوگوی روشن شدن»):
//   ۱۱ مهر: گیف رنگی خود مهسن (photos/mahsa_world.gif ، ۸۰ فریم) ←
//   با tools/gif_to_anim.py به src/boot_gif.h تبدیل شد (پالت ۲۵۶ رنگ +
//   فریم‌های تفاضلی + RLE ، حدود ۱٫۷ مگ). به همین خاطر partition = huge_app.
//   انیمیشن گربه‌ي قبلی در tools/anim_frames_cat.h مانده.
//   هر فریم ردیف به ردیف باز می‌شود و مستقیم به نمایشگر می‌رود ،
//   پس بافر کامل صفحه لازم نیست (حافظه‌ي ESP32 ما کم است).
//
// بخش ۲ — چشم‌ها:
//   دو چشم گرد‌گوشه. مردمک‌ها با کج شدن ربات به سمت پایین «می‌غلتند».
//   پلک زدن تصادفی ، تکان شدید → گیجی (مردمک‌ها می‌چرخند) ،
//   حرکت ناگهانی → تعجب (چشم‌ها بزرگ می‌شوند).
//   هر فریم فقط ناحیه‌ي چشم‌ها ردیف به ردیف دوباره کشیده می‌شود → بدون پرپر زدن.
//
// سیم‌کشی (همان قبلی):
//   نمایشگر ۱٫۶۹: SCL→G14  SDA→G13  RES→G25  DC→G26  CS→G27  VCC,BLK→ریل+  GND→ریل−
//   شتاب‌سنج:      SDA→G33  SCL→G32  VCC→ریل+  GND→ریل−
// ============================================

// ---------- نمایشگر ----------
const int PIN_SCK = 14, PIN_MOSI = 13, PIN_RST = 25, PIN_DC = 26, PIN_CS = 27;
const uint8_t SCREEN_ROTATION = 2;   // ۱۱ مهر: با 0 سر و ته بود
Adafruit_ST7789 tft(PIN_CS, PIN_DC, PIN_RST);

// ---------- شتاب‌سنج (MPU6500 ، همان test 03) ----------
const int PIN_SDA = 33, PIN_SCL = 32;
const uint8_t MPU_ADDR = 0x68;
bool imuOk = false;

// ---------- کالیبره‌ي جهت (۱۱ مهر) ----------
// شتاب‌سنج روی بردبورد چرخیده و وارونه نصب است ، پس محورهایش با صفحه
// یکی نیست (مهسن دید چپ/راست ، بالا/پایین نشان می‌داد). به جای حدس زدن
// علامت‌ها ، یک بار از خود کاربر می‌پرسیم: «طرف راست را پایین ببر» و
// «طرف پایین را پایین ببر» — و جهت واقعی را اندازه می‌گیریم.
// نتیجه در حافظه‌ي ماندگار (NVS) ذخیره می‌شود و دفعه‌های بعد تکرار نمی‌شود.
// اگر شتاب‌سنج را جابه‌جا کردی: این را یک بار true کن ، آپلود ، بعد دوباره false.
const bool FORCE_CALIBRATION = false;

struct Calib { uint32_t magic; float g0[3], ex[3], ey[3]; };
const uint32_t CALIB_MAGIC = 0x4D4F4F31;   // "MOO1"
Calib cal;
bool calOk = false;
float restNow[3] = {0, 0, 1};               // حالت صاف همین روشن شدن

// ---------- رنگ‌ها ----------
uint16_t C_BG, C_EYE, C_PUPIL, C_SHINE, C_ANIM;

// ============================================
//  بخش ۱: انیمیشن شروع
// ============================================
static uint16_t lineBuf[240];

// گیف شروع: هر فریم = یک مستطیل (فریم اول کل صفحه ، بقیه فقط جای تغییرکرده).
// داده‌ي داخل مستطیل به صورت جفت‌های (تعداد ، شماره‌ي رنگ در پالت) است.
// ردیف به ردیف باز می‌شود و مستقیم به نمایشگر می‌رود — بافر کامل لازم نیست.
uint32_t drawGifFrame(int k) {
  uint32_t off = BOOT_GIF_IDX[k][0];
  int x = BOOT_GIF_IDX[k][1], y = BOOT_GIF_IDX[k][2];
  int w = BOOT_GIF_IDX[k][3], h = BOOT_GIF_IDX[k][4];
  uint32_t dur = BOOT_GIF_IDX[k][5];
  const uint8_t *p = BOOT_GIF_DATA + off;

  tft.startWrite();
  tft.setAddrWindow(x, y, w, h);
  uint32_t total = (uint32_t)w * h, done = 0;
  int col = 0;
  while (done < total) {
    uint8_t n = pgm_read_byte(p++);
    uint16_t c = BOOT_GIF_PAL[pgm_read_byte(p++)];
    while (n-- && done < total) {
      lineBuf[col++] = c;
      done++;
      if (col == w) { tft.writePixels(lineBuf, w); col = 0; }
    }
  }
  tft.endWrite();
  return dur;
}

void playBootAnimation(uint32_t holdLastMs) {
  uint32_t slowest = 0;
  for (int k = 0; k < BOOT_GIF_FRAMES; k++) {
    uint32_t t0 = millis();
    uint32_t dur = drawGifFrame(k);
    uint32_t dt = millis() - t0;
    if (dt > slowest) slowest = dt;
    if (dt < dur) delay(dur - dt);
  }
  Serial.print("گیف شروع پخش شد. کندترین فریم: ");
  Serial.print(slowest);
  Serial.println(" میلی‌ثانیه (هدف: ۴۰)");
  delay(holdLastMs);   // فریم آخر (اسم) کمی بماند
}

// ============================================
//  شتاب‌سنج
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
  writeReg(0x6B, 0x80); delay(100);     // ریست
  writeReg(0x6B, 0x01); delay(50);      // بیدار
  writeReg(0x1A, 0x03);                 // فیلتر ژیرو
  writeReg(0x1B, 0x08);                 // ±500 °/s
  writeReg(0x1C, 0x08);                 // ±4g
  if (who != 0x68) writeReg(0x1D, 0x03);
  return true;
}

// شتاب به g و چرخش به °/s
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

// ============================================
//  کالیبره‌ي جهت شتاب‌سنج
// ============================================
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

// صفحه‌ي راهنما: arrow = -1 بدون فلش ، 0 = راست ، 1 = پایین
// (فونت نمایشگر فارسی ندارد ، پس متن انگلیسی کوتاه + فلش بزرگ)
void promptScreen(const char *title, const char *sub, int arrow, uint16_t col) {
  tft.fillScreen(C_BG);
  tft.setTextColor(col);
  tft.setTextSize(3);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, 22);
  tft.print(title);
  tft.setTextSize(2);
  tft.getTextBounds(sub, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, 60);
  tft.print(sub);
  if (arrow == 0) {             // فلش به راست
    tft.fillRect(40, 128, 100, 28, col);
    tft.fillTriangle(140, 100, 140, 184, 200, 142, col);
  } else if (arrow == 1) {      // فلش به پایین
    tft.fillRect(106, 90, 28, 90, col);
    tft.fillTriangle(78, 180, 162, 180, 120, 236, col);
  } else {                      // خط تراز
    tft.fillRect(50, 140, 140, 6, col);
    tft.fillCircle(120, 143, 10, col);
  }
  tft.drawRect(30, 250, 180, 14, col);
}

void progressBar(float p, uint16_t col) {
  if (p < 0) p = 0;
  if (p > 1) p = 1;
  int w = (int)(176 * p);
  tft.fillRect(32, 252, w, 10, col);
  tft.fillRect(32 + w, 252, 176 - w, 10, C_BG);
}

// صبر تا ربات ساکن و صاف بماند ؛ میانگین جاذبه را برمی‌گرداند
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

// صبر تا ربات به اندازه‌ي کافی کج شود و همان‌جا نگه داشته شود
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

// صبر تا دوباره صاف شود
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

  // محور «راست» = جهتِ تغییر ، عمود بر جاذبه
  float gh[3] = {cal.g0[0], cal.g0[1], cal.g0[2]}; norm3(gh);
  float dR[3] = {aR[0]-cal.g0[0], aR[1]-cal.g0[1], aR[2]-cal.g0[2]};
  float k = dot3(dR, gh);
  for (int i = 0; i < 3; i++) cal.ex[i] = dR[i] - k * gh[i];
  norm3(cal.ex);
  // محور «پایین» = عمود بر هر دو ؛ علامتش از حرکت دوم
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
  pr.begin("moochi", true);
  size_t n = pr.getBytes("calib", &cal, sizeof(cal));
  pr.end();
  return n == sizeof(cal) && cal.magic == CALIB_MAGIC;
}

// ============================================
//  بخش ۲: چشم‌ها
// ============================================
// ناحیه‌ای که هر فریم دوباره کشیده می‌شود (مختصات صفحه‌ي ۲۴۰×۲۸۰)
const int RX0 = 14, RY0 = 70, RW = 212, RH = 140;
const int EYE_CY = 140;
const int EYE_CX[2] = {72, 168};
const float EYE_HW = 38, EYE_HH = 48, EYE_R = 22;   // نصف عرض ، نصف ارتفاع ، شعاع گوشه
const float PUP_RX = 16, PUP_RY = 19;

// وضعیت
float gazeX = 0, gazeY = 0;          // -1..1 (نرم‌شده)
float openness = 1;                  // 1 باز ، 0 بسته
float scaleK = 1;                    // اندازه‌ي چشم (تعجب)
uint32_t nextBlink = 0, blinkStart = 0;
bool blinking = false;
uint32_t dizzyUntil = 0, surpriseUntil = 0;
float dizzyAng = 0;
float lastAmag = 1;

inline bool inRoundRect(float dx, float dy, float hw, float hh, float r) {
  dx = fabsf(dx); dy = fabsf(dy);
  if (dx > hw || dy > hh) return false;
  if (r > hw) r = hw;
  if (r > hh) r = hh;
  float qx = dx - (hw - r), qy = dy - (hh - r);
  if (qx <= 0 || qy <= 0) return true;
  return qx * qx + qy * qy <= r * r;
}

void drawEyes() {
  float hw = EYE_HW * scaleK;
  float hh = EYE_HH * scaleK * openness;
  if (hh < 3) hh = 3;
  float r = EYE_R * scaleK;

  // مردمک‌ها
  float px[2], py[2];
  for (int e = 0; e < 2; e++) {
    float gx = gazeX, gy = gazeY;
    if (millis() < dizzyUntil) {              // گیجی: می‌چرخند ، هر چشم یک طرف
      float a = dizzyAng + (e ? 3.1416f : 0);
      gx = 0.6f * cosf(a); gy = 0.6f * sinf(a);
    }
    px[e] = EYE_CX[e] + gx * (hw - PUP_RX - 4);
    py[e] = EYE_CY    + gy * (EYE_HH * scaleK - PUP_RY - 4);
  }

  tft.startWrite();
  tft.setAddrWindow(RX0, RY0, RW, RH);
  for (int y = RY0; y < RY0 + RH; y++) {
    for (int x = RX0; x < RX0 + RW; x++) {
      uint16_t c = C_BG;
      int e = (x < 120) ? 0 : 1;
      float dx = x - EYE_CX[e], dy = y - EYE_CY;
      if (inRoundRect(dx, dy, hw, hh, r)) {
        c = C_EYE;
        float ux = (x - px[e]) / PUP_RX, uy = (y - py[e]) / PUP_RY;
        if (ux * ux + uy * uy <= 1.0f) {
          c = C_PUPIL;
          float sx = x - (px[e] - 6), sy = y - (py[e] - 7);
          if (sx * sx + sy * sy <= 25) c = C_SHINE;   // برق چشم
        }
      }
      lineBuf[x - RX0] = c;
    }
    tft.writePixels(lineBuf, RW);
  }
  tft.endWrite();
}

void updateEyes() {
  uint32_t now = millis();
  float tx = 0, ty = 0;   // هدف نگاه

  if (imuOk) {
    float ax, ay, az, gmag;
    if (readImu(ax, ay, az, gmag)) {
      if (calOk) {
        // تغییر نسبت به حالت صاف ، روی دو محوری که در کالیبره اندازه گرفتیم
        float d[3] = {ax - restNow[0], ay - restNow[1], az - restNow[2]};
        tx = constrain(dot3(d, cal.ex) * 1.8f, -1.0f, 1.0f);
        ty = constrain(dot3(d, cal.ey) * 1.8f, -1.0f, 1.0f);
      }

      float amag = sqrtf(ax * ax + ay * ay + az * az);
      if (gmag > 250 || fabsf(amag - 1) > 0.8f) {          // تکان شدید
        if (now > dizzyUntil) Serial.println("گیج شدم!");
        dizzyUntil = now + 2000;
      } else if (fabsf(amag - lastAmag) > 0.25f && now > dizzyUntil) {  // حرکت ناگهانی
        if (now > surpriseUntil) Serial.println("تعجب!");
        surpriseUntil = now + 600;
      }
      lastAmag = amag;
    }
  } else {
    // بدون شتاب‌سنج: نگاه آرام به این طرف و آن طرف
    tx = 0.7f * sinf(now / 1300.0f);
    ty = 0.3f * sinf(now / 1700.0f);
  }

  // نرم کردن حرکت مردمک
  gazeX += (tx - gazeX) * 0.25f;
  gazeY += (ty - gazeY) * 0.25f;

  // تعجب: چشم‌ها کمی بزرگ‌تر
  float targetK = (now < surpriseUntil) ? 1.15f : (now < dizzyUntil ? 0.92f : 1.0f);
  scaleK += (targetK - scaleK) * 0.3f;

  if (now < dizzyUntil) dizzyAng += 0.35f;

  // پلک زدن تصادفی (۱۸۰ میلی‌ثانیه)
  if (!blinking && now > nextBlink && now > dizzyUntil) {
    blinking = true; blinkStart = now;
  }
  if (blinking) {
    float t = (now - blinkStart) / 180.0f;
    if (t >= 1) { blinking = false; openness = 1; nextBlink = now + random(2000, 6000); }
    else openness = (t < 0.5f) ? 1 - t * 2 : (t - 0.5f) * 2;
  }
}

// ============================================
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  موچی - test 07 : انیمیشن + چشم");
  Serial.println("==============================");

  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
  tft.init(240, 280);
  tft.setSPISpeed(40000000);
  tft.setRotation(SCREEN_ROTATION);

  C_BG    = ST77XX_BLACK;
  C_ANIM  = tft.color565(255, 255, 255);
  C_EYE   = tft.color565(120, 220, 255);   // فیروزه‌ای روشن
  C_PUPIL = tft.color565(10, 22, 45);      // سرمه‌ای تیره
  C_SHINE = tft.color565(255, 255, 255);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  imuOk = setupImu();
  if (!imuOk) Serial.println("⚠️ شتاب‌سنج جواب نداد — چشم‌ها خودشان آرام می‌چرخند. سیم‌های G33/G32 را چک کن.");

  playBootAnimation(2000);

  if (imuOk) {
    calOk = !FORCE_CALIBRATION && loadCalibration();
    if (calOk) Serial.println("کالیبره‌ي ذخیره‌شده خوانده شد.");
    else runCalibration();

    // حالت صاف «همین الان»: اگر ربات ساکن است ، همین را مرکز نگاه حساب کن
    // (تا چشم‌ها اول کار دقیقا وسط باشند ، حتی اگر میز کمی کج است)
    float acc[3] = {0, 0, 0}; int n = 0;
    for (int i = 0; i < 25; i++) {
      float a[3], g;
      if (readA(a, g) && g < 20) { for (int j = 0; j < 3; j++) acc[j] += a[j]; n++; }
      delay(15);
    }
    if (n >= 20) { for (int j = 0; j < 3; j++) restNow[j] = acc[j] / n; }
    else         { for (int j = 0; j < 3; j++) restNow[j] = cal.g0[j]; }
  }

  tft.fillScreen(C_BG);
  randomSeed(micros());
  nextBlink = millis() + 1500;
  Serial.println("چشم‌ها روشن شدند. موچی را کج کن ، تکان بده ، و ببین.");
}

void loop() {
  uint32_t t0 = millis();
  updateEyes();
  drawEyes();
  static uint32_t lastReport = 0;
  if (millis() - lastReport > 2000) {
    lastReport = millis();
    Serial.print("فریم چشم: ");
    Serial.print(millis() - t0);
    Serial.print(" ms   نگاه x=");
    Serial.print(gazeX, 2);
    Serial.print(" y=");
    Serial.println(gazeY, 2);
  }
  uint32_t dt = millis() - t0;
  if (dt < 33) delay(33 - dt);    // حدود ۳۰ فریم در ثانیه
}
