#pragma once
// =====================================================
//   MOOCHI  —  کتابخانه‌ي کشيدن چهره
//   همه‌ي شکل‌ها روي بافر PSRAM کشيده مي‌شوند
//   هيچ تابعي اينجا delay ندارد — همه بايد سريع باشند
// =====================================================

#include <Arduino.h>
#include "moochi_lcd.h"

// ---------- بافر ----------
extern uint16_t *fb;

#define CX (LCD_W / 2)
#define CY (LCD_H / 2)

static inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
static inline uint16_t swap16(uint16_t c) { return (c >> 8) | (c << 8); }

// ---------- رنگ‌ها ----------
// تصميم‌هاي قطعي — در docs/طراحي-چهره-موچي.md ثبت شده‌اند
//
// ⚠️ سفيد جايش را به **بنفشِ ملايم** داد.
//   سفيد روي مشکي تيز است و حس «صفحه‌ي کامپيوتر» مي‌دهد.
//   بنفش روشن نرم‌تر مي‌نشيند و چهره را گرم‌تر مي‌کند.
const uint16_t C_EYE   = rgb(198, 188, 244);   // بنفش ملايم — رنگ اصلي چشم
const uint16_t C_PUPIL = rgb(  0,   0,   0);
const uint16_t C_BG    = rgb(  0,   0,   0);
// ⚠️ لبخند هم‌رنگ چشم است ، نه نعنايي.
//   اول نعنايي گذاشتم چون در ويدئوي مرجع همين بود ، ولي آنجا
//   بخشي از يک پالتِ ديگر بود. لبخندِ سبز روي اين چهره بي‌معنا
//   به نظر مي‌رسيد. درس: رنگ را از مرجع برندار ، از خودِ چهره
//   دربياور.
const uint16_t C_SMILE = rgb(198, 188, 244);   // هم‌رنگ چشم
const uint16_t C_MINT  = rgb( 62, 232, 200);   // نعنايي — فعلا استفاده نمي‌شود
const uint16_t C_MOUTH = rgb(240,  80, 100);   // دهانِ باز
const uint16_t C_PINK  = rgb(255, 150, 190);   // پاپيون
const uint16_t C_KNOT  = rgb(240, 120, 168);   // گره‌ي پاپيون
const uint16_t C_RED   = rgb(255,  40,  55);   // قلب
const uint16_t C_BLUSH = rgb(215,  85, 130);   // گونه
const uint16_t C_TEAR  = rgb(130, 200, 255);   // اشک
const uint16_t C_DIM   = rgb( 90,  90,  95);

// =====================================================
//   پالتِ نسل دوم — از ورقِ طراحيِ خودِ موچي
//
//   ⚠️ يک درس که قبلا گرفتيم و اين‌بار درست رعايتش کرديم:
//      «رنگ را از مرجع برندار ، از خودِ چهره دربياور.»
//      اين‌بار مرجعْ خودِ طرحِ موچي است ، نه ويدئوي کسِ ديگر.
//      پس رنگ‌ها مستقيم از همان ورق مي‌آيند.
//
//   نکته‌ي مهمِ طرح: دهان **هم‌رنگ چشم نيست**. چشم سفيد/بنفشِ
//   خيلي روشن است و دهان فيروزه‌اي. همين تضادِ کوچک چهره را
//   از «يک‌دست و تخت» بيرون مي‌آورد.
// =====================================================
const uint16_t C_WHITE  = rgb(244, 242, 255);   // سفيدِ چشم ، با تهِ بنفش
const uint16_t C_TEAL   = rgb( 46, 214, 208);   // دهانِ کوچک — فيروزه‌اي
const uint16_t C_HOT    = rgb(255,  72, 132);   // گونه و زبان — صورتيِ داغ
const uint16_t C_STAR   = rgb(255, 205,  60);   // جرقه‌ي ستاره‌اي
// ☠️ دست هم‌رنگِ چشم نبايد باشد.
//    اول C_WHITE گذاشتم و در «نگاه نکن» دست روي چشمِ سفيد
//    افتاد و ناپديد شد — فقط شکافِ لايِ انگشت‌ها ديده مي‌شد.
//    يک پله تيره‌تر کافي است تا مرز بيفتد ، بي‌آنکه از پالت
//    بيرون بزند.
const uint16_t C_HAND  = rgb(214, 208, 242);

const uint16_t C_BROW   = rgb(226, 222, 245);   // ابروي نازک

// شعله — از بيرون به درون روشن‌تر مي‌شود
const uint16_t C_FL_OUT = rgb(226,  38,  30);   // سرخِ بيروني
const uint16_t C_FL_MID = rgb(255, 136,  20);   // نارنجي
const uint16_t C_FL_IN  = rgb(255, 224,  92);   // زردِ مرکز

// ---------- جاي دهان ----------
// چشم‌ها تا حدود CY+42 پايين مي‌آيند و گونه‌ها روي CY+74 هستند.
// صفحه گرد است: در فاصله‌ي ۱۰۰ پيکسلي از مرکز ، نصف پهناي
// قابل ديدن حدود ۱۴۹ پيکسل است. پس دهان تا ۱۲۰ پيکسل جا دارد.
const int MOUTH_Y = 104;

// ---------- اندازه‌هاي چشم ----------
// ⚙️ نصف فاصله‌ي دو چشم — براي آزمايش قابل تنظيم است.
//    شعاعِ چشمِ گرد ۵۲ است ، پس:
//       ۷۸  فاصله‌ي زياد (حالتِ قبلي)
//       ۶۰  شانزده پيکسل فاصله
//       ۵۸  دوازده پيکسل — تقريبا چسبيده  ← الان
//       ۵۲  کاملا چسبيده
//       ۴۶  روي هم مي‌افتند
int EYE_DX = 58;
const int EYE_W  = 64;
const int EYE_H  = 84;
const int EYE_R  = 18;
const int PUP_R  = 14;

// اندازه‌هاي حالت خوشحال (گنبد)
const int HAP_W    = 74;
const int HAP_H    = 84;
const int HAP_R    = 24;
const int HAP_RISE = 42;

// =====================================================
//  ابزارهاي پايه
// =====================================================
inline void fbFill(uint16_t c) {
  uint16_t s = swap16(c);
  uint32_t v = ((uint32_t)s << 16) | s;
  uint32_t *p = (uint32_t *)fb;
  for (int i = 0; i < (LCD_W * LCD_H) / 2; i++) p[i] = v;
}

static inline void px(int x, int y, uint16_t s) {
  if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H) return;
  fb[y * LCD_W + x] = s;
}

inline void fbCircle(int cx, int cy, int r, uint16_t c) {
  uint16_t s = swap16(c);
  if (r < 1) r = 1;
  int r2 = r * r;
  for (int y = cy - r; y <= cy + r; y++) {
    int dy = y - cy;
    int w = (int)sqrtf((float)(r2 - dy * dy));
    for (int x = cx - w; x <= cx + w; x++) px(x, y, s);
  }
}

inline void fbEllipse(int cx, int cy, int rx, int ry, uint16_t c) {
  uint16_t s = swap16(c);
  if (rx < 1) rx = 1;
  if (ry < 1) ry = 1;
  for (int y = cy - ry; y <= cy + ry; y++) {
    float dy = (float)(y - cy) / ry;
    float k = 1.0f - dy * dy;
    if (k < 0) continue;
    int w = (int)(rx * sqrtf(k));
    for (int x = cx - w; x <= cx + w; x++) px(x, y, s);
  }
}

// بيضيِ چرخيده — براي دست ، که گِرد نيست بلکه کشيده و
// هم‌راستاي بازوست. هر پيکسل را برعکسِ زاويه مي‌چرخانيم و
// بعد با معادله‌ي بيضيِ ساده تست مي‌کنيم.
inline void fbEllipseRot(int cx, int cy, int rx, int ry, float ang, uint16_t c) {
  uint16_t s = swap16(c);
  if (rx < 1) rx = 1;
  if (ry < 1) ry = 1;
  int R = (rx > ry ? rx : ry) + 1;
  float ca = cosf(-ang), sa = sinf(-ang);
  for (int y = cy - R; y <= cy + R; y++)
    for (int x = cx - R; x <= cx + R; x++) {
      float dx = x - cx, dy = y - cy;
      float u = dx * ca - dy * sa;
      float v = dx * sa + dy * ca;
      if ((u * u) / (float)(rx * rx) + (v * v) / (float)(ry * ry) <= 1.0f)
        px(x, y, s);
    }
}

inline void fbRoundRect(int x, int y, int w, int h, int r, uint16_t c) {
  uint16_t s = swap16(c);
  if (w < 1 || h < 1) return;
  if (r > w / 2) r = w / 2;
  if (r > h / 2) r = h / 2;
  if (r < 0) r = 0;
  for (int j = 0; j < h; j++) {
    int inset = 0;
    if (r > 0) {
      if (j < r)           { int d = r - j;           inset = r - (int)sqrtf((float)(r*r - d*d)); }
      else if (j >= h - r) { int d = j - (h - r - 1); inset = r - (int)sqrtf((float)(r*r - d*d)); }
    }
    for (int i = inset; i < w - inset; i++) px(x + i, y + j, s);
  }
}

// چهارگوشِ گِردگوشه‌ي **چرخان**
//
//   ☠️ اين را لازم شدم چون کفِ دست را دايره کشيده بودم و
//      کاربر گفت «قلمبه است و شبيه دست نيست» — و درست
//      مي‌گفت. کفِ دست چهارگوش است ، نه توپ. fbRoundRect
//      نمي‌چرخد و fbEllipseRot چهارگوش نيست ، پس اين سومي
//      لازم بود.
inline void fbRRectRot(int cx, int cy, int hw, int hh, int rad,
                       float ang, uint16_t c) {
  uint16_t sw = swap16(c);
  float ca = cosf(-ang), sa = sinf(-ang);
  int span = (int)sqrtf((float)(hw * hw + hh * hh)) + 2;
  int ix = hw - rad, iy = hh - rad;      // مستطيلِ درونيِ بي‌گوشه
  if (ix < 0) ix = 0;
  if (iy < 0) iy = 0;
  for (int y = cy - span; y <= cy + span; y++)
    for (int x = cx - span; x <= cx + span; x++) {
      float dx = x - cx, dy = y - cy;
      float lx = dx * ca - dy * sa;      // به دستگاهِ خودِ شکل
      float ly = dx * sa + dy * ca;
      float ax = fabsf(lx), ay = fabsf(ly);
      if (ax > hw || ay > hh) continue;
      if (ax <= ix || ay <= iy) { px(x, y, sw); continue; }
      float qx = ax - ix, qy = ay - iy;  // فقط گوشه مي‌ماند
      if (qx * qx + qy * qy <= (float)(rad * rad)) px(x, y, sw);
    }
}

inline void fbRect(int x, int y, int w, int h, uint16_t c) {
  uint16_t s = swap16(c);
  for (int j = 0; j < h; j++)
    for (int i = 0; i < w; i++) px(x + i, y + j, s);
}

// منحني سهمي — پايه‌ي لبخند ، اخم ، و چشمِ قوسي
//   depth مثبت → وسطش پايين‌تر  (لبخند  ⌣)
//   depth منفي → وسطش بالاتر   (قوس    ⌒)
inline void fbCurve(int cx, int cy, int halfW, int depth, int thick, uint16_t c) {
  if (halfW < 1) halfW = 1;
  for (int x = cx - halfW; x <= cx + halfW; x++) {
    float t = (float)(x - cx) / halfW;
    fbCircle(x, cy + (int)(depth * (1.0f - t * t)), thick, c);
  }
}

inline void fbLine(int x1, int y1, int x2, int y2, int t, uint16_t c) {
  int steps = max(abs(x2 - x1), abs(y2 - y1));
  if (steps < 1) steps = 1;
  for (int i = 0; i <= steps; i++)
    fbCircle(x1 + (x2 - x1) * i / steps, y1 + (y2 - y1) * i / steps, t, c);
}

// قلب — از فرمول رياضي  (x²+y²−1)³ − x²y³ ≤ 0
inline void fbHeart(int cx, int cy, int size, uint16_t c) {
  uint16_t s = swap16(c);
  if (size < 6) size = 6;
  float sc = size / 2.4f;
  int span = (int)(1.3f * sc) + 2;
  for (int y = cy - span; y <= cy + span; y++)
    for (int x = cx - span; x <= cx + span; x++) {
      float nx = (x - cx) / sc;
      float ny = -(float)(y - cy) / sc - 0.2f;
      float a = nx * nx + ny * ny - 1.0f;
      if (a * a * a - nx * nx * ny * ny * ny <= 0) px(x, y, s);
    }
}

// ستاره‌ي چهارپر — منحني آستروييد ، لبه‌ها به داخل قوس دارند
inline void fbSparkle(int cx, int cy, int r, uint16_t c) {
  uint16_t s = swap16(c);
  if (r < 2) r = 2;
  for (int y = cy - r; y <= cy + r; y++)
    for (int x = cx - r; x <= cx + r; x++) {
      float nx = fabsf((float)(x - cx) / r);
      float ny = fabsf((float)(y - cy) / r);
      if (powf(nx, 0.667f) + powf(ny, 0.667f) <= 1.0f) px(x, y, s);
    }
}

// مارپيچ — چشمِ حالت سرگيجه
inline void fbSpiral(int cx, int cy, float turns, int rmax, int thick, float rot, uint16_t c) {
  const int steps = 200;
  for (int i = 0; i <= steps; i++) {
    float t   = (float)i / steps;
    float ang = rot + t * turns * 6.28318f;
    float r   = rmax * t;
    fbCircle(cx + (int)(r * cosf(ang)), cy + (int)(r * sinf(ang)), thick, c);
  }
}

// =====================================================
//  قطره‌ي آب
//
//  شکلِ درستِ قطره: **نوک‌تيز بالا ، گِرد و چاق پايين.**
//  اگر صاف بکشي‌اش مي‌شود يک ميله ؛ اگر فقط دايره بکشي
//  مي‌شود حباب. آنچه چشم به عنوان «قطره» مي‌شناسد همين
//  باريک شدنِ تدريجي به سمت نوک است.
//
//  نيمه‌ي پايين يک نيم‌دايره است.
//  نيمه‌ي بالا با فرمول  w = r × √(1−u)  باريک مي‌شود ،
//  که کنارها را کمي بيرون‌زده نگه مي‌دارد — نه مخروطِ صاف.
//
//  cy مرکزِ ته گِرد است ، tip ارتفاع نوک از آن.
// =====================================================
inline void fbDrop(int cx, int cy, int r, int tip, uint16_t c) {
  uint16_t s = swap16(c);
  if (r < 2) r = 2;
  if (tip < 2) tip = 2;

  for (int y = cy; y <= cy + r; y++) {          // ته گِرد
    int dy = y - cy;
    int w = (int)sqrtf((float)(r * r - dy * dy));
    for (int x = cx - w; x <= cx + w; x++) px(x, y, s);
  }
  for (int y = cy - tip; y < cy; y++) {          // باريک شدن تا نوک
    float u = (float)(cy - y) / tip;
    int w = (int)(r * sqrtf(1.0f - u));
    for (int x = cx - w; x <= cx + w; x++) px(x, y, s);
  }
  // برقِ ريز — همين يک نقطه است که آب را «خيس» نشان مي‌دهد
  if (r >= 4) fbCircle(cx - r / 3, cy - r / 4, r / 3, rgb(215, 240, 255));
}

// =====================================================
//  دستکشِ کارتوني
//
//  همه‌ي اجزا در يک دستگاه مختصاتِ محلي چيده مي‌شوند:
//     u = در امتدادِ بازو ، رو به بيرون
//     v = عمود بر بازو
//  بعد با زاويه‌ي مچ به مختصات صفحه تبديل مي‌شوند.
//
//  اين‌طوري دستکش هر جا که بازو برود درست مي‌چرخد و لازم
//  نيست براي هر زاويه دستي حساب کنيم.
//
//  sd = ‏−۱ چپ ، ‏+۱ راست — براي آينه کردنِ شست
// =====================================================
inline void gloveXY(float wx, float wy, float ca, float sa, int sd,
                    float u, float v, int &X, int &Y) {
  float vv = v * sd;
  X = (int)(wx + ca * u - sa * vv);
  Y = (int)(wy + sa * u + ca * vv);
}

inline void drawGlove(float wx, float wy, float ang, int sd, uint16_t c) {
  float ca = cosf(ang), sa = sinf(ang);
  int X, Y, x1, y1, x2, y2;

  gloveXY(wx, wy, ca, sa, sd,  0,   0, X, Y);          // مچ‌بند
  fbEllipseRot(X, Y, 5, 13, ang, c);

  gloveXY(wx, wy, ca, sa, sd, 15,   0, X, Y);          // کفِ دست
  fbEllipseRot(X, Y, 14, 14, ang, c);

  for (int i = -1; i <= 1; i++) {                       // سه انگشت
    gloveXY(wx, wy, ca, sa, sd, 29, i * 9.0f, X, Y);
    fbEllipseRot(X, Y, 10, 5, ang, c);
  }

  gloveXY(wx, wy, ca, sa, sd, 10, -16, X, Y);          // شست
  fbEllipseRot(X, Y, 9, 6, ang - sd * 1.1f, c);

  // جداسازيِ انگشت‌ها
  for (int i = 0; i < 2; i++) {
    float v = (i == 0) ? -4.5f : 4.5f;
    gloveXY(wx, wy, ca, sa, sd, 23, v, x1, y1);
    gloveXY(wx, wy, ca, sa, sd, 37, v, x2, y2);
    fbLine(x1, y1, x2, y2, 1, C_BG);
  }

  // درزهاي پشتِ دست — همان سه خطِ کوتاهِ آشناي دستکش
  for (int i = -1; i <= 1; i++) {
    gloveXY(wx, wy, ca, sa, sd,  7, i * 6.5f, x1, y1);
    gloveXY(wx, wy, ca, sa, sd, 19, i * 6.5f, x2, y2);
    fbLine(x1, y1, x2, y2, 1, C_BG);
  }
}

