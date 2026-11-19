#pragma once
// =====================================================
//   MOOCHI — لایه‌ي کشیدن نواری (۱۱ مهر ۱۴۰۵)
//
//   موتور چهره برای صفحه‌ي گرد ۳۶۰×۳۶۰ نوشته شده و همه‌چیز را
//   روی یک «بوم مجازی» ۳۶۰×۳۶۰ می‌کشد.
//   صفحه‌ي ما ۲۴۰×۲۸۰ است و ESP32 ما PSRAM ندارد
//   (بزرگ‌ترین تکه‌ي آزاد ۱۰۷ KB؛ بافر کامل ۱۳۴ KB می‌خواهد).
//
//   راه‌حل:
//   ۱. بوم مجازی ۳۶۰ با ضریب ۲/۳ به ۲۴۰ نگاشت می‌شود (عرض کامل)
//      و ۲۰ ردیف از بالا فاصله می‌گیرد (۲۴۰ وسط ۲۸۰).
//   ۲. صفحه در چند نوار افقی کشیده می‌شود. برای هر نوار کل چهره
//      دوباره «کشیده» می‌شود ولی فقط پیکسل‌های همان نوار می‌مانند.
//
//   نسخه‌ي ۲ (۱۱ مهر ، بعد از اولین تست: ۷۴ ms برای چهره‌ي عادی ،
//   ۴۷۶ ms برای چهره‌ي دست‌دار — «کند و گیر می‌کند»):
//   • به جای ۴ نوار ثابت ، اول ۲ نوار ۱۴۰ ردیفی امتحان می‌شود
//     (بافر ۶۷ KB از heap) ؛ اگر جا نشد ۴ نوار ۷۰ ردیفی.
//   • شکل‌های اصلی ردیف‌های بیرون از نوار را اصلا حساب نمی‌کنند.
//   • ردیف‌ها با hspan یکجا پر می‌شوند ، نه پیکسل به پیکسل.
//   • شکل‌های «تست نقطه» (بیضی چرخیده ، قلب ، …) مستقیم روی
//     پیکسل‌های واقعی می‌چرخند ، نه پیکسل‌های مجازی (۲٫۲۵ برابر کمتر).
//   شرط درستی: تابع‌های کشیدن نباید عوارض جانبی داشته باشند
//   (random ، تغییر حالت). در کد اصلی چک شد: ندارند.
// =====================================================
#include <Arduino.h>

#define LCD_W 360          // بوم مجازی
#define LCD_H 360
#ifndef DISPLAY_ROUND
#define DISPLAY_ROUND 0
#endif
#define PHYS_W 240         // صفحه‌ي واقعی
#if DISPLAY_ROUND
#define PHYS_H 240         // ۱۲ مهر: نمایشگر گرد GC9A01 ، ۲۴۰×۲۴۰
#define PHYS_OY 0
#else
#define PHYS_H 280
#define PHYS_OY 20         // (۲۸۰ − ۲۴۰) / ۲
#endif

extern uint16_t *bandBuf;          // بافر یک نوار (PHYS_W × bandH)
extern int       bandH;            // ارتفاع نوار: ۱۴۰ یا ۷۰
extern int       bandN;            // تعداد نوار: ۲ یا ۴
extern int       bandY0;           // ردیف واقعیِ بالای نوار جاری
extern int       vBandY0, vBandY1; // بازه‌ي ردیف‌های مجازی که در نوار می‌افتند (شامل هر دو سر)
extern int16_t   vx2p[LCD_W];      // جدول مجازی ← واقعی
extern int16_t   vy2p[LCD_H];

// ۱۲ مهر: «لایه‌ي نئون» برای اضافات و دست‌های کاتالوگ — در گذرهای درخشش ،
// رنگ‌ها به جای نوشتن با «بیشینه‌ي هر کانال» ترکیب می‌شوند.
inline bool gfxMax = false;
inline uint16_t gfxBlend(uint16_t a, uint16_t b) {     // هر دو big-endian
  if (!a) return b;
  if (!b) return a;
  uint16_t A = (a >> 8) | (a << 8), B = (b >> 8) | (b << 8);
  uint16_t r = ((A & 0xF800) > (B & 0xF800)) ? (A & 0xF800) : (B & 0xF800);
  uint16_t g = ((A & 0x07E0) > (B & 0x07E0)) ? (A & 0x07E0) : (B & 0x07E0);
  uint16_t bl = ((A & 0x001F) > (B & 0x001F)) ? (A & 0x001F) : (B & 0x001F);
  uint16_t c = r | g | bl;
  return (c >> 8) | (c << 8);
}