// حرف Z — براي حالت خواب
inline void fbZ(int cx, int cy, int size, int thick, uint16_t c) {
  int h = size, w = size;
  int x0 = cx - w / 2, y0 = cy - h / 2;
  fbLine(x0,     y0,     x0 + w, y0,     thick, c);   // خط بالا
  fbLine(x0 + w, y0,     x0,     y0 + h, thick, c);   // قطر
  fbLine(x0,     y0 + h, x0 + w, y0 + h, thick, c);   // خط پايين
}

inline void fbPush() { lcdBlit(0, 0, LCD_W - 1, LCD_H - 1, fb); }

// =====================================================
//  اجزاي چهره
// =====================================================

// ---------- ☠️ نامتقارنيِ عمدي — برداشته شد ----------
//
// استدلالم اين بود: «دو چشمِ دقيقا يکسان مکانيکي به نظر
// مي‌رسد ، تفاوتِ ريز حسِ زنده بودن مي‌دهد.»
//
// و در **نقاشيِ دستي** درست است. ولي در طرحِ گرافيکيِ موچي
// غلط بود ، به دو دليل که از ورق‌هاي مرجع فهميدم:
//
//   ۱ آن طرح‌ها **کاملا متقارن** هستند. تقارن بخشي از
//     زبانشان است ، نه کمبودشان.
//
//   ۲ وقتي چشم‌ها کوچک بودند تفاوتِ ۳ پيکسلي ديده نمي‌شد.
//     با چشمِ گردِ درشت (شعاع ۵۲) همان ۳ پيکسل آشکار شد و
//     چهره «کج» به نظر مي‌رسيد نه «زنده».
//
// درس: يک قاعده‌ي درستِ طراحي ، در سبکِ ديگري مي‌تواند غلط
// باشد. قاعده را از خودِ طرح بردار ، نه از اصولِ کلي.
//
// تابع مي‌ماند (بيست جا صدايش مي‌زنند) ولي صفر برمي‌گرداند.
inline void eyeSkew(int sd, int &dw, int &dh, int &dy, int &dp) {
  (void)sd;
  dw = 0;  dh = 0;  dy = 0;  dp = 0;
}

// چشم عادي: مستطيل گرد + مردمک
//   open 0..100 ، مردمک هم‌زمان با چشم لِه مي‌شود
inline void drawEye(int sd, int ox_face, int oy_face, int gazeX, int gazeY,
                    int open, int w = EYE_W, int hFull = EYE_H, int pr = PUP_R) {
  int dw, dh, dy, dp;
  eyeSkew(sd, dw, dh, dy, dp);
  w += dw;  hFull += dh;  pr += dp;

  int h = (hFull * open) / 100;
  if (h < 4) h = 4;

  int ex = CX + sd * EYE_DX + ox_face + gazeX;
  int ey = CY + oy_face + gazeY + dy;

  fbRoundRect(ex - w / 2, ey - h / 2, w, h, EYE_R, C_EYE);

  int prx = pr;
  int pry = (pr * open) / 100;
  if (pry < 1) pry = 1;

  // مردمک نصف سرعتِ نگاه حرکت مي‌کند — حس عمق
  int px_ = (gazeX * 50) / 100;
  int py_ = (gazeY * 50) / 100;
  int limX = w / 2 - prx - 5;  if (limX < 0) limX = 0;
  int limY = h / 2 - pry - 5;  if (limY < 0) limY = 0;
  if (px_ >  limX) px_ =  limX;   if (px_ < -limX) px_ = -limX;
  if (py_ >  limY) py_ =  limY;   if (py_ < -limY) py_ = -limY;

  fbEllipse(ex + px_, ey + py_, prx, pry, C_PUPIL);
}

// =====================================================
//   نسلِ دومِ چشم‌ها
//
//   ايده‌ي کلي (انتخابِ «ترکيبي»):
//     حالت‌هاي آرام  → چشمِ **درشت و براق** با مردمک
//                      تا نگاه کردن و دنبالِ انگشت رفتن
//                      معنا داشته باشد
//     احساسِ قوي    → چشم به **قلم‌مو** تبديل مي‌شود ،
//                      مردمک ناپديد ، همه‌چيز در شکلِ خط
//
//   و خودِ اين تغييرِ سبک يک نشانه است: وقتي مردمک‌ها
//   مي‌روند ، يعني «اين‌جا احساس جدي است».
// =====================================================

// ---------- قلم‌موي پايه ----------
//
// يک نوارِ ضخيمِ گِرده‌سر که مي‌تواند خم شود و بچرخد.
// همه‌ي چشم‌هاي احساسي از همين يکي ساخته مي‌شوند.
//
//   halfW  نصفِ طول
//   thick  ضخامت
//   curve  خميدگي — مثبت يعني قوسِ رو به بالا (⌒)
//                   منفي يعني رو به پايين (⌄)
//                   صفر يعني خطِ صاف
//   tilt   چرخشِ کل ، بر حسب راديان
inline void brushStroke(int cx, int cy, int halfW, int thick,
                        float curve, float tilt, uint16_t c) {
  float ca = cosf(tilt), sa = sinf(tilt);
  int   r  = thick / 2;
  if (r < 1) r = 1;
  // گام را از ضخامت کوچک‌تر مي‌گيريم تا نوار پيوسته دربيايد
  // 🛡 اگر halfW صفر شود ، steps صفر مي‌شود و تقسيم بر صفر
  //    مي‌دهد. هيچ‌وقت در عمل نديديمش ، ولي `scale` از بيرون
  //    مي‌آيد و روزي ممکن است کوچک فرستاده شود.
  if (halfW < 1) halfW = 1;
  //   ☠️ باگي که تا رندرِ بيرون از برد نديدمش:
  //
  //      `(int)` به سمتِ **صفر** گرد مي‌کند. مختصاتِ چپِ نوار
  //      منفي است و خطاي ريزِ float ‏۴۰٫۹۹۹۹۹− را ۴۰− مي‌کرد
  //      نه ۴۱− ، پس ستونِ ۴۱− اصلا مهر نمي‌خورد.
  //
  //      دايره‌ي شعاعِ ۹ فقط در ستونِ **مرکزش** رديفِ بالايي را
  //      پر مي‌کند ، پس هر ستونِ جاافتاده يک دندانه‌ي يک‌پيکسلي
  //      روي لبه‌ي بالا و پايين مي‌شد. چشمِ خطي «شانه» شده بود.
  //      اين روي هر ابرو و هر قوس و هر دهانِ قلم‌مويي اثر داشت.
  //
  //      دو اصلاح ، چون يکي‌اش کافي نيست:
  //        ۱ گردکردن با lroundf به‌جاي بريدن
  //        ۲ وصل کردنِ هر مهر به مهرِ قبلي — اين‌طور حتي اگر
  //          گام از يک پيکسل بيشتر شود هم درز نمي‌افتد.
  int steps = halfW * 2;
  int prevX = 0, prevY = 0;
  for (int i = 0; i <= steps; i++) {
    float u  = -1.0f + 2.0f * i / steps;        // ‎−۱ تا ۱
    float lx = u * halfW;
    float ly = -curve * (1.0f - u * u);         // سهمي
    int   x  = cx + (int)lroundf(ca * lx - sa * ly);
    int   y  = cy + (int)lroundf(sa * lx + ca * ly);
    //   ☠️ و بلافاصله يک باگِ تازه ساختم: پارامترِ آخرِ fbLine
    //      **شعاع** است نه ضخامت (خودش را به fbCircle پاس
    //      مي‌دهد). r*2 فرستادم ، پس هر قلم‌مو دو برابر کلفت
    //      شد و «< >» به يک لکه تبديل شد.
    //      درس: نامِ پارامتر را نخوان ، تعريفش را بخوان.
    if (i > 0) fbLine(prevX, prevY, x, y, r, c);
    fbCircle(x, y, r, c);
    prevX = x;  prevY = y;
  }
}

// ---------- شکل‌هاي چشمِ قلم‌مويي ----------
enum EyeShape {
  ES_ARC_UP,     // ⌒  خوشحال ، راضي
  ES_ARC_DOWN,   // ⌄  ناراحت ، نگران
  ES_CARET,      // >  ذوق‌زده ، چشمکِ محکم
  ES_LINE,       // ―  بي‌تفاوت ، خسته
  ES_HEART,      // ♥  عاشق
  ES_RING,       // ○  جا خورده ، بوس
  ES_UP,         // ˄  چشم‌غره — نگاه به بالا
  ES_WOBBLE      // ⌄ لرزان — بغض
};

// چشمِ قلم‌مويي.
//   sd     −۱ چپ ، ‎+۱ راست
//   ph     فاز براي لرزش و تپش ؛ معمولا زمانِ حالت
inline void drawBrushEye(int sd, int ox_face, int oy_face,
                         EyeShape shape, float ph, uint16_t c = C_WHITE,
                         int scale = 100) {
  int dw, dh, dy, dp;
  eyeSkew(sd, dw, dh, dy, dp);          // همان نامتقارنيِ عمدي

  int ex = CX + sd * EYE_DX + ox_face;
  int ey = CY + oy_face + dy;

  // در طرحِ مرجع ، قوس‌ها **کوتاه‌تر و ضخيم‌تر** از چيزي
  // هستند که اول زدم. نسبتِ طول به ضخامت حدود ۲٫۶ است.
  int hw = (42 * scale) / 100 + dw;     // نصف طول
  int th = (18 * scale) / 100;          // ضخامت
  if (th < 5) th = 5;

  switch (shape) {

    case ES_ARC_UP:
      // ⌒ — قوسِ رو به بالا. چشمِ خندان.
      brushStroke(ex, ey + 8, hw, th, 26.0f + dh * 0.5f, 0, c);
      break;

    case ES_ARC_DOWN:
      // ⌄ — قوسِ رو به پايين. ابروي افتاده‌ي غم.
      brushStroke(ex, ey - 6, hw, th, -22.0f, 0, c);
      break;

    case ES_CARET: {
      // > — دو خطِ کوتاه که به هم مي‌رسند. ذوق و چشمک.
      // نوکِ زاويه هميشه رو به **بيرونِ** صورت است.
      float a = 0.62f;
      int   L = (hw * 62) / 100;
      int   tipx = ex + sd * L / 2;
      brushStroke(tipx - sd * L / 2, ey - L / 2 + 2, L / 2, th, 0,  sd * a, c);
      brushStroke(tipx - sd * L / 2, ey + L / 2 - 2, L / 2, th, 0, -sd * a, c);
      break;
    }

    case ES_LINE:
      //   ☠️ با طولِ کاملِ hw ، لبه‌ي داخليِ دو چشم فقط ۱۴ پيکسل
      //      از هم فاصله داشت و به چشم **يک نوارِ بلند** مي‌آمد ،
      //      نه دو چشمِ بسته. پلکِ بسته هم در واقع کوتاه‌تر از
      //      چشمِ باز است.
      brushStroke(ex, ey, (hw * 78) / 100, th, 0, 0, c);
      break;

    case ES_HEART: {
      int hr = (30 * scale) / 100;
      // تپش ريز — قلب که ساکن باشد مرده است
      hr += (int)(2.5f * sinf(ph * 6.0f + sd));
      fbHeart(ex, ey, hr, c);
      break;
    }

    case ES_RING: {
      int rr = (30 * scale) / 100;
      fbCircle(ex, ey, rr, c);
      fbCircle(ex, ey, rr - th, C_BG);
      break;
    }

    case ES_UP: {
      // ˄ — چشم‌غره. قوسِ رو به بالاي تيز ، و کمي بالاتر
      // از جاي معمول ، انگار مردمک رفته زير پلک.
      brushStroke(ex, ey - 12, hw, th, 34.0f, 0, c);
      break;
    }

    case ES_WOBBLE: {
      // بغض — قوسِ رو به پايين که مي‌لرزد.
      // ☠️ لرزش بايد **نامتقارن** باشد. اگر دو چشم هم‌فاز
      //    بلرزند ، مثل ويبره‌ي گوشي به نظر مي‌رسد نه بغض.
      float wob = 2.6f * sinf(ph * 17.0f + (sd > 0 ? 1.9f : 0.0f));
      int   wy  = (int)(1.8f * sinf(ph * 13.0f + sd * 2.2f));
      brushStroke(ex + (int)wob, ey - 4 + wy, hw, th, -20.0f, 0, c);
      break;
    }
  }
}

// ---------- چشمِ درشت و براق ----------
//
// نسخه‌ي بزرگ‌ترِ چشمِ قبلي با دو تفاوت:
//   ۱ بيضي است نه مستطيلِ گرد — نرم‌تر
//   ۲ يک برقِ کوچک بالا-چپِ مردمک دارد
//
// آن برق تنها چيزي است که چشم را «زنده» مي‌کند. بدونش
// مردمک يک سوراخِ سياه است.
inline void drawGlossyEye(int sd, int ox_face, int oy_face,
                          int gazeX, int gazeY, int open,
                          int w = 78, int hFull = 92) {
  int dw, dh, dy, dp;
  eyeSkew(sd, dw, dh, dy, dp);
  w += dw;  hFull += dh;

  int h = (hFull * open) / 100;
  if (h < 4) h = 4;

  int ex = CX + sd * EYE_DX + ox_face + gazeX;
  int ey = CY + oy_face + gazeY + dy;

  fbEllipse(ex, ey, w / 2, h / 2, C_EYE);

  // مردمکِ درشت — حدود ۵۵٪ پهناي چشم
  int prx = (w * 27) / 100 + dp;
  int pry = (prx * open) / 100;
  if (pry < 1) pry = 1;

  int px_ = (gazeX * 55) / 100;
  int py_ = (gazeY * 55) / 100;
  int limX = w / 2 - prx - 6;  if (limX < 0) limX = 0;
  int limY = h / 2 - pry - 6;  if (limY < 0) limY = 0;
  if (px_ >  limX) px_ =  limX;   if (px_ < -limX) px_ = -limX;
  if (py_ >  limY) py_ =  limY;   if (py_ < -limY) py_ = -limY;

  int cx_ = ex + px_, cy_ = ey + py_;
  fbEllipse(cx_, cy_, prx, pry, C_PUPIL);

  // برقِ چشم — فقط وقتي چشم به اندازه‌ي کافي باز است
  if (open > 45) {
    int gr = prx / 3;  if (gr < 2) gr = 2;
    fbCircle(cx_ - prx / 3, cy_ - pry / 2, gr, C_EYE);
    // يک برقِ ريزترِ پايين-راست ، براي عمق
    int g2 = gr / 2;
    if (g2 >= 1) fbCircle(cx_ + prx / 2, cy_ + pry / 2, g2, C_EYE);
  }
}

// =====================================================
//   نسلِ سوم — از ورقِ طراحيِ خودِ موچي
//
//   تفاوت‌هاي کليدي با آن‌چه داشتيم:
//     ۱ چشم **دايره** است نه مستطيلِ گرد ، و **سفيد** نه بنفش
//     ۲ مردمکِ مشکيِ درشت با **جرقه‌ي ستاره‌اي** داخلش ، نه
//       برقِ گِردِ ساده. آن ستاره چيزي است که چهره را از
//       «ربات» به «شخصيت» مي‌برد.
//     ۳ دهان فيروزه‌اي و **خيلي کوچک** است
//     ۴ ابرو جداست و فقط در چند حالت مي‌آيد
// =====================================================

// ---------- جرقه‌ي ستاره‌اي داخل مردمک ----------
//
// چهار پرِ باريک. از fbSparkle (که اَستروييد است) استفاده
// مي‌کنيم چون دقيقا همين شکل را مي‌دهد: پرهاي کشيده‌ي نازک
// با مرکزِ پُر — نه يک الماسِ صاف.
inline void pupilStar(int cx, int cy, int r, uint16_t c = C_WHITE) {
  if (r < 3) { fbCircle(cx, cy, r < 1 ? 1 : r, c); return; }
  fbSparkle(cx, cy, r, c);
}

// ---------- چشمِ گردِ درشت ----------
//
//   r      شعاعِ سفيديِ چشم
//   open   ۰ تا ۱۰۰ — پلک
//   stars  چند جرقه داخل مردمک (۰ ، ۱ يا ۲)
//   stars : ۰ بي‌برق · ۱ يک برق · ۲ دو برق (پيش‌فرضِ طرح)
//           ۳ جرقه‌ي ستاره‌اي (فقط ذوق و شگفتي)
inline void eyeRound(int sd, int ox_face, int oy_face,
                     int gazeX, int gazeY, int open,
                     int r = 52, int stars = 2) {
  int dw, dh, dy, dp;
  eyeSkew(sd, dw, dh, dy, dp);
  r += dw;                                  // نامتقارنيِ عمدي

  int ry = (r * open) / 100;
  if (ry < 3) ry = 3;

  int ex = CX + sd * EYE_DX + ox_face + gazeX;
  int ey = CY + oy_face + gazeY + dy;

  fbEllipse(ex, ey, r, ry, C_WHITE);

  // مردمک حدود ۶۰٪ چشم — در طرح خيلي درشت است
  int pr  = (r * 60) / 100 + dp;
  int pry = (pr * open) / 100;
  if (pry < 1) pry = 1;

  // مردمک نصفِ سرعتِ نگاه حرکت مي‌کند — حسِ عمق
  int px_ = (gazeX * 45) / 100;
  int py_ = (gazeY * 45) / 100;
  int limX = r - pr - 5;    if (limX < 0) limX = 0;
  int limY = ry - pry - 4;  if (limY < 0) limY = 0;
  if (px_ >  limX) px_ =  limX;   if (px_ < -limX) px_ = -limX;
  if (py_ >  limY) py_ =  limY;   if (py_ < -limY) py_ = -limY;

  int cx_ = ex + px_, cy_ = ey + py_;
  fbEllipse(cx_, cy_, pr, pry, C_PUPIL);

  // ☠️ اول pr/2 زده بودم — نصفِ مردمک! روي صفحه غول بود و
  //    مردمک را مي‌بلعيد. در طرحِ مرجع جرقه حدود **يک‌سومِ**
  //    مردمک است ، يعني pr/3 ، و آن يکي ريزتر.
  //
  // ☠️☠️ و بعد از بازبينيِ دوباره‌ي ورق‌ها ، يک اشتباهِ
  //    بزرگ‌ترِ سبکي پيدا شد:
  //
  //    **حالتِ پيش‌فرضِ مردمک ، دو برقِ گِرد است ، نه جرقه‌ي
  //    ستاره‌اي.** يکي درشت بالا-چپ ، يکي ريزتر پايين-راست.
  //    جرقه‌ي ستاره‌اي فقط در چند حالتِ خاص هست (ذوق و شگفتي).
  //
  //    من ستاره را پيش‌فرض کرده بودم و همه‌ي چهره‌ها را کمي
  //    «خيره» و غيرطبيعي نشان مي‌داد. اين تفاوتِ ريز ، در
  //    ۱۲۶ چهره ضرب مي‌شود.
  //
  //    درس: وقتي يک المان در **همه‌جا** تکرار مي‌شود ،
  //    اشتباهِ کوچکش هم همه‌جا تکرار مي‌شود. اين‌ها را بايد
  //    دقيق‌تر از بقيه بررسي کرد.
  if (open > 45 && stars > 0) {
    if (stars >= 3) {
      // حالتِ ستاره‌اي — فقط براي ذوق و شگفتي
      int s1 = pr / 3;  if (s1 < 3) s1 = 3;
      pupilStar(cx_ - sd * pr / 3, cy_ - pry / 3, s1);
      int s2 = pr / 6;  if (s2 < 2) s2 = 2;
      pupilStar(cx_ + sd * pr / 2, cy_ + pry / 2, s2);
    } else {
      // ---------- حالتِ پيش‌فرض: دو برقِ گِرد ----------
      // نسبتِ اندازه از ورق‌ها: برقِ بزرگ حدود ۴۰٪ مردمک ،
      // کوچک حدود ۱۸٪.
      int g1 = (pr * 40) / 100;  if (g1 < 3) g1 = 3;
      fbCircle(cx_ - sd * pr / 3, cy_ - pry / 3, g1, C_WHITE);
      if (stars > 1) {
        int g2 = (pr * 18) / 100;  if (g2 < 2) g2 = 2;
        fbCircle(cx_ + sd * pr / 2, cy_ + pry / 2, g2, C_WHITE);
      }
    }
  }
}

// =====================================================
//   ابرو — نسلِ دوم
//
//   در ورق‌هاي مرجع ابرو **تقريبا در همه‌ي چهره‌ها هست** و
//   بيشترِ احساس را همان مي‌سازد. من اول فقط در چهار حالت
//   گذاشته بودمش و صورت «چشم و دهان» بود نه صورت.
//
//   شکلش: کمانِ نازکِ سفيدِ کوتاه ، با فاصله‌ي روشن از چشم.
//   نه ضخيم مثل چشمِ قلم‌مويي ، نه چسبيده به پلک.
//
//   سه چيز شکلش را تعيين مي‌کند:
//     lift   چقدر بالاي مرکزِ چشم بنشيند
//     tilt   چرخش — علامتش نسبت به **بيرونِ صورت** است
//     curve  خميدگي — صفر يعني خطِ صاف
// =====================================================
enum BrowShape {
  BR_NONE,      // بدون ابرو
  BR_CALM,      // کمانِ آرام — حالتِ پيش‌فرض
  BR_UP,        // بالا رفته — تعجب ، ذوق
  BR_SAD,       // سرِ بيروني افتاده — غم ، نگراني
  BR_ANGRY,     // سرِ داخلي افتاده — اخم
  BR_ONE_UP,    // فقط يکي بالا — شک ، کنجکاوي
  BR_FLAT       // صاف — بي‌حوصلگي
};

// ---------- نگاهِ جاري ، براي همراهيِ ابرو ----------
//
// ابروي ثابت روي چشمِ متحرک ، صورت را چسبيده و مصنوعي نشان
// مي‌دهد. در صورتِ واقعي ابرو با چشم حرکت مي‌کند — ولي
// **کمتر** ، چون به استخوانِ پيشاني بسته است نه به کاسه‌ي
// چشم.
//
//   افقي  ۳۵٪ حرکتِ چشم
//   عمودي ۵۵٪ — ابرو در جهتِ بالا و پايين آزادتر است
//
// اين دو عدد را به‌عمد نامساوي گذاشتم. اگر هر دو ۱۰۰٪ باشند
// ابرو روي چشم مي‌چسبد ؛ اگر هر دو صفر باشند صورت يخ مي‌زند.
int browGazeX = 0, browGazeY = 0;
inline void setBrowGaze(int gx, int gy) { browGazeX = gx; browGazeY = gy; }

// tilt مثبت = سرِ **بيروني** بالاتر
inline void browStroke(int sd, int ox_face, int oy_face,
                       int lift, float tilt, float curve,
                       int halfW = 24, int thick = 8,
                       uint16_t c = C_BROW) {
  int ex = CX + sd * EYE_DX + ox_face + sd * 4 + (browGazeX * 35) / 100;
  int ey = CY + oy_face - lift + (browGazeY * 55) / 100;
  brushStroke(ex, ey, halfW, thick, curve, -sd * tilt, c);
}

// ابروي يک سمت ، بر اساسِ شکلِ نام‌دار
inline void eyeBrowShape(int sd, int ox_face, int oy_face,
                         BrowShape sh, int lift = 74,
                         uint16_t c = C_BROW) {
  switch (sh) {
    case BR_NONE:  break;
    // قوسِ ملايم ، کمي رو به بالا — چهره‌ي آرام
    case BR_CALM:  browStroke(sd, ox_face, oy_face, lift,  0.00f, 8.0f); break;
    // بالاتر و قوس‌دارتر — «واقعا؟»
    case BR_UP:    browStroke(sd, ox_face, oy_face, lift + 14, 0.10f, 11.0f); break;
    // سرِ بيروني مي‌افتد → غم. زاويه‌اش کم ولي اثرش زياد است.
    case BR_SAD:   browStroke(sd, ox_face, oy_face, lift,  0.38f, 5.0f); break;
    // سرِ داخلي مي‌افتد → اخم. **علامتِ منفي** ، دقيقا برعکسِ غم.
    case BR_ANGRY: browStroke(sd, ox_face, oy_face, lift, -0.50f, 3.0f, 26, 9); break;
    // فقط ابروي چپ بالا — کنجکاوي
    case BR_ONE_UP:
      if (sd < 0) browStroke(sd, ox_face, oy_face, lift + 18, 0.14f, 10.0f);
      else        browStroke(sd, ox_face, oy_face, lift,      0.00f,  7.0f);
      break;
    case BR_FLAT:  browStroke(sd, ox_face, oy_face, lift,  0.00f, 0.0f, 22, 7); break;
  }
  (void)c;
}

// هر دو ابرو با هم — پرمصرف‌ترين حالت
inline void drawBrows(int ox_face, int oy_face, BrowShape sh, int lift = 74) {
  if (sh == BR_NONE) return;
  eyeBrowShape(-1, ox_face, oy_face, sh, lift);
  eyeBrowShape(+1, ox_face, oy_face, sh, lift);
}

// ---------- سازگاري با کدِ قبلي ----------
// چند حالت هنوز eyeBrow قديمي را صدا مي‌زنند. نگهش مي‌داريم
// تا چيزي نشکند ، ولي تازه‌ها از drawBrows استفاده مي‌کنند.
inline void eyeBrow(int sd, int ox_face, int oy_face,
                    int lift, float tilt, uint16_t c = C_BROW) {
  browStroke(sd, ox_face, oy_face, lift, tilt, 9.0f, 24, 8, c);
}

// ---------- گونه: سه خطِ مورب ----------
//
// در طرح ، گونه دو شکل دارد: بيضيِ پُر (حالتِ ذوق) و سه خطِ
// مورب (حالتِ خجالت و بوس). دومي ظريف‌تر است.
// ☠️ نسخه‌ي اول ديده نمي‌شد: خط‌ها ۱۳ پيکسل و ضخامت ۵ بود.
//    روي صفحه‌ي ۳۶۰ پيکسلي اين تقريبا نامرئي است. در طرحِ
//    مرجع هر خط حدود يک‌ششمِ پهناي صورت است.
//    درس: اندازه‌ها را با نسبتِ صفحه بسنج ، نه با حس.
inline void blushHatch(int ox_face, int oy_face, int scale = 100) {
  for (int sd = -1; sd <= 1; sd += 2) {
    int bx = CX + sd * (EYE_DX + 42) + ox_face;
    int by = CY + 56 + oy_face;
    for (int k = -1; k <= 1; k++) {
      // ☠️ دوبار اشتباه پشت سر هم: اول ۱۳ بود و نامرئي ،
      //    بعد ۲۴ زدم و «خيلي خيلي بزرگ» شد. ۱۷ وسطِ درست
      //    است. درسِ عمومي‌تر: وقتي چيزي کوچک است ، دو برابر
      //    نکن — يک‌ونيم برابر کن و دوباره ببين.
      int L  = (17 * scale) / 100;
      int gx = bx + k * 15;
      brushStroke(gx, by + k * 3, L, 6, 0, -0.72f, C_HOT);
    }
  }
}

// گونه‌ي بيضيِ پُر — درشت‌تر و صريح‌تر
inline void blushOval(int ox_face, int oy_face, int wide = 26) {
  fbEllipse(CX - EYE_DX - 10 + ox_face, CY + 60 + oy_face, wide, 12, C_HOT);
  fbEllipse(CX + EYE_DX + 10 + ox_face, CY + 60 + oy_face, wide, 12, C_HOT);
}

// ---------- چشمِ شعله‌اي ----------
//
// به جاي دودِ دماغ. سه لايه‌ي تو در تو: سرخ ، نارنجي ، زرد.
// هر لايه زبانه‌هايي دارد که بالا و پايين مي‌روند.
//
// ☠️ زبانه‌ها بايد **نامنظم** باشند. اگر همه يک‌شکل و هم‌فاز
//    بالا و پايين بروند ، شبيه شعله نيست — شبيه يک ارّه‌ي
//    متحرک است. پس هر زبانه فاز و بلنديِ خودش را دارد.
// ---------- يک زبانه‌ي شعله ----------
//
// ☠️ نسخه‌ي اول شعله نشد ، و علتش را حالا مي‌فهمم:
//    يک **بيضيِ موج‌دار** کشيده بودم. موجِ روي لبه‌اش را
//    زياد و کم مي‌کردم و اميد داشتم شبيه شعله شود.
//    نشد ، چون شعله لبه‌ي موج‌دار ندارد — **زبانه** دارد.
//
//    تفاوتشان بنيادي است: موج يعني يک شکلِ بسته که کناره‌اش
//    مي‌لرزد. زبانه يعني چند شاخه‌ي جدا که هرکدام نوکِ تيزِ
//    خودش را دارد و مستقل بالا و پايين مي‌رود.
//
// هر زبانه:
//   تهِ گِرد (نيم‌دايره)  →  بالاتنه‌ي باريک‌شونده  →  نوکِ تيز
//   و نوکش با v² کج مي‌شود ، يعني پايين آرام و بالا تند —
//   دقيقا کاري که آتشِ واقعي مي‌کند.
inline void flameTongue(int bx, int by, int W, int H,
                        float sway, float t, float speed, float phase,
                        uint16_t c) {
  uint16_t sw = swap16(c);
  if (W < 2 || H < 4) return;

  for (int i = 0; i <= H; i++) {
    float v = (float)i / H;                 // ۰ ته ، ۱ نوک

    float w;
    if (v < 0.22f) {
      // تهِ گِرد — نيم‌دايره ، تا پايه پهن و نرم بنشيند
      float u = (0.22f - v) / 0.22f;
      w = W * sqrtf(1.0f - u * u);
    } else {
      // بالاتنه — تواني ‎۰٫۷ که نوکِ تيز مي‌دهد.
      // با توانِ ۱ مثلث مي‌شود ، با ۰٫۵ گلابي. ۰٫۷ وسطِ
      // درست است.
      w = W * powf((1.0f - v) / 0.78f, 0.7f);
    }

    // کجيِ نوک — با v² ، پس پايين تقريبا ثابت و بالا تند
    float bend = sway * v * v * sinf(t * speed + phase);
    // و يک لرزشِ ريزِ مستقل روي پهنا
    w *= (1.0f + 0.12f * sinf(t * (speed * 1.7f) + phase * 2.3f + v * 5.0f));

    int cx = bx + (int)bend;
    int y  = by - i;
    int hw = (int)w;
    if (hw < 1) hw = 1;
    for (int x = cx - hw; x <= cx + hw; x++) px(x, y, sw);
  }
}

// ---------- چشمِ شعله‌اي ----------
//
// سه زبانه‌ي هم‌پوشان (بلندِ وسط ، دو کوتاهِ کنار) و سه لايه‌ي
// رنگ روي هم. هر زبانه فاز و سرعتِ خودش را دارد.
//
// ☠️ اگر زبانه‌ها هم‌فاز باشند ، کلِ شعله مثل يک تکه بالا و
//    پايين مي‌رود و مثلِ پرچم به نظر مي‌رسد نه آتش.
inline void eyeFlame(int sd, int ox_face, int oy_face, float t) {
  int ex = CX + sd * EYE_DX + ox_face;
  int by = CY + oy_face + 52;                 // پايه‌ي شعله

  // هر زبانه: جابه‌جايي ، پهنا ، بلندي ، تاب ، سرعت ، فاز
  struct T { int dx, w, h; float sway, sp, ph; };
  const T TG[3] = {
    { -18, 19,  74, 11.0f, 5.3f, 0.0f },      // کنارِ چپ
    {   2, 27, 112, 14.0f, 4.1f, 2.1f },      // بلندِ وسط
    {  20, 17,  62, 10.0f, 6.2f, 4.0f },      // کنارِ راست
  };

  // سه لايه از بيرون به درون. هر لايه کوچک‌تر و روشن‌تر ،
  // پس مرکزِ شعله سفيدتر به نظر مي‌رسد — همان چيزي که در
  // آتشِ واقعي مي‌بيني.
  const uint16_t COL[3] = { C_FL_OUT, C_FL_MID, C_FL_IN };
  const float    SCW[3] = { 1.00f, 0.64f, 0.33f };
  const float    SCH[3] = { 1.00f, 0.74f, 0.46f };

  for (int L = 0; L < 3; L++)
    for (int k = 0; k < 3; k++) {
      int w = (int)(TG[k].w * SCW[L]);
      int h = (int)(TG[k].h * SCH[L]);
      flameTongue(ex + sd * TG[k].dx, by, w, h,
                  TG[k].sway, t, TG[k].sp, TG[k].ph + L * 0.7f, COL[L]);
    }
}

// ---------- دهان‌هاي تازه ----------

// لبخندِ فيروزه‌اي کوچک — دهانِ پيش‌فرضِ طرح
inline void mouthTiny(int ox, int oy, int halfW = 15, int depth = 8) {
  fbCurve(CX + ox, CY + MOUTH_Y + oy - 18, halfW, depth, 5, C_TEAL);
}

// زبانِ بيرون — شيطنت
inline void mouthTongue(int ox, int oy, float t) {
  int y = CY + MOUTH_Y + oy - 22;
  int wig = (int)(2.5f * sinf(t * 7.0f));         // تکانِ ريزِ زبان
  fbRoundRect(CX + ox - 17 + wig, y, 34, 52, 16, C_HOT);
  // خطِ وسطِ زبان
  fbLine(CX + ox + wig, y + 12, CX + ox + wig, y + 40, 3, rgb(226, 40, 96));
}