inline void gfxInitTables() {
  for (int i = 0; i < LCD_W; i++) vx2p[i] = (int16_t)((i * 2) / 3);
  for (int i = 0; i < LCD_H; i++) vy2p[i] = (int16_t)((i * 2) / 3 + PHYS_OY);
}

// نوار شماره‌ي b را «جاری» کن
inline void gfxSetBand(int b) {
  bandY0 = b * bandH;
  int y1 = bandY0 + bandH;            // انحصاری
  vBandY0 = LCD_H; vBandY1 = -1;
  for (int y = 0; y < LCD_H; y++) {
    if (vy2p[y] >= bandY0 && vy2p[y] < y1) {
      if (y < vBandY0) vBandY0 = y;
      vBandY1 = y;
    }
  }
}

// آیا بازه‌ي عمودی [ya, yb] مجازی با نوار جاری هم‌پوشانی دارد؟
inline bool vRowsHit(int ya, int yb) { return yb >= vBandY0 && ya <= vBandY1; }

// پر کردن یک تکه‌ي افقی (مختصات مجازی ، شامل هر دو سر)
inline void hspan(int xa, int xb, int y, uint16_t s) {
  if (y < vBandY0 || y > vBandY1) return;
  if (xa < 0) xa = 0;
  if (xb > LCD_W - 1) xb = LCD_W - 1;
  if (xa > xb) return;
  uint16_t *row = bandBuf + (vy2p[y] - bandY0) * PHYS_W;
  int X1 = vx2p[xb];
  if (gfxMax) { for (int X = vx2p[xa]; X <= X1; X++) row[X] = gfxBlend(row[X], s); }
  else        { for (int X = vx2p[xa]; X <= X1; X++) row[X] = s; }
}

// پیمایش پیکسل‌های واقعیِ داخل یک جعبه‌ي مجازی ، فقط در نوار جاری.
// برای هر پیکسل واقعی ، مختصات مجازیِ مرکزش (vx , vy) را به تابع می‌دهد.
// fn(vx, vy) باید true برگرداند اگر پیکسل داخل شکل است.
template <class F>
inline void physFill(int vxa, int vya, int vxb, int vyb, uint16_t s, F fn) {
  if (vxa < 0) vxa = 0;
  if (vya < 0) vya = 0;
  if (vxb > LCD_W - 1) vxb = LCD_W - 1;
  if (vyb > LCD_H - 1) vyb = LCD_H - 1;
  if (vxa > vxb || vya > vyb) return;
  int Ya = vy2p[vya], Yb = vy2p[vyb];
  if (Ya < bandY0) Ya = bandY0;
  if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
  int Xa = vx2p[vxa], Xb = vx2p[vxb];
  for (int Y = Ya; Y <= Yb; Y++) {
    float vy = (Y - PHYS_OY) * 1.5f + 0.5f;
    uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
    for (int X = Xa; X <= Xb; X++) {
      float vx = X * 1.5f + 0.5f;
      if (fn(vx, vy)) row[X] = gfxMax ? gfxBlend(row[X], s) : s;
    }
  }
}

// ۱۲ مهر: نوار با شروع و ارتفاع دلخواه (برای فرستادن فقط ردیف‌های صورت)
inline void gfxSetBandRows(int y0, int h) {
  bandY0 = y0;
  bandH = h;
  int y1 = y0 + h;
  vBandY0 = LCD_H; vBandY1 = -1;
  for (int y = 0; y < LCD_H; y++) {
    if (vy2p[y] >= y0 && vy2p[y] < y1) {
      if (y < vBandY0) vBandY0 = y;
      vBandY1 = y;
    }
  }
}