// دهانِ موجيِ غم — در طرح آبي است نه صورتي
inline void mouthSadWave(int ox, int oy) {
  int y = CY + MOUTH_Y + oy - 14;
  // کمانِ رو به پايينِ ضخيم با يک موج وسطش
  for (int i = -22; i <= 22; i++) {
    float u = i / 22.0f;
    int yy = y + (int)(9 * u * u) + (int)(3.5f * sinf(u * 6.3f));
    fbCircle(CX + ox + i, yy, 4, C_TEAR);
  }
}

// دهانِ بوس — شکلِ «۳» خوابيده
inline void mouthKiss3(int ox, int oy, float t) {
  int x = CX + ox - 4;
  int y = CY + MOUTH_Y + oy - 12;
  int p = (int)(1.5f * sinf(t * 5.0f));
  // دو قوسِ رو به چپ که روي هم نشسته‌اند
  for (int half = 0; half < 2; half++) {
    int cy_ = y + (half ? 13 : -13);
    int rr  = (half ? 13 : 14) + p;
    for (int a = -80; a <= 80; a += 3) {
      float rad = a * 3.14159f / 180.0f;
      fbCircle(x + (int)(rr * cosf(rad)), cy_ + (int)(rr * sinf(rad)), 4, C_HOT);
    }
  }
}

// =====================================================
//   سيستمِ ترکيبيِ چهره
//
//   چرا لازم شد: ۲۲ حالت يعني ۲۲ تابعِ جدا که هرکدام ۲۰ تا
//   ۵۰ خط است. براي رسيدن به صد و بيست و پنج چهره ، اين
//   مسير به سه هزار خطِ تکراري مي‌رسيد.
//
//   ايده: يک چهره **ترکيبِ پنج شکاف** است.
//
//        ابرو  +  چشم  +  دهان  +  لپ  +  اضافه
//
//   با شش تا ده گزينه در هر شکاف ، هزاران ترکيب ممکن
//   مي‌شود — و اضافه کردنِ يک چهره‌ي تازه يک **خط** است ،
//   نه يک تابع.
//
//   ☠️ ولي يک خطرِ جدي دارد که بايد از اول جلويش را گرفت:
//      ترکيبِ خودکار ، چهره‌هاي بي‌معني هم مي‌سازد. «چشمِ
//      خواب‌آلود + زبانِ بيرون + تاج» يک ترکيبِ معتبر است
//      ولي هيچ احساسي را نمي‌گويد.
//
//      پس ترکيب‌ها **دستي انتخاب مي‌شوند** ، نه با حلقه‌ي
//      تودرتو. سيستم فقط نوشتنشان را آسان مي‌کند.
// =====================================================

// ---------- شکل‌هاي چشم ----------
// (فراتر از EyeShape که براي قلم‌مو بود)
enum EyeKind {
  EK_ROUND,      // دايره‌ي سفيد با مردمک و جرقه — پيش‌فرض
  EK_ROUND2,     // همان ، با دو جرقه — براقِ ذوق‌زده
  EK_ARC,        // ⌒ خندان
  EK_CARET,      // > < ذوق
  EK_HEART,      // ♥ عاشق
  EK_RING,       // ○ جا خورده
  EK_LINE,       // ― بي‌تفاوت
  EK_WOBBLE,     // ⌄ لرزان — بغض
  EK_UP,         // ˄ چشم‌غره
  EK_SPIRAL,     // @ سرگيجه
  EK_SLEEP,      // ▁ خطِ خواب
  EK_FLAME,      // 🔥 عصباني
  EK_WINK_L,     // چشمکِ چپ  (چپ > ، راست گرد)
  EK_WINK_R,     // چشمکِ راست
  EK_STAR,       // ✦ چشمِ ستاره‌اي — شگفت‌زده
  EK_HALF,       // نيمه‌باز — خسته ، خمار
  // — دورِ دوم —
  EK_DOT,        // • مردمکِ تنها ، بي‌سفيدي — خشک و بي‌روح
  EK_SWIRL,      // ๑ گردابِ آرام — گيج
  EK_SQUINT,     // ︿ ريز کرده — مشکوک
  EK_CROSS,      // ✗ چشمِ ضربدري — از حال رفته
  EK_CROSS2,     // ✗ روي چشمِ باز — «نبين!»
  EK_ROLLUP      // ↥ مردمکِ چسبيده به بالا — کلافه
};

// ---------- شکل‌هاي دهان ----------
enum MouthKind {
  MK_NONE,
  MK_TINY,       // کمانِ ريزِ فيروزه‌اي — پيش‌فرض
  MK_WIDE,       // لبخندِ پهن‌تر
  MK_FLAT,       // خطِ صاف
  MK_OPEN,       // بيضيِ باز
  MK_TONGUE,     // زبانِ بيرون
  MK_KISS3,      // «۳» بوس
  MK_SADWAVE,    // موجِ آبيِ غم
  MK_WAVE,       // موجِ حالِ بد
  MK_SHOUT,      // فريادِ دندان‌دار
  MK_BLOB,       // لکه‌ي فيروزه‌اي — «او»
  MK_SMALL_O,    // دايره‌ي ريز
  // — دورِ دوم —
  MK_SQUIGGLE,   // ∿ خطِ مورمور — معذب
  MK_ZIGZAG,     // ⋀⋁ دندانه — ترسيده
  MK_OVAL,       // بيضيِ ايستاده — «اوه»
  MK_TONGUEOUT,  // زبانِ کاملا بيرون — شيطنت
  MK_CROOKED,    // کجِ يک‌طرفه — طعنه
  MK_BIGSMILE    // نيش‌خندِ پهنِ دندان‌نما
};

// ---------- لپ ----------
enum BlushKind { BK_NONE, BK_OVAL, BK_HATCH, BK_BIG };

// ---------- اضافات ----------
enum ExtraKind {
  XK_NONE,
  XK_SPARKLES,   // ستاره‌هاي دورِ سر
  XK_HEARTS,     // قلب‌هاي بالارونده
  XK_TEARS,      // اشک
  XK_SWEAT,      // يک قطره‌ي عرق
  XK_CROWN,      // تاج
  XK_GLASSES,    // عينکِ آفتابي
  XK_CATEARS,    // گوشِ گربه
  XK_HORNS,      // شاخِ شيطونک
  XK_NOTES,      // نتِ موسيقي
  XK_BULB,       // لامپِ ايده
  XK_ZZZ,        // حرف‌هاي خواب
  XK_ANGERMARK,  // علامتِ رگِ عصبانيت
  XK_QUESTION,   // علامتِ سوال
  XK_BUBBLE,     // حبابِ خواب
  // — دورِ دوم —
  XK_BATTERY,    // باتريِ خالي
  XK_CHARGING,   // باتري + صاعقه
  XK_MAGNIFIER,  // ذره‌بين
  XK_HOURGLASS,  // ساعتِ شني
  XK_SNOW,       // دانه‌ي برف
  XK_SUN,        // خورشيد
  XK_MOON,       // ماه
  XK_THERMO,     // دماسنج
  XK_FLOWER,     // گل
  XK_CANDY,      // آبنبات
  XK_WIZARDHAT,  // کلاهِ جادوگر
  XK_PARTYHAT,   // کلاهِ جشن
  XK_CONFETTI,   // کاغذرنگي
  XK_EYEPATCH,   // چشم‌بندِ دزدِ دريايي
  XK_BROKENHEART,// قلبِ شکسته
  XK_FANGS,      // دندانِ نيش
  XK_SWEATMULTI  // چند قطره عرق
};

// دست‌ها ميدانِ جداگانه دارند ، نه جاي «اضافات» —
// چون يک چهره مي‌تواند هم تاج داشته باشد و هم دست تکان دهد.
enum HandKind {
  HK_NONE,
  HK_WAVE,       // سلام
  HK_THUMBUP,    // 👍
  HK_VICTORY,    // ✌
  HK_POINT,      // اشاره
  HK_OVERMOUTH,  // دست جلوي دهان — خنده‌ي پنهان
  HK_OVEREYES,   // دو دست روي چشم — خجالت / ترس
  HK_OPEN,       // دو دستِ باز — «چي؟»
  HK_CLAP,       // کف زدن
  HK_HEARTHAND,  // قلب با دو دست
  HK_SALUTE,     // احترام
  HK_SHRUG,      // شانه بالا
  HK_SHY         // دو دستِ چسبيده به هم
};

// =====================================================
//   نسخه‌ي يک چهره
// =====================================================
struct FaceRecipe {
  const char *name;
  BrowShape   brow;
  EyeKind     eye;
  MouthKind   mouth;
  BlushKind   blush;
  ExtraKind   extra;
  uint8_t     flags;      // بيت ۰ = پاپيون · بيت ۱ = لرزش
  HandKind    hand;       // آخر آمد تا ۱۲۶ سطرِ قديمي دست نخورند
};
#define FR_BOW    0x01
#define FR_SHAKE  0x02

// بريدن کفِ چشم با منحني رو به بالا — پايه‌ي حالت خوشحال
inline void carveSmile(int ex, int bot, int halfW, int rise) {
  uint16_t s = swap16(C_BG);
  for (int x = ex - halfW - 6; x <= ex + halfW + 6; x++) {
    float t = (float)(x - ex) / (float)halfW;
    if (t < -1) t = -1;
    if (t >  1) t =  1;
    int by = bot - (int)(rise * (1.0f - t * t));
    for (int y = by; y <= bot + 14; y++) px(x, y, s);
  }
}

// چشم خوشحال — گنبد توپُر با کفِ لبخند
//   ★ طرح قطعي ، بعد از آزمودن هشت گزينه
inline void drawHappyEye(int sd, int ox_face, int oy_face, int open,
                         int w = HAP_W, int hFull = HAP_H, int rise = HAP_RISE) {
  int dw, dh, dy, dp;
  eyeSkew(sd, dw, dh, dy, dp);
  w += dw;  hFull += dh;

  int h = (hFull * open) / 100;
  if (h < 8) h = 8;
  int r = (rise * open) / 100;

  int ex  = CX + sd * EYE_DX + ox_face;
  int ey  = CY + oy_face + dy;
  int top = ey - h / 2;
  int bot = ey + h / 2;

  fbRoundRect(ex - w / 2, top, w, h, HAP_R, C_EYE);
  carveSmile(ex, bot, w / 2, r);
}

// پلک شيب‌دار
//   dInner = چقدر از سمت داخل (طرف بيني) پايين بيايد
//   dOuter = چقدر از سمت بيرون
//   ناراحت (4,40) · عصباني (42,6)
inline void slantLid(int sd, int ox_face, int oy_face, int gazeX, int gazeY,
                     int w, int hFull, int dInner, int dOuter) {
  uint16_t s = swap16(C_BG);
  int dw, dh, dy, dp;
  eyeSkew(sd, dw, dh, dy, dp);      // پلک بايد همان نامتقارني چشم را داشته باشد
  w += dw;  hFull += dh;

  int ex  = CX + sd * EYE_DX + ox_face + gazeX;
  int ey  = CY + oy_face + gazeY + dy;
  int top = ey - hFull / 2;
  for (int x = ex - w / 2 - 3; x <= ex + w / 2 + 3; x++) {
    float t = (float)(x - (ex - w / 2)) / w;
    float f = (sd < 0) ? t : 1.0f - t;        // 0 = بيرون ، 1 = داخل
    int lidY = top + (int)(dOuter + (dInner - dOuter) * f);
    for (int y = top - 6; y < lidY; y++) px(x, y, s);
  }
}

inline void drawBlush(int ox_face, int oy_face, int wide) {
  fbEllipse(CX - EYE_DX - 6 + ox_face, CY + 74 + oy_face, wide, 13, C_BLUSH);
  fbEllipse(CX + EYE_DX + 6 + ox_face, CY + 74 + oy_face, wide, 13, C_BLUSH);
}

// =====================================================
//  دهان
//
//  بزرگ‌ترين چيزي که چهره کم داشت. با دهان ، همان حالت‌هاي
//  موجود دو برابر گوياتر مي‌شوند بدون اينکه چشم‌ها عوض شوند.
//
//  depth مثبت = لبخند (وسطش پايين‌تر از دو سرش)
//  depth منفي = اخم
// =====================================================
inline void mouthCurve(int ox, int oy, int halfW, int depth, int thick, uint16_t c) {
  int cx = CX + ox;
  int cy = CY + MOUTH_Y + oy;
  for (int x = cx - halfW; x <= cx + halfW; x++) {
    float t = (float)(x - cx) / halfW;
    fbCircle(x, cy + (int)(depth * (1.0f - t * t)), thick, c);
  }
}

// خط صاف — حالت خنثي
inline void mouthLine(int ox, int oy, int halfW, int thick, uint16_t c) {
  mouthCurve(ox, oy, halfW, 0, thick, c);
}

// موج — براي سرگيجه و حالِ بد
inline void mouthWave(int ox, int oy, int halfW, int amp, int thick, uint16_t c) {
  int cx = CX + ox;
  int cy = CY + MOUTH_Y + oy;
  for (int x = cx - halfW; x <= cx + halfW; x++) {
    float t = (float)(x - cx) / halfW;
    fbCircle(x, cy + (int)(amp * sinf(t * 3.14159f * 2.2f)), thick, c);
  }
}

// دهانِ باز — براي جا خوردن. با زبان.
inline void mouthOpen(int ox, int oy, int rx, int ry, bool tongue) {
  int cx = CX + ox;
  int cy = CY + MOUTH_Y + oy;
  fbEllipse(cx, cy, rx, ry, C_MOUTH);
  if (tongue && ry > 10)
    fbEllipse(cx, cy + ry / 2, rx / 2, ry / 3, rgb(255, 140, 155));
}

// ---------- پاپيون نرم و متحرک ----------
inline void softWing(int cx, int cy, int w, int h, int dir, uint16_t c) {
  uint16_t s = swap16(c);
  for (int i = 2; i <= w; i++) {
    float t = (float)i / w;
    int hh = (int)(h * (0.28f + 0.72f * powf(t, 0.55f)));
    if (t > 0.80f) {                       // گرد کردن لبه‌ي بيروني
      float u = (t - 0.80f) / 0.20f;
      hh = (int)(hh * sqrtf(1.0f - u * u));
    }
    if (hh < 1) continue;
    int x = cx + dir * i;
    for (int y = cy - hh; y <= cy + hh; y++) px(x, y, s);
  }
}

// سه حرکت با سرعت‌هاي متفاوت ، تا مکانيکي به نظر نرسد
inline void drawBow(int cx, int cy, float phase, int w = 26, int h = 16) {
  int bob  = (int)(3.5f * sinf(phase));
  int tilt = (int)(4.5f * sinf(phase * 0.73f));
  int sway = (int)(6.0f * sinf(phase * 0.61f));
  int by   = cy + bob;

  for (int d = -1; d <= 1; d += 2) {           // دنباله‌هاي روبان
    int tipX = cx + d * 13 + sway;
    int tipY = by + 26;
    for (int i = 0; i <= 22; i++) {
      float t = (float)i / 22.0f;
      fbCircle(cx + (int)((tipX - cx) * t),
               by + (int)((tipY - by) * t),
               (int)(5.0f * (1.0f - 0.55f * t)), C_PINK);
    }
  }
  softWing(cx, by - tilt, w, h, -1, C_PINK);
  softWing(cx, by + tilt, w, h, +1, C_PINK);
  fbEllipse(cx, by, h / 2 + 1, h / 2 + 2, C_KNOT);
}

// جاي ثابت پاپيون: بالاي چشم چپ
inline void drawBowDefault(int ox_face, int oy_face, float phase) {
  drawBow(CX - EYE_DX - 34 + ox_face, CY - 66 + oy_face, phase);
}

// =====================================================
//   اضافات — از ورق‌هاي طراحي
//
//   همه روي مختصاتِ نسبي به مرکزِ صورت کار مي‌کنند تا با
//   جابه‌جاييِ صورت هماهنگ بمانند.
// =====================================================

// ---------- تاج ----------
// سه دندانه‌ي مثلثي روي يک نوار ، با گوي‌هاي ريز روي نوک‌ها
inline void xCrown(int ox, int oy, float t) {
  int bx = CX + ox, by = CY + oy - 118;
  int w = 30;
  int bob = (int)(2.0f * sinf(t * 2.2f));
  by += bob;
  // نوارِ پايه
  fbRoundRect(bx - w, by + 14, w * 2, 9, 4, C_STAR);
  // سه دندانه — وسطي بلندتر
  const int H[3] = { 20, 30, 20 };
  for (int k = -1; k <= 1; k++) {
    int tx = bx + k * 20;
    int h  = H[k + 1];
    for (int i = 0; i <= h; i++) {
      float u = (float)i / h;
      int hw = (int)(9 * (1.0f - u));
      if (hw < 1) hw = 1;
      for (int x = tx - hw; x <= tx + hw; x++) px(x, by + 14 - i, swap16(C_STAR));
    }
    fbCircle(tx, by + 14 - h - 3, 4, C_HOT);     // گويِ نوک
  }
}

// ---------- عينکِ آفتابي ----------
// دو شيشه‌ي مشکي با قابِ فيروزه‌اي و يک برقِ مورب
inline void xGlasses(int ox, int oy) {
  for (int sd = -1; sd <= 1; sd += 2) {
    int gx = CX + sd * EYE_DX + ox;
    int gy = CY + oy;
    fbRoundRect(gx - 52, gy - 30, 104, 60, 16, C_TEAL);
    fbRoundRect(gx - 47, gy - 25,  94, 50, 13, C_PUPIL);
    // برقِ مورب روي شيشه
    for (int i = 0; i < 26; i++)
      fbLine(gx - 30 + i, gy - 18, gx - 14 + i, gy + 10, 1, C_WHITE);
  }
  // پلِ بينِ دو شيشه
  fbRect(CX + ox - EYE_DX + 52, CY + oy - 6,
         2 * EYE_DX - 104, 8, C_TEAL);
}

// ---------- گوشِ گربه ----------
inline void xCatEars(int ox, int oy) {
  for (int sd = -1; sd <= 1; sd += 2) {
    int bx = CX + ox + sd * 74;
    int by = CY + oy - 104;
    // مثلثِ بيروني
    for (int i = 0; i <= 46; i++) {
      float u = (float)i / 46;
      int hw = (int)(26 * (1.0f - u));
      int cx = bx + (int)(sd * u * 10);
      for (int x = cx - hw; x <= cx + hw; x++) px(x, by - i, swap16(C_WHITE));
    }
    // داخلِ صورتي
    for (int i = 0; i <= 32; i++) {
      float u = (float)i / 32;
      int hw = (int)(14 * (1.0f - u));
      int cx = bx + (int)(sd * u * 8);
      for (int x = cx - hw; x <= cx + hw; x++) px(x, by - 6 - i, swap16(C_HOT));
    }
  }
}

// ---------- شاخِ شيطونک ----------
inline void xHorns(int ox, int oy) {
  for (int sd = -1; sd <= 1; sd += 2) {
    int bx = CX + ox + sd * 66;
    int by = CY + oy - 104;
    for (int i = 0; i <= 40; i++) {
      float u = (float)i / 40;
      int hw = (int)(11 * (1.0f - u * 0.92f));
      // خمِ رو به بيرون
      int cx = bx + (int)(sd * u * u * 16);
      for (int x = cx - hw; x <= cx + hw; x++) px(x, by - i, swap16(C_HOT));
    }
  }
}

// ---------- نتِ موسيقي ----------
inline void xNotes(int ox, int oy, float t) {
  for (int k = 0; k < 3; k++) {
    float ph = fmodf(t * 0.8f + k * 0.34f, 1.0f);
    int sgn = (k % 2) ? 1 : -1;
    int nx = CX + ox + sgn * (86 + (int)(ph * 24));
    int ny = CY + oy - 56 - (int)(ph * 74);
    int sz = 9 - (int)(ph * 3);
    if (sz < 4) continue;
    uint16_t c = (k % 2) ? C_HOT : C_TEAL;
    fbEllipseRot(nx, ny, sz, sz - 2, -0.4f, c);         // سرِ نت
    fbRect(nx + sz - 2, ny - sz * 3, 3, sz * 3, c);     // دسته
    if (k == 1) fbRect(nx + sz - 2, ny - sz * 3, 10, 3, c);  // پرچم
  }
}

// ---------- لامپِ ايده ----------
inline void xBulb(int ox, int oy, float t) {
  int bx = CX + ox + 92, by = CY + oy - 96;
  int glow = (int)(2.0f * sinf(t * 6.0f));
  fbCircle(bx, by, 20 + glow, C_STAR);
  fbRect(bx - 8, by + 17, 16, 10, C_DIM);        // سرپيچ
  fbRect(bx - 8, by + 24, 16, 4, C_DIM);
  // پرتوها
  for (int k = 0; k < 6; k++) {
    float a = k * 1.047f;
    int x1 = bx + (int)(27 * cosf(a)), y1 = by + (int)(27 * sinf(a));
    int x2 = bx + (int)(37 * cosf(a)), y2 = by + (int)(37 * sinf(a));
    fbLine(x1, y1, x2, y2, 2, C_STAR);
  }
}

// ---------- قطره‌ي عرق ----------
inline void xSweat(int ox, int oy, float t) {
  float ph = fmodf(t * 0.9f, 1.0f);
  int sx = CX + ox + EYE_DX + 52;
  int sy = CY + oy - 62 + (int)(ph * 44);
  fbDrop(sx, sy, 9 - (int)(ph * 3), 15, C_TEAR);
}

// ---------- رگِ عصبانيت ----------
// آن علامتِ صليب‌مانندِ کارتوني روي شقيقه
inline void xAngerMark(int ox, int oy, float t) {
  int mx = CX + ox + EYE_DX + 46;
  int my = CY + oy - 86;
  int p  = (int)(2.0f * sinf(t * 9.0f));
  for (int k = 0; k < 2; k++) {
    int d = (k ? 1 : -1);
    fbLine(mx - 14, my + d * 10 + p, mx,      my - d * 4 + p, 4, C_FL_OUT);
    fbLine(mx,      my - d * 4 + p, mx + 14, my + d * 10 + p, 4, C_FL_OUT);
  }
}

// ---------- علامتِ سوال ----------
inline void xQuestion(int ox, int oy, float t) {
  int qx = CX + ox + EYE_DX + 48;
  int qy = CY + oy - 92 + (int)(3.0f * sinf(t * 2.4f));
  for (int a = -150; a <= 110; a += 6) {
    float r = a * 3.14159f / 180.0f;
    fbCircle(qx + (int)(15 * cosf(r)), qy + (int)(15 * sinf(r)), 4, C_STAR);
  }
  fbRect(qx - 2, qy + 14, 5, 12, C_STAR);
  fbCircle(qx, qy + 34, 5, C_STAR);
}

// ---------- حبابِ خواب ----------
inline void xBubble(int ox, int oy, float t) {
  float ph = fmodf(t * 0.35f, 1.0f);
  int r = 8 + (int)(ph * 16);
  int bx = CX + ox + 34, by = CY + oy + MOUTH_Y - 16;
  fbCircle(bx, by, r, C_TEAR);
  fbCircle(bx, by, r - 3, C_BG);
  if (r > 8) fbCircle(bx - r / 3, by - r / 3, 3, C_WHITE);
}

// =====================================================
//   دست‌ها — يک ورقِ کاملِ مرجع   (بازنويسيِ دوم)
//
//   ☠️ نسخه‌ي اول را کاربر رد کرد: «خيلي گرد و قلمبه‌اند و
//      اصلا شبيه دست نيستن — همان اشتباهي که بار اول براي
//      قلبِ انگشتي و دستِ خميازه کردي.»
//
//   سه اشتباهِ مشخص داشتم و هر سه يک ريشه دارند — **دستم
//   را از توده ساختم ، نه از استخوان**:
//
//     ۱. کفِ دست را دايره کشيدم. کفِ دست چهارگوش است.
//        دايره «پنجه‌ي عروسک» مي‌دهد ، نه دست.
//
//     ۲. انگشت‌ها بيضيِ چاق بودند و به هم چسبيده. انگشت
//        باريک است: پهنايش حدود يک‌نهمِ پهنايِ کف.
//
//     ۳. درزِ انگشت‌ها را با خطِ ضخامتِ ۱ کشيدم. روي
//        صفحه‌ي ۳۶۰ اين يک خطِ مويي است که ديده نمي‌شود.
//        شکاف بايد **فضاي واقعي** باشد ، نه خطِ نقاشي‌شده.
//
//   قاعده‌ي تازه — همه چيز از r گرفته مي‌شود:
//
//        کفِ دست     پهنا 1.44r · بلندي 1.16r · گوشه 0.30r
//        انگشت       شعاع 0.11r · بلندي 0.55r تا 0.78r
//        فاصله‌ي مرکز 0.40r    →  شکافِ سياهِ 0.18r
//        شست        شعاع 0.14r · بلندي 0.60r
//
//   شکافِ 0.18r يعني در r=30 پنج پيکسلِ سياه. ديده مي‌شود.
// =====================================================

// ---------- هندسه‌ي پايه ----------
//
//   واحدِ همه چيز r است و r = **نيمِ پهنايِ کفِ دست**.
//   نسبت‌ها از دستِ واقعي گرفته شده‌اند ، نه از حدس:
//
//      کفِ دست      پهنا 2.00r · بلندي 1.76r · گوشه 0.56r
//      انگشت        قطر 0.36r · بلندي 0.84r تا 1.20r
//      فاصله‌ي مرکز  0.50r     →  شکافِ سياهِ 0.14r
//      شست         قطر 0.48r · بلندي 1.00r
//      مچ          قطر 0.70r
//
//   ☠️ در هر تابع ang يعني **جهتي که انگشت‌ها به آن اشاره
//      مي‌کنند** ، نه زاويه‌ي مستطيل. بارِ اول اين دو را قاطي
//      کردم: در ژستِ «خجالت» زاويه‌ي ۹۰ درجه دادم ، کف
//      عمودي شد و شکاف‌ها افقي — و دست «دمبل» از آب درآمد.
//      يک قرارداد ، در همه‌ي توابع.

// کفِ دست — چهارگوشِ گِردگوشه
//   ☠️ چرا دست‌ها «تخت» بودند:
//
//      هر تکه‌ي دست را با **يک رنگ** و بدونِ مرز کشيده بودم.
//      وقتي کف و چهار انگشت و شست همه C_HAND باشند و به هم
//      بچسبند ، سيلوئتشان يکي مي‌شود — چشم مرز نمي‌بيند ، پس
//      «انگشت» نمي‌بيند.
//
//      راهِ حلِ استاندارد: هر تکه **دو بار** کشيده مي‌شود — اول
//      کمي بزرگ‌تر با رنگِ خط ، بعد اندازه‌ي واقعي با رنگِ پوست.
//      چون تکه‌ها پشتِ سرِ هم کشيده مي‌شوند ، حلقه‌ي تيره‌ي هر
//      تکه داخلِ تکه‌ي قبلي مي‌بُرد و **خودش** خطِ جداکننده
//      مي‌سازد. بند انگشت رايگان درمي‌آيد.
//
//      ⚠️ تلهِ مهم: خطِ **مشکي** روي پس‌زمينه‌ي مشکي ديده نمي‌شود.
//         رنگِ خط بايد بنفشِ تيره باشد تا هم روي سياه و هم روي
//         پوستِ روشن مرز بسازد.
const uint16_t C_HAND_LN = rgb(84, 78, 126);   // خطِ دورِ دست
const int      HAND_LN   = 3;                  // ضخامتِ خط

inline void handPalm(int x, int y, int r, float ang, uint16_t c = C_HAND) {
  fbRRectRot(x, y, r + HAND_LN * 2, (r * 88) / 100 + HAND_LN * 2,
             (r * 56) / 100 + HAND_LN, ang + 1.5708f, C_HAND_LN);
  fbRRectRot(x, y, r, (r * 88) / 100, (r * 56) / 100, ang + 1.5708f, c);
}

// يک انگشت — کپسول از ته تا نوک
inline void handFinger(int x, int y, int len, int w, float ang,
                       uint16_t c = C_HAND) {
  if (w < 3) w = 3;
  int ex = x + (int)(len * cosf(ang)), ey = y + (int)(len * sinf(ang));
  fbLine(x, y, ex, ey, w + HAND_LN, C_HAND_LN);
  fbLine(x, y, ex, ey, w,           c);
}

// مچ — نمي‌گذارد دست در هوا شناور بماند
//   ☠️ بارِ اول مچ را از ۰٫۸r **پشتِ** مرکز شروع کردم ، ولي
//      کفِ دست فقط تا ۰٫۴۴r عقب مي‌رود. يعني يک نوارِ خاليِ
//      سياه لايِ کف و مچ مي‌افتاد و دست «قطع‌شده» ديده مي‌شد.
//      حالا از ۰٫۲۰r شروع مي‌شود — يعني **زيرِ** کف.
inline void handWrist(int x, int y, int r, float ang, uint16_t c = C_HAND) {
  int bx = x - (int)((r * 0.20f) * cosf(ang));
  int by = y - (int)((r * 0.20f) * sinf(ang));
  handFinger(bx, by, (r * 78) / 100, (r * 36) / 100, ang + 3.1416f, c);
}

// چهار انگشت روي لبه‌ي کف
inline void handFingers4(int x, int y, int r, float ang, float spread,
                         uint16_t c = C_HAND) {
  const int LEN[4] = { 104, 120, 110, 84 };   // درصدِ r — وسطي بلندتر
  int w    = (r * 18) / 100;
  int step = (r * 50) / 100;
  float ux = cosf(ang + 1.5708f), uy = sinf(ang + 1.5708f);
  // ته‌ي انگشت کمي **داخلِ** کف مي‌نشيند تا درز نيفتد
  int bx = x + (int)((r * 0.62f) * cosf(ang));
  int by = y + (int)((r * 0.62f) * sinf(ang));
  for (int k = 0; k < 4; k++) {
    float off = (k - 1.5f) * step;
    handFinger(bx + (int)(off * ux), by + (int)(off * uy),
               (r * LEN[k]) / 100, w, ang + (k - 1.5f) * spread, c);
  }
}

// شست — کلفت‌تر و کوتاه‌تر
inline void handThumb(int x, int y, int r, float ang, uint16_t c = C_HAND) {
  handFinger(x, y, r, (r * 24) / 100, ang, c);
}

// مشتِ بسته
//
//   ☠️ بارِ اول مشت را يک توده کشيدم و دو نوارِ سياه رويش
//      انداختم. نتيجه «آبنباتِ کاغذپيچ» شد ، نه مشت — در
//      ژستِ خجالت کاربر دو تا دمبل ديد.
//
//      اشتباه همان اشتباهِ هميشگي بود: شکل را با **خط**
//      توضيح دادم به‌جاي اينکه با **حجم** بسازم. مشت هم
//      انگشت دارد ، فقط انگشت‌هايش خوابيده‌اند. پس سه ميله‌ي
//      جدا با شکافِ سياهِ واقعي مي‌کشم ، درست مثل دستِ باز.
//
//   ang = جهتي که انگشت‌هاي خوابيده به آن دراز شده‌اند.
inline void handFist(int x, int y, int r, float ang, uint16_t c = C_HAND) {
  //   ☠️ ايرادِ سومِ مشت ، که فقط با رندرِ بيرون از برد ديدمش:
  //      پاشنه را ۷۸×۴۶ با شعاعِ گوشه‌ي ۴۰ کشيده بودم. وقتي
  //      شعاعِ گوشه به نصفِ ضلعِ کوتاه برسد ، مستطيل **قرص**
  //      مي‌شود. مشت هيچ گوشه‌اي نداشت ، پس «دستکشِ فِر» شد.
  //
  //      مشت بايد **مربعِ گوشه‌گرد** باشد. گوشه داشتنش همان
  //      چيزي است که به چشم مي‌گويد «اين يک مشت است».
  float ux = cosf(ang),           uy = sinf(ang);
  float vx = cosf(ang + 1.5708f), vy = sinf(ang + 1.5708f);

  // تنه‌ي مشت — تقريبا مربع ، با گوشه‌ي ملايم (۲۸٪ نه ۵۰٪)
  fbRRectRot(x, y, (r * 96) / 100 + HAND_LN * 2, (r * 84) / 100 + HAND_LN * 2,
             (r * 26) / 100 + HAND_LN, ang + 1.5708f, C_HAND_LN);
  fbRRectRot(x, y, (r * 96) / 100, (r * 84) / 100, (r * 26) / 100,
             ang + 1.5708f, c);

  // سه انگشتِ خوابيده روي لبه‌ي جلو ، با شکافِ **واقعيِ** سياه
  //   و ايرادِ چهارم: انگشت‌هاي خوابيده را کوتاه کشيدم
  //   (۰٫۴۲r) و پهن (۰٫۲۲r) ، پس تقريبا مربع شدند و به چشم
  //   «سه دکمه» آمدند نه «سه انگشت». انگشت بايد **دراز**
  //   باشد ، وگرنه انگشت نيست.
  int w    = (r * 17) / 100;   if (w < 4) w = 4;
  int step = (r * 26) / 100;
  for (int k = -1; k <= 1; k++) {
    float off = k * step;
    int ax = x + (int)(off * vx - r * 0.34f * ux);
    int ay = y + (int)(off * vy - r * 0.34f * uy);
    int bx = x + (int)(off * vx + r * 0.42f * ux);
    int by = y + (int)(off * vy + r * 0.42f * uy);
    fbLine(ax, ay, bx, by, w + HAND_LN, C_HAND_LN);
    fbLine(ax, ay, bx, by, w,           c);
  }
}

// ---------- ۱ · دستِ باز (سلام ، تکان دادن) ----------
inline void handWave(int x, int y, float t, int sd, int r = 36) {
  float wob = 0.24f * sinf(t * 6.0f);
  float up  = -1.5708f + wob;
  handWrist(x, y, r, up);
  handPalm(x, y, r, up);
  handFingers4(x, y, r, up, 0.17f);
  handThumb(x - (int)(sd * r * 0.88f), y + (r * 18) / 100, r,
            up + sd * 1.25f);
}

// ---------- ۲ · شستِ بالا (تاييد) ----------
inline void handThumbUp(int x, int y, int r = 36) {
  //   مشت را از پهلو مي‌بينيم ، پس انگشت‌هاي خوابيده افقي‌اند.
  //   شست بايد **بلند و جدا** باشد — کوتاهش کني ، مشت مي‌ماند.
  handWrist(x, y + (r * 46) / 100, r, -1.5708f);
  handFist(x, y + (r * 10) / 100, r, 3.1416f);
  //   ☠️ و ايرادِ آخر ، که تا رندر نديدمش: شست هم‌رنگِ مشت بود
  //      و چسبيده به آن — پس سيلوئتشان **يکي** شد و کلِ ژست
  //      به يک مستطيلِ بلند تبديل شد. شست بدونِ شکاف ديده
  //      نمي‌شود.
  //   ☠️ «آفرين» و «اشاره» در رندر **عينِ هم** درآمدند ، چون هر
  //      دو مشت + يک ميله‌ي عموديِ بلند بودند. فرقشان در واقع
  //      اين است:
  //        شست  = کوتاه ، کلفت ، از **پهلوي** مشت
  //        اشاره = بلند ، باريک ، از **بالاي** مشت
  //      پس شست را کوتاه و کلفت کردم و به لبه‌ي چپ بردم.
  int tx = x - (r * 46) / 100, ty = y - (r * 18) / 100;
  int te = ty - (r * 56) / 100;
  int tw = (r * 34) / 100;
  fbLine(tx, ty, tx, te, tw + HAND_LN, C_HAND_LN);
  fbCircle(tx, te, tw / 2 + HAND_LN, C_HAND_LN);
  fbLine(tx, ty, tx, te, tw, C_HAND);
  fbCircle(tx, te, tw / 2, C_HAND);
}

// ---------- ۳ · پيروزي ✌ ----------
inline void handVictory(int x, int y, int r = 36) {
  //   دو انگشت بايد **بلند** باشند (۱٫۵ برابرِ شعاع) و ميانشان
  //   شکافِ سياهِ واقعي بيفتد ، وگرنه يک خرگوشِ دوگوشِ کوتاه
  //   مي‌شود — که دقيقا همان چيزي بود که در رندر ديدم.
  int fy = y + (r * 24) / 100;
  int w  = (r * 24) / 100;
  for (int k = -1; k <= 1; k += 2) {
    float a = -1.5708f + k * 0.34f;
    int bx = x + k * (r * 16) / 100, by = fy - (r * 20) / 100;
    int ex = bx + (int)((r * 1.55f) * cosf(a));
    int ey = by + (int)((r * 1.55f) * sinf(a));
    fbLine(bx, by, ex, ey, w + HAND_LN, C_HAND_LN);
    fbCircle(ex, ey, w / 2 + HAND_LN, C_HAND_LN);
    fbLine(bx, by, ex, ey, w,          C_HAND);
    fbCircle(ex, ey, w / 2, C_HAND);
  }
  handWrist(x, fy + (r * 58) / 100, r, -1.5708f);
  handFist(x, fy + (r * 18) / 100, r, 3.1416f);
}

// ---------- ۴ · اشاره با يک انگشت ----------
inline void handPoint(int x, int y, float ang, int r = 34) {
  // انگشت‌هاي خوابيده **عمود** بر انگشتِ اشاره‌اند ، نه هم‌جهتش.
  handWrist(x, y, r, ang);
  handFist(x, y, r, ang - 1.5708f);
  handFinger(x + (int)((r * 0.40f) * cosf(ang)),
             y + (int)((r * 0.40f) * sinf(ang)),
             (r * 175) / 100, (r * 16) / 100, ang);   // بلند و باريک
  handFinger(x + (int)((r * 0.55f) * cosf(ang - 1.5f)),
             y + (int)((r * 0.55f) * sinf(ang - 1.5f)),
             (r * 50) / 100, (r * 25) / 100, ang - 1.1f);
}

// ---------- ۵ · دستِ روي دهان (خنده‌ي پنهان) ----------
inline void handOverMouth(int ox, int oy, float t) {
  // ☠️ اول دو دست کشيدم و نتيجه «دو دستِ باز» شد ، نه
  //    «دست جلوي دهان». يک دست کافي است ، و انگشت‌ها بايد
  //    **چسبيده** باشند — دستِ باز پنهان نمي‌کند.
  //   و کمي به راست جابه‌جا شد: دستي که دقيقا وسط بنشيند
  //   دهان را کامل مي‌بلعد و ژست خوانده نمي‌شود. بايد گوشه‌ي
  //   دهان پيدا بماند.
  int y  = CY + oy + MOUTH_Y - 26 + (int)(2.0f * sinf(t * 4.0f));
  int hx = CX + ox + 24;
  int r  = 38;
  float up = -1.5708f - 0.34f;
  handWrist(hx, y, r, up);
  handPalm(hx, y, r, up);
  handFingers4(hx, y, r, up, 0.02f);
  handThumb(hx + (r * 82) / 100, y + (r * 22) / 100, r, up - 1.30f);
}

// ---------- ۶ · پوشاندنِ چشم‌ها ----------
inline void handsOverEyes(int ox, int oy, float t) {
  int peek = (int)(4.0f * sinf(t * 1.6f));
  int r = 42;
  for (int sd = -1; sd <= 1; sd += 2) {
    int hx = CX + sd * EYE_DX + ox;
    int hy = CY + oy + peek + 26;
    handWrist(hx, hy, r, -1.5708f);
    handPalm(hx, hy, r, -1.5708f);
    // شکافِ باز — سياهيِ پس‌زمينه از لايِ انگشت‌ها پيداست
    handFingers4(hx, hy, r, -1.5708f, 0.12f);
    handThumb(hx - sd * (r * 88) / 100, hy + (r * 18) / 100, r,
              -1.5708f + sd * 1.3f);
  }
}

// ---------- ۷ · دو دستِ باز (بغل ، «چي؟») ----------
inline void handsOpen(int ox, int oy, float t) {
  for (int sd = -1; sd <= 1; sd += 2)
    handWave(CX + ox + sd * 116, CY + oy + 34, t + sd, sd, 30);
}

// ---------- ۸ · تشويق ----------
inline void handsClap(int ox, int oy, float t) {
  int gap = 18 + (int)(14.0f * fabsf(sinf(t * 7.0f)));
  int r = 28, cy = CY + oy + 86;
  for (int sd = -1; sd <= 1; sd += 2) {
    int hx = CX + ox + sd * (gap + r);
    // ☠️ افقيِ محض «دو شانه‌ي مو» مي‌دهد. کفِ دست موقعِ تشويق
    //    مايل است — انگشت‌ها رو به بالا-داخل.
    float in = -1.5708f + sd * 1.05f;
    handWrist(hx, cy, r, in);
    handPalm(hx, cy, r, in);
    handFingers4(hx, cy, r, in, 0.06f);
    handThumb(hx - (int)(sd * r * 0.30f), cy - (r * 80) / 100, r,
              in - sd * 1.25f);
  }
  if (gap < 24)
    for (int k = 0; k < 3; k++) {
      float a = -0.75f + k * 0.75f;
      fbLine(CX + ox + (int)(56 * cosf(a)), cy + (int)(56 * sinf(a)) - 40,
             CX + ox + (int)(74 * cosf(a)), cy + (int)(74 * sinf(a)) - 54,
             3, C_STAR);
    }
}

// ---------- ۹ · قلب با دو دست ----------
//   قلب را **انگشت‌ها** مي‌سازند: هر دست کج شده ، اشاره‌اش
//   روي لبه‌ي بالايِ قلب و شستش زيرِ آن. دست‌ها قلب را
//   لمس مي‌کنند ، نه اينکه کنارش شناور باشند.
inline void handsHeart(int ox, int oy, float t) {
  //   بالاتر رفت: دست‌ها در جاي قبلي روي کاسه‌ي چشم مي‌نشستند.
  int cy = CY + oy - 114;
  int pulse = (int)(2.5f * sinf(t * 4.0f));
  int r = 26;
  //   ☠️ دو دست موازي و رو به بالا بودند ، پس «دو پارو» ديده
  //      مي‌شدند نه قابِ قلب. نوکِ انگشت‌ها بايد به **مرکزِ قلب**
  //      اشاره کند. زاويه را از هندسه گرفتم ، نه از حدس:
  //          atan2(قلب − دست)  ⇒  ‎−۱٫۵۷ − sd·۰٫۷۴
  for (int sd = -1; sd <= 1; sd += 2) {
    int hx = CX + ox + sd * 50;
    int hy = cy + 54;
    float up = -1.5708f - sd * 0.74f;
    handWrist(hx, hy, r, up);
    handPalm(hx, hy, r, up);
    handFingers4(hx, hy, r, up, 0.06f);
    handThumb(hx + (int)(sd * r * 0.55f), hy + (r * 45) / 100, r,
              up + sd * 1.30f);
  }
  fbHeart(CX + ox, cy + 10, 58 + pulse, C_HOT);
}

// ---------- ۱۰ · سلامِ نظامي ----------
//   ☠️ نسخه‌ي قبل «هفت‌تير» شد. دو علت داشت و هر دو را ديدم
//      نه حدس زدم:
//        ۱ ساعد يک مستطيلِ **جدا** بود ، ۳۰ پيکسل پايين‌تر و
//          با زاويه‌ي ديگر — پس لوله‌اي شد که از قنداق مي‌زند
//          بيرون.
//        ۲ شست از گوشه‌ي پايين بيرون زده بود = ماشه.
//      حالا ساعد از **زيرِ** کف مي‌آيد و شست روي کف مي‌خوابد.
inline void handSalute(int ox, int oy) {
  //   ☠️ دو نسخه‌ي قبل هر دو «دستکشِ بسته» شدند ، چون انگشت‌ها
  //      را با فاصله‌ي ۰٫۰۳ راديان کنارِ هم گذاشته بودم و
  //      هم‌رنگ بودند — يعني يک تخته.
  //
  //      درس: به‌جاي ساختنِ يک هندسه‌ي تازه ، همان ساختِ
  //      «سلام» را برداشتم که در رندر **ثابت شد خوانا است** و
  //      فقط چرخاندمش. شکلي که يک بار جواب داده ، دوباره هم
  //      جواب مي‌دهد.
  //   جا هم مهم بود: نسخه‌ي قبل وسطِ کاسه‌ي چشم نشست. نوکِ
  //   انگشت بايد به **سرِ ابرو** برسد و کفِ دست بيرونِ صورت
  //   بماند — احترام کنارِ شقيقه است ، نه روي چشم.
  int r = 36;
  int hx = CX + ox + 116, hy = CY + oy - 60;
  float ang = 3.1416f + 0.38f;               // انگشت‌ها رو به پيشاني
  // ساعد — از پاشنه به پايين-راست ، چسبيده
  fbLine(hx + (int)(r * 0.30f), hy + (int)(r * 0.30f),
         hx + (int)(r * 0.85f), hy + (int)(r * 1.55f),
         (r * 42) / 100, C_HAND);
  handWrist(hx, hy, r, ang);
  handPalm (hx, hy, r, ang);
  handFingers4(hx, hy, r, ang, 0.13f);       // همان فاصله‌ي «سلام»
  handThumb(hx + (int)(r * 0.20f), hy + (int)(r * 0.80f), r, ang - 1.15f);
}

// ---------- ۱۱ · شانه بالا انداختن ----------
inline void handsShrug(int ox, int oy) {
  int r = 32;
  for (int sd = -1; sd <= 1; sd += 2) {
    int hx = CX + ox + sd * 124, hy = CY + oy + 52;
    // کف رو به **بالا**-بيرون است ، نه افقي: «نمي‌دانم» را
    // کفِ باز مي‌گويد ، نه پشتِ دست.
    float out = (sd > 0) ? -0.62f : 3.1416f + 0.62f;
    handWrist(hx, hy, r, out);
    handPalm(hx, hy, r, out);
    handFingers4(hx, hy, r, out, 0.22f);
    handThumb(hx + (int)((r * 0.85f) * cosf(out + sd * 1.5f)),
              hy + (int)((r * 0.85f) * sinf(out + sd * 1.5f)), r,
              out + sd * 1.25f);
  }
}

// ---------- ۱۲ · نوکِ انگشت‌ها به هم (خجالت) ----------
//   ☠️ نسخه‌ي قبل دو «چنگالِ خوابيده» شد ، چون دست‌ها را افقي
//      گذاشته بودم. حالتِ خجالت اين نيست. در همه‌ي ورق‌هاي
//      مرجع دست‌ها **ايستاده**اند و نوکِ انگشت‌ها بالا و رو به
//      هم — مثل کسي که با انگشت‌هايش بازي مي‌کند.
inline void handsShy(int ox, int oy, float t) {
  int gap = 10 + (int)(5.0f * sinf(t * 2.2f));
  int hy = CY + oy + 72, r = 30;
  for (int sd = -1; sd <= 1; sd += 2) {
    int hx = CX + ox + sd * (gap + r);
    float up = -1.5708f - sd * 0.42f;        // ايستاده ، کمي رو به هم
    handWrist(hx, hy + (r * 55) / 100, r, up);
    handPalm(hx, hy, r, up);
    handFingers4(hx, hy, r, up, 0.05f);      // چسبيده
    handThumb(hx + (int)(sd * r * 0.62f), hy + (r * 30) / 100, r,
              up + sd * 1.15f);
  }
}


// =====================================================
//   چشم‌ها و دهان‌هاي تازه — دورِ دوم
// =====================================================

// ---------- چشمِ نقطه‌اي (شوک) ----------
// سفيديِ کامل ، مردمکِ خيلي ريزِ وسط. آن حالتِ «خشکم زد».
inline void eyeDot(int sd, int ox, int oy, int r = 50) {
  int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
  fbCircle(ex, ey, r, C_WHITE);
  fbCircle(ex, ey, 7, C_PUPIL);
}

// ---------- چشمِ گردابي (هيپنوتيزم) ----------
// فرقش با مارپيچِ سرگيجه: اين **حلقه‌هاي تودرتو**ي ثابت است
// نه مارپيچِ چرخان.
inline void eyeSwirl(int sd, int ox, int oy, float t, int r = 50) {
  int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
  fbCircle(ex, ey, r, C_WHITE);
  int rings = 4;
  for (int k = rings; k >= 1; k--) {
    int rr = (r * k) / (rings + 1);
    fbCircle(ex, ey, rr, (k % 2) ? C_PUPIL : C_WHITE);
  }
  (void)t;
}

// ---------- چشمِ بسته‌ي مورب (خنده‌ي محکم) ----------
// دو خطِ مورب که به هم مي‌رسند — فرقش با ES_CARET اين است
// که زاويه بازتر و خط بلندتر است.
inline void eyeSquint(int sd, int ox, int oy, uint16_t c = C_WHITE) {
  int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
  brushStroke(ex - sd * 14, ey - 10, 20, 12, 0,  sd * 0.42f, c);
  brushStroke(ex - sd * 14, ey + 10, 20, 12, 0, -sd * 0.42f, c);
}

// ---------- چشمِ ضربدر (از حال رفته) ----------
inline void eyeCross(int sd, int ox, int oy, uint16_t c = C_WHITE) {
  int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
  fbLine(ex - 26, ey - 26, ex + 26, ey + 26, 7, c);
  fbLine(ex + 26, ey - 26, ex - 26, ey + 26, 7, c);
}

// ---------- چشمِ چپ (دو طرف نگاه مي‌کنند) ----------
inline void eyeCross2(int sd, int ox, int oy, int open, int r = 50) {
  // مردمک‌ها به سمت **داخل** — نگاهِ چپ
  eyeRound(sd, ox, oy, -sd * 22, 4, open, r, 2);
}

// ---------- چشمِ نيمه‌بسته با مردمکِ بالا (چشم‌غره‌ي واقعي) ----------
// در ورق‌ها چشم‌غره چشمِ بسته نيست — مردمک بالا مي‌رود و
// سفيديِ **پايينِ** چشم ديده مي‌شود.
inline void eyeRollUp(int sd, int ox, int oy, int r = 50) {
  int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
  fbCircle(ex, ey, r, C_WHITE);
  // مردمک بالا و نيمه‌بيرون از کادر
  fbEllipse(ex, ey - (r * 55) / 100, (r * 55) / 100, (r * 40) / 100, C_PUPIL);
  // پلکِ بالا که رويش مي‌افتد
  for (int y = ey - r - 4; y < ey - (r * 45) / 100; y++)
    for (int x = ex - r - 4; x <= ex + r + 4; x++) px(x, y, swap16(C_BG));
  // و خطِ پلک
  brushStroke(ex, ey - (r * 45) / 100, r - 4, 9, 4.0f, 0, C_WHITE);
}

// =====================================================
//   دهان‌هاي تازه
// =====================================================

// ---------- دهانِ موجيِ نگراني ----------
// «〰» کوچکِ فيروزه‌اي — در ورق‌ها خيلي تکرار شده
inline void mouthSquiggle(int ox, int oy, int halfW = 18) {
  int y = CY + MOUTH_Y + oy - 16;
  for (int i = -halfW; i <= halfW; i++) {
    float u = (float)i / halfW;
    int yy = y + (int)(5.0f * sinf(u * 6.3f));
    fbCircle(CX + ox + i, yy, 4, C_TEAL);
  }
}

// ---------- دهانِ زيگزاگِ ترس ----------
inline void mouthZigzag(int ox, int oy, int halfW = 20) {
  int y = CY + MOUTH_Y + oy - 16;
  int n = 6;
  for (int k = 0; k < n; k++) {
    int x1 = CX + ox - halfW + (halfW * 2 * k) / n;
    int x2 = CX + ox - halfW + (halfW * 2 * (k + 1)) / n;
    int y1 = y + ((k % 2) ? 8 : -8);
    int y2 = y + ((k % 2) ? -8 : 8);
    fbLine(x1, y1, x2, y2, 4, C_TEAL);
  }
}

// ---------- دهانِ بيضيِ عمودي (شوک) ----------
inline void mouthOval(int ox, int oy, int rx = 9, int ry = 18) {
  fbEllipse(CX + ox, CY + MOUTH_Y + oy - 14, rx, ry, C_TEAL);
}

// ---------- زبانِ بلندِ بيرون‌زده ----------
// فرقش با mouthTongue: اين از **زير** دهانِ بسته بيرون مي‌زند
inline void mouthTongueOut(int ox, int oy, float t) {
  int y = CY + MOUTH_Y + oy - 18;
  int len = 40 + (int)(5.0f * sinf(t * 5.0f));
  fbCurve(CX + ox, y - 6, 16, 7, 5, C_TEAL);           // لبخندِ بالا
  fbRoundRect(CX + ox - 11, y, 22, len, 11, C_HOT);    // زبان
}

// ---------- دهانِ کج (شک) ----------
inline void mouthCrooked(int ox, int oy, int sd = 1) {
  //   ☠️ دو بار اشتباه شد و هر دو بار در رندر ديدمش:
  //        ۱ اول شعاعِ ۴ و عرضِ ۳۷ — «خطِ شناورِ جدا» نه دهان
  //        ۲ بعد ضخيمش کردم ولي **صاف** ماند ، و شد يک خطِ
  //          موربِ ۴۵ درجه که شبيه خط‌خوردگي بود نه لبخند
  //
  //      دهانِ کج در واقع يک **قوس** است که يک گوشه‌اش بالاتر
  //      نشسته. پس سهمي + کجيِ ملايم ، نه شيبِ خالص.
  int y = CY + MOUTH_Y + oy - 14;
  for (int i = -26; i <= 26; i++) {
    float u    = (float)i / 26.0f;
    float arc  = -9.0f * (1.0f - u * u);     // قوسِ لبخند
    float lean =  sd * u * 6.0f;             // گوشه‌ي يک‌طرف بالاتر
    fbCircle(CX + ox + i, y + (int)lroundf(arc + lean), 6, C_TEAL);
  }
}

// ---------- لبخندِ بازِ بزرگ با زبان ----------
inline void mouthBigSmile(int ox, int oy) {
  int cx = CX + ox, cy = CY + MOUTH_Y + oy - 18;
  // نيم‌دايره‌ي رو به پايين
  for (int y = cy; y <= cy + 22; y++) {
    int dy = y - cy;
    int w = (int)(26 * sqrtf(1.0f - (float)(dy * dy) / (22 * 22)));
    for (int x = cx - w; x <= cx + w; x++) px(x, y, swap16(C_TEAL));
  }
  fbRect(cx - 26, cy - 3, 52, 5, C_TEAL);
}

// =====================================================
//   اضافات — دورِ دوم
// =====================================================

// ---------- باتري ----------
//   level  ۰ تا ۱۰۰ درصد
inline void xBattery(int ox, int oy, int level, float t) {
  int bx = CX + ox + 102, by = CY + oy + 82;
  uint16_t c = (level < 25) ? C_FL_OUT : rgb(60, 220, 90);
  //   ☠️ فازِ خاموشِ چشمک را C_DIM گذاشته بودم که روي سياه
  //      تقريبا نامرئي است — کاربر نيمي از وقت فقط يک قابِ
  //      خالي مي‌ديد. چشمک بايد **کم‌نور** شود ، نه ناپديد.
  if (level < 25 && sinf(t * 6.0f) < 0) c = rgb(150, 30, 30);
  fbRoundRect(bx - 26, by - 14, 52, 28, 5, C_WHITE);
  fbRoundRect(bx - 23, by - 11, 46, 22, 3, C_BG);
  fbRect(bx + 26, by - 6, 5, 12, C_WHITE);           // سرِ مثبت
  //   ☠️ در ۸٪ عرضِ پرشدگي ۳ پيکسل مي‌شد — يعني در عمل نامرئي
  //      و کاربر فقط يک «قابِ خالي» مي‌ديد. کف گذاشتم تا هميشه
  //      چيزي ديده شود ، و رنگِ هشدار را پررنگ کردم.
  int w = (44 * level) / 100;
  if (w < 7) w = 7;
  fbRoundRect(bx - 22, by - 10, w, 20, 2, c);
}

// ---------- صاعقه‌ي شارژ ----------
inline void xCharging(int ox, int oy, float t) {
  //   صاعقه بايد **روي** باتري بيفتد ، نه کنارش. کنارش بود و
  //   دو شيءِ بي‌ربط ديده مي‌شدند.
  xBattery(ox, oy, 60 + (int)(35.0f * fabsf(sinf(t * 1.6f))), t);
  int lx = CX + ox + 102, ly = CY + oy + 82;
  fbLine(lx + 6, ly - 16, lx - 4, ly + 2,  5, C_STAR);
  fbLine(lx - 4, ly + 2,  lx + 4, ly + 2,  5, C_STAR);
  fbLine(lx + 4, ly + 2,  lx - 6, ly + 18, 5, C_STAR);
}

// ---------- ذره‌بين ----------
inline void xMagnifier(int ox, int oy, float t) {
  int mx = CX + ox + 74 + (int)(4.0f * sinf(t * 2.0f));
  int my = CY + oy + 74;
  fbCircle(mx, my, 31, C_TEAL);
  fbCircle(mx, my, 23, C_BG);
  fbLine(mx + 21, my + 21, mx + 40, my + 40, 11, C_STAR);
}

// ---------- ساعتِ شني ----------
inline void xHourglass(int ox, int oy, float t) {
  int hx = CX + ox + 100, hy = CY + oy + 84;
  fbRect(hx - 18, hy - 32, 36, 5, C_STAR);
  fbRect(hx - 18, hy + 27, 36, 5, C_STAR);
  // دو مثلثِ سرِ هم
  for (int i = 0; i <= 26; i++) {
    int w = 16 - (int)(16.0f * i / 26.0f);
    for (int x = hx - w; x <= hx + w; x++) {
      px(x, hy - 27 + i, swap16(C_WHITE));
      px(x, hy + 27 - i, swap16(C_WHITE));
    }
  }
  // شنِ در حال ريختن
  float f = fmodf(t * 0.5f, 1.0f);
  int top = (int)((1.0f - f) * 14);
  for (int i = 0; i < top; i++) {
    int w = 14 - i;
    for (int x = hx - w; x <= hx + w; x++) px(x, hy - 24 + i, swap16(C_STAR));
  }
  fbLine(hx, hy - 2, hx, hy + 12, 2, C_STAR);
}

// ---------- دانه‌ي برف ----------
inline void xSnow(int ox, int oy, float t) {
  int sx = CX + ox + 40, sy = CY + oy - 143;
  float rot = t * 0.8f;
  for (int k = 0; k < 6; k++) {
    float a = rot + k * 1.047f;
    int x2 = sx + (int)(24 * cosf(a)), y2 = sy + (int)(24 * sinf(a));
    fbLine(sx, sy, x2, y2, 3, C_TEAR);
    // شاخه‌هاي کوچک
    float b1 = a + 0.5f, b2 = a - 0.5f;
    int mx = sx + (int)(15 * cosf(a)), my = sy + (int)(15 * sinf(a));
    fbLine(mx, my, mx + (int)(8 * cosf(b1)), my + (int)(8 * sinf(b1)), 2, C_TEAR);
    fbLine(mx, my, mx + (int)(8 * cosf(b2)), my + (int)(8 * sinf(b2)), 2, C_TEAR);
  }
}

// ---------- خورشيد ----------
inline void xSun(int ox, int oy, float t) {
  int sx = CX + ox + 42, sy = CY + oy - 142;
  fbCircle(sx, sy, 17, C_STAR);
  for (int k = 0; k < 8; k++) {
    float a = t * 0.6f + k * 0.785f;
    fbLine(sx + (int)(22 * cosf(a)), sy + (int)(22 * sinf(a)),
           sx + (int)(32 * cosf(a)), sy + (int)(32 * sinf(a)), 4, C_STAR);
  }
}

// ---------- ماه ----------
inline void xMoon(int ox, int oy) {
  int mx = CX + ox + 44, my = CY + oy - 140;
  fbCircle(mx, my, 24, rgb(200, 190, 255));
  fbCircle(mx + 11, my - 6, 21, C_BG);      // بريدنِ هلال
}

// ---------- دماسنج ----------
inline void xThermo(int ox, int oy, float t) {
  int tx = CX + ox + 112, ty = CY + oy + 92;
  fbRoundRect(tx - 6, ty - 34, 12, 46, 6, C_WHITE);
  fbCircle(tx, ty + 18, 13, C_WHITE);
  int h = 18 + (int)(6.0f * sinf(t * 3.0f));
  fbRoundRect(tx - 3, ty + 8 - h, 6, h, 3, C_FL_OUT);
  fbCircle(tx, ty + 18, 9, C_FL_OUT);
}

// ---------- گل ----------
inline void xFlower(int ox, int oy, float t) {
  int fx = CX + ox + 102, fy = CY + oy + 98;
  float rot = t * 0.5f;
  for (int k = 0; k < 5; k++) {
    float a = rot + k * 1.257f;
    fbCircle(fx + (int)(15 * cosf(a)), fy + (int)(15 * sinf(a)), 11, C_WHITE);
  }
  fbCircle(fx, fy, 9, C_STAR);
}

// ---------- آبنبات ----------
inline void xCandy(int ox, int oy, float t) {
  //   ☠️ نسخه‌ي اول سه خطِ مورب روي يک مستطيل بود ، و در
  //      اندازه‌ي واقعي «خط‌خطيِ زرد» ديده مي‌شد نه آبنبات.
  //      آبنباتِ کاغذپيچ دو نشانه دارد و هر دو لازم‌اند:
  //        ۱ تنه‌ي **گِرد** (نه مستطيل)
  //        ۲ دو سرِ **مثلثيِ** پيچ‌خورده که از تنه باريک‌تر است
  int cx = CX + ox + 96, cy = CY + oy + 96;
  int wob = (int)(2.0f * sinf(t * 3.0f));
  cy += wob;
  // دو سرِ کاغذ — مثلث ، رو به بيرون
  for (int sd = -1; sd <= 1; sd += 2)
    for (int i = 0; i <= 17; i++) {
      int h = 2 + (i * 9) / 17;               // از نوک پهن مي‌شود
      int x = cx + sd * (34 - i);
      for (int y = cy - h; y <= cy + h; y++) px(x, y, swap16(C_HOT));
    }
  // تنه
  fbCircle(cx, cy, 19, C_STAR);
  // دو نوارِ مارپيچ روي تنه — منحني ، نه خطِ صاف
  for (int k = -1; k <= 1; k += 2)
    for (int i = -16; i <= 16; i++) {
      int y = cy + i;
      int d2 = 19 * 19 - i * i;  if (d2 < 0) continue;
      int hx = (int)sqrtf((float)d2);
      int x  = cx + k * 7 + (int)(5.0f * sinf(i * 0.13f));
      if (x > cx - hx && x < cx + hx)
        for (int w = -2; w <= 2; w++) px(x + w, y, swap16(C_HOT));
    }
}

// ---------- کلاهِ جادوگر ----------
inline void xWizardHat(int ox, int oy, float t) {
  int hx = CX + ox, hy = CY + oy - 96;
  int lean = (int)(3.0f * sinf(t * 1.4f));
  fbRoundRect(hx - 46, hy + 8, 92, 12, 6, rgb(120, 60, 190));
  for (int i = 0; i <= 62; i++) {
    float u = (float)i / 62;
    int w = (int)(34 * (1.0f - u));
    int cx2 = hx + (int)(u * u * lean * 3);
    for (int x = cx2 - w; x <= cx2 + w; x++)
      px(x, hy + 8 - i, swap16(rgb(120, 60, 190)));
  }
  for (int k = 0; k < 3; k++)
    fbSparkle(hx - 12 + k * 14, hy - 14 - k * 14, 6, C_STAR);
}

// ---------- کلاهِ جشن ----------
inline void xPartyHat(int ox, int oy) {
  int hx = CX + ox - 56, hy = CY + oy - 90;
  for (int i = 0; i <= 52; i++) {
    float u = (float)i / 52;
    int w = (int)(24 * (1.0f - u));
    int cx2 = hx + (int)(u * 12);
    uint16_t c = ((i / 9) % 2) ? C_STAR : C_TEAL;
    for (int x = cx2 - w; x <= cx2 + w; x++) px(x, hy - i, swap16(c));
  }
  fbCircle(hx + 12, hy - 56, 8, C_HOT);
}

// ---------- کاغذرنگي ----------
inline void xConfetti(int ox, int oy, float t) {
  const uint16_t C[4] = { C_HOT, C_TEAL, C_STAR, rgb(140, 120, 255) };
  for (int k = 0; k < 10; k++) {
    float ph = fmodf(t * 0.7f + k * 0.11f, 1.0f);
    int sgn = (k % 2) ? 1 : -1;
    int x = CX + ox + sgn * (48 + (k % 5) * 22);
    int y = CY + oy - 110 + (int)(ph * 220);
    int s = 7 - (k % 3);
    fbEllipseRot(x, y, s, s / 2 + 2, ph * 6.0f + k, C[k % 4]);
  }
}

// ---------- چشم‌بندِ دزد دريايي ----------
inline void xEyePatch(int ox, int oy) {
  // ☠️ اول با C_PUPIL کشيدمش — يعني **مشکيِ خالص** ، که
  //    همان C_BG است. روي صفحه چشم‌بند اصلا ديده نمي‌شد و
  //    فقط جمجمه‌ي سفيد در هوا شناور بود.
  //
  //    درس: هر چيزي که قرار است روي پس‌زمينه ديده شود نبايد
  //    هم‌رنگِ پس‌زمينه باشد. C_PUPIL فقط **درونِ** سفيديِ
  //    چشم معنا دارد.
  const uint16_t C_PATCH = rgb(44, 40, 66);      // زغاليِ بنفش‌گون
  const uint16_t C_STRAP = rgb(30, 27, 48);
  int ex = CX + ox + EYE_DX, ey = CY + oy;
  fbLine(ex - 40, ey - 34, CX + ox - EYE_DX - 46, ey - 54, 7, C_STRAP);
  fbLine(ex + 40, ey - 34, CX + ox + EYE_DX + 66, ey - 50, 7, C_STRAP);
  fbRoundRect(ex - 46, ey - 38, 92, 76, 16, C_PATCH);
  // جمجمه‌ي ريز
  fbCircle(ex, ey - 6, 15, C_WHITE);
  fbCircle(ex - 6, ey - 8, 4, C_PATCH);
  fbCircle(ex + 6, ey - 8, 4, C_PATCH);
  fbRect(ex - 9, ey + 8, 18, 7, C_WHITE);
}

// ---------- قلبِ شکسته ----------
inline void xBrokenHeart(int ox, int oy, float t) {
  int hx = CX + ox + 88, hy = CY + oy + 78;
  int split = 5 + (int)(4.0f * sinf(t * 1.6f));
  // نيمه‌ي چپ و راست با شکافِ زيگزاگ
  fbHeart(hx - split, hy, 58, C_HOT);
  fbHeart(hx + split, hy, 58, C_HOT);
  for (int k = 0; k < 6; k++) {
    int y1 = hy - 28 + k * 11;
    fbLine(hx + ((k % 2) ? -8 : 8), y1,
           hx + ((k % 2) ?  8 : -8), y1 + 11, 4, C_BG);
  }
}

// ---------- دندانِ نيش ----------
inline void xFangs(int ox, int oy) {
  //   ☠️ نيش را تنها کشيده بودم و در هوا شناور بود. نيش بدونِ
  //      دهان معني ندارد — بايد از **سقفِ يک دهانِ باز** آويزان
  //      باشد. پس خودِ دهان را هم همين‌جا مي‌کشم تا با هر
  //      نسخه‌اي کار کند.
  int cx = CX + ox, cy = CY + oy + MOUTH_Y - 6;
  fbRoundRect(cx - 38, cy - 19, 76, 38, 14, C_MOUTH);
  for (int sd = -1; sd <= 1; sd += 2) {
    int x = cx + sd * 21;
    for (int i = 0; i <= 24; i++) {
      int w = 9 - (9 * i) / 24;
      for (int xx = x - w; xx <= x + w; xx++)
        px(xx, cy - 19 + i, swap16(C_WHITE));
    }
  }
}

// ---------- چند قطره‌ي عرق ----------
inline void xSweatMulti(int ox, int oy, float t) {
  for (int k = 0; k < 3; k++) {
    float ph = fmodf(t * 0.9f + k * 0.33f, 1.0f);
    int sgn = (k % 2) ? 1 : -1;
    int sx = CX + ox + sgn * (EYE_DX + 44) + k * 4;
    int sy = CY + oy - 70 + (int)(ph * 52);
    int r = 8 - (int)(ph * 3);
    if (r > 3) fbDrop(sx, sy, r, r + 6, C_TEAR);
  }
}

// =====================================================
//   موتورِ ترکيبي
//
//   يک نسخه (FaceRecipe) مي‌گيرد و چهره را مي‌کشد.
//   ترتيبِ کشيدن مهم است و از تجربه‌ي امروز آمده:
//
//     پس‌زمينه → اضافاتِ پشت → چشم → ابرو → لپ → دهان
//               → اضافاتِ جلو
//
//   ☠️ چرا ابرو **بعد از** چشم؟ چون در حالت‌هايي که چشم
//      درشت است (شعاع ۵۶) ابرو نزديکش مي‌نشيند و اگر زير
//      چشم کشيده شود ، لبه‌اش گم مي‌شود.
//
//   ☠️ و چرا لپ بعد از ابرو؟ چون در طرحِ مرجع لپ **روي**
//      لبه‌ي پايينِ چشم مي‌افتد ، نه زيرش.
// =====================================================

// ---------- کشيدنِ چشمِ يک سمت بر اساسِ نوع ----------
inline void faceEye(int sd, int ox, int oy, EyeKind k,
                    int gx, int gy, int open, float t) {
  switch (k) {
    case EK_ROUND:   eyeRound(sd, ox, oy, gx, gy, open, 52, 2); break;
    case EK_ROUND2:  eyeRound(sd, ox, oy, gx, gy, open, 56, 3); break;   // ستاره‌اي
    case EK_HALF:    eyeRound(sd, ox, oy, gx, gy, (open * 52) / 100, 52, 2); break;
    case EK_ARC:     drawBrushEye(sd, ox, oy, ES_ARC_UP,   t); break;
    case EK_CARET:   drawBrushEye(sd, ox, oy, ES_CARET,    t); break;
    //   ☠️ مقياسِ ۸۸ يعني شعاعِ ۲۶ — نصفِ چشمِ گرد. در ده چهره
    //      اين «چشمِ قلبي» آن‌قدر ريز بود که صورت **بي‌چشم**
    //      ديده مي‌شد. فقط همين فراخواني را بزرگ کردم ، نه خودِ
    //      ES_HEART را — چون main.cpp با مقياسِ ۶۶ و ۸۲ براي
    //      انيميشنِ عشق صدايش مي‌زند و آن‌ها درست‌اند.
    case EK_HEART:   drawBrushEye(sd, ox, oy, ES_HEART,    t, C_HOT, 165); break;
    case EK_RING:    drawBrushEye(sd, ox, oy, ES_RING,     t); break;
    case EK_LINE:    drawBrushEye(sd, ox, oy, ES_LINE,     t); break;
    case EK_WOBBLE:  drawBrushEye(sd, ox, oy, ES_WOBBLE,   t); break;
    case EK_UP:      drawBrushEye(sd, ox, oy, ES_UP,       t); break;
    case EK_FLAME:   eyeFlame(sd, ox, oy, t); break;

    case EK_SPIRAL: {
      int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
      fbCircle(ex, ey, 50, C_WHITE);
      fbSpiral(ex, ey, 2.4f, 34, 4, t * (sd > 0 ? 7.0f : -7.0f), C_PUPIL);
      break;
    }
    case EK_SLEEP: {
      int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
      int h = 12;
      fbRoundRect(ex - 30, ey - h / 2, 60, h, h / 2, C_WHITE);
      break;
    }
    case EK_STAR: {
      // چشمِ ستاره‌اي — سفيدي گرد ، و به جاي مردمک يک ستاره
      int ex = CX + sd * EYE_DX + ox, ey = CY + oy;
      fbCircle(ex, ey, 52, C_WHITE);
      int r = 30 + (int)(2.5f * sinf(t * 5.0f));
      fbSparkle(ex, ey, r, C_STAR);
      break;
    }
    // چشمک: يک سمت بسته ، سمتِ ديگر گرد
    case EK_WINK_L:
      if (sd < 0) drawBrushEye(sd, ox, oy, ES_CARET, t);
      else        eyeRound(sd, ox, oy, gx, gy, open, 52, 2);
      break;
    case EK_WINK_R:
      if (sd > 0) drawBrushEye(sd, ox, oy, ES_CARET, t);
      else        eyeRound(sd, ox, oy, gx, gy, open, 52, 2);
      break;

    // — دورِ دوم —
    case EK_DOT:    eyeDot(sd, ox, oy, 50);          break;
    case EK_SWIRL:  eyeSwirl(sd, ox, oy, t, 50);     break;
    case EK_SQUINT: eyeSquint(sd, ox, oy);           break;
    case EK_CROSS:  eyeCross(sd, ox, oy);            break;
    case EK_CROSS2: eyeCross2(sd, ox, oy, open, 50); break;
    case EK_ROLLUP: eyeRollUp(sd, ox, oy, 50);       break;
  }
}

// ---------- دهان ----------
inline void faceMouth(int ox, int oy, MouthKind k, float t) {
  switch (k) {
    case MK_NONE:    break;
    case MK_TINY:    mouthTiny(ox, oy, 15, 8);  break;
    case MK_WIDE:    mouthTiny(ox, oy, 22, 11); break;
    case MK_FLAT:    mouthLine(ox, oy, 20, 4, C_TEAL); break;
    case MK_OPEN:    fbEllipse(CX + ox, CY + MOUTH_Y + oy - 14, 16, 18, C_TEAL); break;
    case MK_BLOB:    fbEllipse(CX + ox, CY + MOUTH_Y + oy - 12, 20, 14, C_TEAL); break;
    case MK_SMALL_O: fbEllipse(CX + ox, CY + MOUTH_Y + oy - 14,  9, 11, C_TEAL); break;
    case MK_TONGUE:  mouthTongue(ox, oy, t);  break;
    case MK_KISS3:   mouthKiss3(ox, oy, t);   break;
    case MK_SADWAVE: mouthSadWave(ox, oy);    break;
    case MK_WAVE:    mouthWave(ox, oy, 26, 5, 4, C_TEAL); break;
    case MK_SHOUT: {
      int mw = 44, mh = 28 + (int)(5 * sinf(t * 14.0f));
      fbRoundRect(CX + ox - mw / 2, CY + oy + MOUTH_Y - mh / 2, mw, mh, 10, C_MOUTH);
      for (int k2 = -1; k2 <= 1; k2++) {
        int tx = CX + ox + k2 * 14;
        int ty = CY + oy + MOUTH_Y - mh / 2;
        fbLine(tx - 6, ty + 1, tx, ty + 9, 4, C_WHITE);
        fbLine(tx, ty + 9, tx + 6, ty + 1, 4, C_WHITE);
      }
      break;
    }

    // — دورِ دوم —
    case MK_SQUIGGLE:  mouthSquiggle(ox, oy, 18);   break;
    case MK_ZIGZAG:    mouthZigzag(ox, oy, 20);     break;
    case MK_OVAL:      mouthOval(ox, oy, 9, 18);    break;
    case MK_TONGUEOUT: mouthTongueOut(ox, oy, t);   break;
    case MK_CROOKED:   mouthCrooked(ox, oy, 1);     break;
    case MK_BIGSMILE:  mouthBigSmile(ox, oy);       break;
  }
}

// ---------- لپ ----------
inline void faceBlush(int ox, int oy, BlushKind k, float t) {
  switch (k) {
    case BK_NONE:  break;
    case BK_OVAL:  blushOval(ox, oy, 26); break;
    case BK_BIG:   blushOval(ox, oy, 30 + (int)(3.0f * sinf(t * 3.0f))); break;
    case BK_HATCH: blushHatch(ox, oy);    break;
  }
}

// ---------- اضافات ----------
// جدا شده به «پشت» و «جلو» ، چون تاج و گوش پشتِ صورت‌اند و
// عينک و قلب جلويش.
inline void faceExtraBack(int ox, int oy, ExtraKind k, float t) {
  switch (k) {
    case XK_CROWN:     xCrown(ox, oy, t);     break;
    case XK_CATEARS:   xCatEars(ox, oy);      break;
    case XK_HORNS:     xHorns(ox, oy);        break;
    // — دورِ دوم — آن‌هايي که بايد **زيرِ** چهره بيفتند
    case XK_WIZARDHAT: xWizardHat(ox, oy, t); break;
    case XK_PARTYHAT:  xPartyHat(ox, oy);     break;
    case XK_SUN:       xSun(ox, oy, t);       break;
    case XK_MOON:      xMoon(ox, oy);         break;
    default: break;
  }
}

inline void faceExtraFront(int ox, int oy, ExtraKind k, float t) {
  switch (k) {
    case XK_GLASSES:   xGlasses(ox, oy);      break;
    case XK_NOTES:     xNotes(ox, oy, t);     break;
    case XK_BULB:      xBulb(ox, oy, t);      break;
    case XK_SWEAT:     xSweat(ox, oy, t);     break;
    case XK_ANGERMARK: xAngerMark(ox, oy, t); break;
    case XK_QUESTION:  xQuestion(ox, oy, t);  break;
    case XK_BUBBLE:    xBubble(ox, oy, t);    break;

    // — دورِ دوم —
    case XK_BATTERY:     xBattery(ox, oy, 8, t);   break;   // تقريبا خالي
    case XK_CHARGING:    xCharging(ox, oy, t);     break;
    case XK_MAGNIFIER:   xMagnifier(ox, oy, t);    break;
    case XK_HOURGLASS:   xHourglass(ox, oy, t);    break;
    case XK_SNOW:        xSnow(ox, oy, t);         break;
    case XK_THERMO:      xThermo(ox, oy, t);       break;
    case XK_FLOWER:      xFlower(ox, oy, t);       break;
    case XK_CANDY:       xCandy(ox, oy, t);        break;
    case XK_CONFETTI:    xConfetti(ox, oy, t);     break;
    case XK_EYEPATCH:    xEyePatch(ox, oy);        break;
    case XK_BROKENHEART: xBrokenHeart(ox, oy, t);  break;
    case XK_FANGS:       xFangs(ox, oy);           break;
    case XK_SWEATMULTI:  xSweatMulti(ox, oy, t);   break;

    case XK_SPARKLES:
      for (int i = 0; i < 4; i++) {
        float a = t * 2.4f + i * 1.571f;
        fbSparkle(CX + ox + (int)(132 * cosf(a)),
                  CY + oy - 24 + (int)(104 * sinf(a)), 11, C_STAR);
      }
      break;

    case XK_HEARTS:
      for (int i = 0; i < 3; i++) {
        float ph = fmodf(t * 0.9f + i * 0.33f, 1.0f);
        int hx = CX + ox + 60 + (int)(ph * 44);
        int hy = CY + oy + 40 - (int)(ph * 130);
        int hr = 22 - (int)(ph * 13);
        if (hr > 4) fbHeart(hx, hy, hr, C_HOT);
      }
      break;

    case XK_TEARS:
      for (int sd = -1; sd <= 1; sd += 2)
        for (int i = 0; i < 2; i++) {
          float ph = fmodf(t * 1.25f + i * 0.5f + (sd > 0 ? 0.27f : 0.0f), 1.0f);
          int tx = CX + sd * EYE_DX + ox + sd * 14;
          int ty = CY + oy + 14 + (int)(ph * ph * 150);
          int tr = 9 - (int)(ph * 3);
          if (tr > 3) fbDrop(tx, ty, tr, tr + 5, C_TEAR);
        }
      break;

    case XK_ZZZ:
      for (int i = 0; i < 3; i++) {
        float ph = fmodf(t * 0.5f + i * 0.33f, 1.0f);
        int zx = CX + ox + 80 + (int)(ph * 30);
        int zy = CY + oy - 30 - (int)(ph * 90);
        int zs = 26 - (int)(ph * 12);
        if (zs > 8) fbZ(zx, zy, zs, 3, C_WHITE);
      }
      break;

    default: break;
  }
}

// —— دست ——
//   ژست‌هاي «دو دستي» خودشان جا را مي‌دانند و فقط ox/oy
//   مي‌گيرند. ژست‌هاي «تک‌دستي» مختصاتِ مطلق مي‌خواهند ،
//   پس جايشان را همين‌جا يک بار تعيين مي‌کنم تا در ۱۲۶
//   چهره تکرار نشود.
inline void faceHand(int ox, int oy, HandKind k, float t) {
  //   ☠️ جاي قبلي (CX+۱۱۸ ، CY+۸) با شعاعِ ۲۲ تا ۲۶ دو ايراد
  //      داشت — هر دو را در رندرِ بيرون از برد ديدم:
  //        ۱ درست روي **کاسه‌ي چشمِ راست** مي‌افتاد (لبه‌ي چشم
  //          CX+۱۱۰ است) ، پس نويز مي‌شد نه ژست.
  //        ۲ آن‌قدر ريز بود که انگشت‌ها به دو پيکسل مي‌رسيدند.
  //      حالا بزرگ است و از **پايينِ راست** بالا مي‌آيد ، جلوي
  //      صورت — همان‌طور که در ورق‌هاي مرجع است.
  const int RX = CX + ox + 96;
  const int RY = CY + oy + 62;
  switch (k) {
    case HK_NONE:      break;
    case HK_WAVE:      handWave(RX, RY - 52, t, +1, 44); break;
    case HK_THUMBUP:   handThumbUp(RX, RY, 44);          break;
    case HK_VICTORY:   handVictory(RX - 8, RY + 2, 40);  break;
    case HK_POINT:     handPoint(RX + 6, RY, -1.35f, 40); break;
    case HK_OVERMOUTH: handOverMouth(ox, oy, t);  break;
    case HK_OVEREYES:  handsOverEyes(ox, oy, t);  break;
    case HK_OPEN:      handsOpen(ox, oy, t);      break;
    case HK_CLAP:      handsClap(ox, oy, t);      break;
    case HK_HEARTHAND: handsHeart(ox, oy, t);     break;
    case HK_SALUTE:    handSalute(ox, oy);        break;
    case HK_SHRUG:     handsShrug(ox, oy);        break;
    case HK_SHY:       handsShy(ox, oy, t);       break;
  }
}

// =====================================================
//   کشيدنِ کاملِ يک چهره از روي نسخه
// =====================================================
inline void drawRecipe(const FaceRecipe &r, int ox, int oy,
                       int gx, int gy, int open, float t) {
  fbFill(C_BG);

  if (r.flags & FR_SHAKE) {
    ox += (int)(2.5f * sinf(t * 31.0f));
    oy += (int)(1.5f * sinf(t * 27.0f));
  }

  faceExtraBack(ox, oy, r.extra, t);

  faceEye(-1, ox, oy, r.eye, gx, gy, open, t);
  faceEye(+1, ox, oy, r.eye, gx, gy, open, t);

  setBrowGaze(gx, gy);

  // ☠️ اضافاتِ **سرپوش** با ابرو جور نمي‌شوند.
  //
  //    اول فقط شاخ را استثنا کردم ، ولي کاربر گفت تاج و
  //    گوش و عينک هم همين‌طورند — و درست مي‌گفت. علتش يکي
  //    است و ارزش دارد نوشته شود:
  //
  //    ابرو روي نوارِ CY−۷۶ تا CY−۹۰ مي‌نشيند. همان نواري
  //    که پايه‌ي تاج ، ته‌ي گوشِ گربه و شاخ هم در آن‌اند.
  //    چهار شکل در يک نوارِ چهارده‌پيکسلي يعني شلوغي.
  //
  //    و نکته‌ي تصويريِ مهم‌تر: اين اضافات **خودشان** نقشِ
  //    ابرو را بازي مي‌کنند. چشم قوسِ گوشِ گربه يا زاويه‌ي
  //    شاخ را به عنوان «ابرو» مي‌خواند ، و عينک که کلا جاي
  //    ابرو را مي‌گيرد. ابروي اضافه پيام را دوبار مي‌گويد.
  //
  //    قاعده‌اش را در **کد** گذاشتم نه در جدول ، تا هر
  //    چهره‌ي سرپوش‌دارِ آينده هم خودبه‌خود درست باشد.
  bool headgear = (r.extra == XK_HORNS)     || (r.extra == XK_CROWN)    ||
                  (r.extra == XK_CATEARS)   || (r.extra == XK_GLASSES)  ||
                  (r.extra == XK_WIZARDHAT) || (r.extra == XK_PARTYHAT) ||
                  (r.extra == XK_EYEPATCH);
  if (!headgear)
    drawBrows(ox, oy, r.brow, (r.eye == EK_ROUND2) ? 84 : 76);

  faceBlush(ox, oy, r.blush, t);
  faceMouth(ox, oy, r.mouth, t);
  faceExtraFront(ox, oy, r.extra, t);
  faceHand(ox, oy, r.hand, t);          // دست هميشه رويِ همه چيز
}
