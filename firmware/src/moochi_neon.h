#pragma once
// =====================================================
//   MOOCHI — چشم‌های نئونی صورتی (۱۱ مهر ۱۴۰۵)
//
//   سبک تازه از عکسی که مهسن فرستاد (photos/photo_2026-10-03_17-29-00.jpg):
//   • دو چشم کپسولی ایستاده (قرص) با درخشش صورتی دورشان
//   • داخل چشم: شیب رنگ ، بالا ارغوانی تیره ← پایین صورتی‌سفید
//   • لبه‌ي داخلی کمی روشن‌تر
//   • برق بزرگ بیضی کج بالا-چپ + یک نقطه‌ي کوچک زیرش
//   • لبخند باریک بین دو چشم ، با همان درخشش
//
//   این شکل‌ها مستقیم روی پیکسل‌های واقعی (۲۴۰×۲۸۰) و فقط در نوار جاری
//   کشیده می‌شوند — هر پیکسل یک بار محاسبه می‌شود («شیدر» ساده).
//   زمینه مشکی است ، پس درخشش = رنگ × شدت (بدون خواندن رنگ زیرین).
// =====================================================
#include <Arduino.h>
#include <math.h>
#include "moochi_gfx.h"

enum NeonKind { NK_NORMAL, NK_HAPPY, NK_SURPRISE, NK_SLEEPY, NK_ASLEEP,
                // ۱۲ مهر: واکنش‌های تازه به تکان و ضربه — همه با همین سبک
                NK_DIZZY,    // مارپیچ چرخان + دهان موجی
                NK_SAD,      // درد/ناراحتی: پلک کج (بیرون پایین) + اخم
                NK_CRY,      // مثل SAD + اشک آبی که می‌چکد
                NK_ANGRY,    // پلک کج (داخل پایین) + اخم کوچک + درخشش سرخ‌تر
                NK_CURIOUS,  // یک چشم درشت‌تر + دهان کوچک
                // ۱۲ مهر (دوم): بقیه‌ي حالت‌ها هم نئونی شدند
                NK_PET,      // لوس شده: ^ ^ + گونه‌ي صورتی + لبخند پهن
                NK_LOVE,     // عاشق: چشم قلبی که می‌تپد
                NK_THINK,    // فکر: نگاه بالا-راست + پلک صاف + «...»
                NK_SHY,      // خجالت: نگاه پایین + گونه + دهان موجی کوچک
                NK_EXCITED,  // هیجان: چشم ستاره‌ای + دهان باز
                NK_YAWN,     // خمیازه: چشم بسته + دهان بیضی که باز و بسته می‌شود
                NK_LISTEN,   // گوش دادن به موسیقی: هدفون + نت‌های شناور + سر تکان دادن
                NK_KISS,     // بوس: چشمک + لب غنچه + قلب کوچک
                NK_EYEROLL,  // چشم چرخاندن: پلک نیمه + نگاه که بالا می‌چرخد
                NK_HEARTHAND,// قلب: ^ ^ + قلب بزرگ پایین
                NK_FEED,     // غذا خوردن: یک غذا (۶ نوع) می‌آید ، گاز زده می‌شود ، می‌جود
                NK_RAINBOW,  // رنگین‌کمان بالای سر
                // شکل‌های چشم برای ۷۴ چهره‌ي کاتالوگ (فقط داخل EyeCfg به کار می‌روند)
                NK_E_CARET,  // <  (چشم راست ؛ چپ آینه = >)
                NK_E_UP,     // ˄
                NK_E_LINE,   // ―
                NK_E_WOBBLE, // ∿ لرزان
                NK_E_RING,   // ○
                NK_E_DOT,    // • کوچک
                NK_E_CROSS   // ✗
};

struct NeonParams {
  float gx = 0, gy = 0;   // جابه‌جایی نگاه (پیکسل واقعی)
  float fx = 0, fy = 0;   // جابه‌جایی کل صورت (پیکسل واقعی)
  float open = 1;         // ۱ باز ، ۰ بسته (پلک)
  NeonKind kind = NK_NORMAL;
  float t = 0;            // ثانیه‌های گذشته از شروع حالت (برای انیمیشن)
  int food = 0;           // غذا خوردن: کدام غذا (۰ تا ۵)
};

// ---------- اندازه‌ها (پیکسل واقعی) ----------
const float NE_CX    = 120;   // وسط صورت
// ۱۲ مهر: روی نمایشگر گرد (۲۴۰ ردیف) همه‌چیز ۲۰ پیکسل بالاتر است (NE_OY = −۲۰)
#define NE_OY ((float)PHYS_OY - 20)
const float NE_CY    = 136 + NE_OY;   // ارتفاع چشم‌ها
const float NE_DX    = 52;    // فاصله‌ي هر چشم از وسط
const float NE_HW    = 24;    // نصف عرض چشم
const float NE_HH    = 34;    // نصف ارتفاع چشم
const float NE_GLOW  = 20;    // پهنای درخشش
const float NE_SMILE_Y  = 163 + NE_OY;  // لبخند
const float NE_SMILE_HW = 20;   // نصف پهنای لبخند
const float NE_SMILE_D  = 7;    // گودی لبخند

// ---------- رنگ‌ها (۰ تا ۲۵۵) ----------
struct RGBf { float r, g, b; };
const RGBf NE_TOP   = {165,   0, 118};   // بالای چشم — ارغوانی
const RGBf NE_BOT   = {255, 246, 253};   // پایین چشم — صورتی‌سفید
const RGBf NE_RIM   = {255, 185, 240};   // لبه‌ي داخلی
const RGBf NE_GLOWC = {255,  30, 190};   // درخشش
const RGBf NE_SHINE = {255, 246, 252};   // برق
const RGBf NE_LINE  = {255, 222, 247};   // لبخند و چشم بسته
const RGBf NE_TEAR  = {140, 210, 255};   // اشک
const RGBf NE_TEARG = { 40, 120, 255};   // درخشش اشک
const RGBf NE_ANGRY_TOP  = {190,   0,  55};
const RGBf NE_ANGRY_GLOW = {255,  35,  80};

inline uint16_t neon565(float r, float g, float b) {
  int R = r > 255 ? 255 : (r < 0 ? 0 : (int)r);
  int G = g > 255 ? 255 : (g < 0 ? 0 : (int)g);
  int B = b > 255 ? 255 : (b < 0 ? 0 : (int)b);
  uint16_t c = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3);
  return (c >> 8) | (c << 8);           // big-endian ، مثل بقیه‌ي بافر
}
inline RGBf mixc(const RGBf &a, const RGBf &b, float t) {
  return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t};
}

// فاصله تا کپسول/مستطیل گردگوشه (منفی = داخل)
inline float sdRoundBox(float px, float py, float hw, float hh, float r) {
  float qx = fabsf(px) - (hw - r), qy = fabsf(py) - (hh - r);
  // ۱۲ مهر: sqrt فقط در گوشه‌ها (ESP32 در sqrt و تقسیم کند است)
  if (qx <= 0 && qy <= 0) return (qx > qy ? qx : qy) - r;
  if (qx <= 0) return qy - r;
  if (qy <= 0) return qx - r;
  return sqrtf(qx * qx + qy * qy) - r;
}

// فاصله تا یک قوس سهمی (لبخند): y = depth·(1 − (x/hw)²) ، نیم‌ضخامت th
// فاصله‌ي واقعی تا یک خط شکسته‌ي ۱۲ تکه روی سهمی. (نسخه‌ي اول تقریب
// عمودی بود و زیر دو سر لبخند یک لبه‌ي صاف در درخشش می‌انداخت.)
inline float sdSmile(float px, float py, float hw, float depth, float th) {
  const int N = 12;
  float best = 1e9f;
  float x0 = -hw, y0 = 0;
  for (int i = 1; i <= N; i++) {
    float u = -1 + 2.0f * i / N;
    float x1 = u * hw, y1 = depth * (1 - u * u);
    float vx = x1 - x0, vy = y1 - y0;
    float wx = px - x0, wy = py - y0;
    float t = (wx * vx + wy * vy) / (vx * vx + vy * vy);
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    float dx = wx - vx * t, dy = wy - vy * t;
    float d2 = dx * dx + dy * dy;
    if (d2 < best) best = d2;
    x0 = x1; y0 = y1;
  }
  return sqrtf(best) - th;
}

// فاصله تا قوس دایره‌ای رو به بالا (چشم خوشحال ^)
inline float sdArcUp(float px, float py, float R, float th) {
  // فقط نیمه‌ي بالایی دایره ، با دو سر گرد
  if (py <= 0) return fabsf(sqrtf(px * px + py * py) - R) - th;
  float dx = fabsf(px) - R;
  return sqrtf(dx * dx + py * py) - th;
}

// فاصله تا مارپیچ ارشمیدسی r = k·(θ − rot) در دایره‌ي شعاع R (چشم سرگیجه)
inline float sdSpiral(float px, float py, float k, float rot, float R, float th) {
  float r = sqrtf(px * px + py * py);
  float a = atan2f(py, px) - rot;
  float f = (r / k - a) / 6.2832f;          // چند دور
  float fr = f - floorf(f + 0.5f);
  float d = fabsf(fr) * 6.2832f * k - th;
  float out = r - R;                         // بیرون دایره
  return d > out ? d : out;
}

// فاصله تا خط شکسته‌ي n نقطه‌ای (دهان‌های مختلف)
inline float sdPoly(float px, float py, const float *xs, const float *ys, int n, float th) {
  float best = 1e9f;
  for (int i = 1; i < n; i++) {
    float vx = xs[i] - xs[i-1], vy = ys[i] - ys[i-1];
    float wx = px - xs[i-1], wy = py - ys[i-1];
    float L = vx * vx + vy * vy;
    float t = L > 0 ? (wx * vx + wy * vy) / L : 0;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    float dx = wx - vx * t, dy = wy - vy * t;
    float d2 = dx * dx + dy * dy;
    if (d2 < best) best = d2;
  }
  return sqrtf(best) - th;
}

// فاصله تا قلب (Inigo Quilez) — مختصات با y رو به بالا ، نوک پایین در (۰،۰) ، بالا حدود ۱٫۱
inline float sdHeartN(float x, float y) {
  x = fabsf(x);
  if (y + x > 1) { float dx = x - 0.25f, dy = y - 0.75f; return sqrtf(dx * dx + dy * dy) - 0.35355f; }
  float ax = x, ay = y - 1;
  float m = (x + y) > 0 ? (x + y) * 0.5f : 0;
  float bx = x - m, by = y - m;
  float d = sqrtf(fminf(ax * ax + ay * ay, bx * bx + by * by));
  return (x - y) > 0 ? d : -d;
}
// قلب با پهنای نیمه‌ي hw ، مرکزش در (۰،۰) ، y صفحه رو به پایین
inline float sdHeart(float px, float py, float hw) {
  float S = hw / 0.6f;
  return sdHeartN(px / S, -py / S + 0.55f) * S;
}
// ستاره‌ي ۵ پر (Inigo Quilez) — r شعاع ، rf نسبت فرورفتگی ؛ y صفحه رو به پایین
inline float sdStar5(float px, float py, float r, float rf) {
  const float k1x = 0.809017f, k1y = -0.587785f, k2x = -0.809017f, k2y = -0.587785f;
  float x = fabsf(px), y = -py;
  float d1 = k1x * x + k1y * y; if (d1 > 0) { x -= 2 * d1 * k1x; y -= 2 * d1 * k1y; }
  float d2 = k2x * x + k2y * y; if (d2 > 0) { x -= 2 * d2 * k2x; y -= 2 * d2 * k2y; }
  x = fabsf(x); y -= r;
  float bax = rf * (-k1y) - 0, bay = rf * k1x - 1;
  float h = (x * bax + y * bay) / (bax * bax + bay * bay);
  if (h < 0) h = 0;
  if (h > r) h = r;
  float dx = x - bax * h, dy = y - bay * h;
  float d = sqrtf(dx * dx + dy * dy);
  return (y * bax - x * bay) > 0 ? d : -d;
}
// فاصله‌ي تقریبی تا بیضی (برای دهان‌های گرد)
inline float sdEllipseA(float px, float py, float rx, float ry) {
  float k = sqrtf((px * px) / (rx * rx) + (py * py) / (ry * ry));
  return (k - 1) * (rx < ry ? rx : ry);
}

// رنگ داخل قلب/ستاره: همان شیب و لبه‌ي چشم + یک برق کوچک
inline RGBf shapeFill(float lx, float ly, float d, float R) {
  float t = (ly + R) / (2 * R);
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  RGBf c = mixc(NE_TOP, NE_BOT, t * t);
  if (d > -3.0f) c = mixc(c, NE_RIM, 1 + d / 3.0f);
  float sx = lx + R * 0.38f, sy = ly + R * 0.30f, rr = R * 0.16f;
  if (sx * sx + sy * sy <= rr * rr) c = NE_SHINE;
  return c;
}

// چشم کپسولی با شیب رنگ و برق — اگر پیکسل داخل است رنگ را می‌دهد
// lidK: شیب پلک (۰ = بدون پلک) ؛ lidY: ارتفاع پلک نسبت به مرکز چشم.
// پیکسل‌های بالای خط پلک بیرون حساب می‌شوند (شکل = کپسول ∩ زیر پلک).
inline bool eyeColor(float lx, float ly, float hw, float hh, float shine, RGBf &out, float &dist,
                     float lidK = 0, float lidY = -1e9f, const RGBf &top = NE_TOP) {
  float d = sdRoundBox(lx, ly, hw, hh, hw < hh ? hw : hh);   // اگر پلک بسته شد ، کپسول خوابیده
  if (lidY > -1e8f) {
    float dl = ((lidY + lidK * lx) - ly) / sqrtf(1 + lidK * lidK);   // مثبت = بالای پلک
    if (dl > d) d = dl;
  }
  dist = d;
  if (d > 0) return false;
  float t = (ly + hh) / (2 * hh);
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  RGBf c = mixc(top, NE_BOT, t * t);
  // نوار روشن عمودی کنار چپِ داخل چشم (مثل عکس)
  float lx2 = lx + hw * 0.62f;
  if (lx2 > -hw * 0.12f && lx2 < hw * 0.12f && t > 0.3f) c = mixc(c, NE_RIM, 0.35f * (t - 0.3f) / 0.7f);
  if (d > -3.0f) c = mixc(c, NE_RIM, 1 + d / 3.0f);                 // لبه‌ي روشن
  if (shine > 0) {
    // برق بزرگ: بیضی کج بالا-چپ
    float sx = lx + hw * 0.30f, sy = ly + hh * 0.52f;
    float ca = 0.766f, sa = 0.643f;                                  // ۴۰ درجه ، سرش رو به بالا-راست
    float u = sx * ca - sy * sa, v = sx * sa + sy * ca;
    float rx = hw * 0.20f * shine, ry = hh * 0.20f * shine;
    { float k = rx; rx = ry * 1.0f; ry = k * 0.62f; }                // بیضی کشیده در جهت کج
    rx = hw * 0.34f * shine; ry = hw * 0.20f * shine;
    if (rx > 0.5f && ry > 0.5f && (u * u) / (rx * rx) + (v * v) / (ry * ry) <= 1) c = NE_SHINE;
    // نقطه‌ي کوچک
    float dx = lx + hw * 0.56f, dy = ly + hh * 0.22f;
    float rr = hw * 0.12f * shine;
    if (dx * dx + dy * dy <= rr * rr) c = NE_SHINE;
  }
  out = c;
  return true;
}

// ردیف‌هایی که صورت نئونی (با درخشش ، اشک و Zها) ممکن است رویشان بیفتد.
// main.cpp فقط همین ردیف‌ها را می‌کشد و می‌فرستد (بقیه‌ي صفحه مشکی می‌ماند).
// نگاهِ خاصِ بعضی حالت‌ها (فکر ، خجالت ، گوش دادن ، چشم چرخاندن)
inline void neonGaze(NeonParams &P) {
  switch (P.kind) {
    case NK_THINK:  P.gx += 8; P.gy -= 9; break;
    case NK_SHY:    P.gx -= 5; P.gy += 7; break;
    case NK_LISTEN: P.fy += 2.2f * sinf(P.t * 6.0f); break;           // سر با آهنگ تکان می‌خورد
    case NK_FEED:   if (P.t < 0.8f) { P.gx += 6; P.gy += 3; } break;  // به غذا نگاه می‌کند
    case NK_EYEROLL: {                     // از چپ ، از بالای سر ، به راست
      float a = P.t < 1.1f ? P.t / 1.1f * 3.1416f : 3.1416f;
      P.gx += -10 * cosf(a); P.gy += -9 * sinf(a) - 2;
      break;
    }
    default: break;
  }
}

inline void neonRowRange(const NeonParams &P0, int &y0, int &y1) {
  NeonParams P = P0; neonGaze(P);
  float cy = NE_CY + P.fy + P.gy;
  y0 = (int)(cy - NE_HH * 1.3f - NE_GLOW - 6);
  y1 = (int)(NE_SMILE_Y + P.fy + NE_SMILE_D + 16 + NE_GLOW);
  if (P.kind == NK_CRY)    y1 = (int)(cy + NE_HH + 4 + 52 + 6 + 10);
  if (P.kind == NK_ASLEEP) y0 = (int)(34 + NE_OY);        // Zها بالا می‌روند
  if (P.kind == NK_THINK)  y0 = (int)(30 + NE_OY);        // «...» بالا-راست
  if (P.kind == NK_RAINBOW) y0 = (int)(30 + NE_OY);       // رنگین‌کمان
  if (P.kind == NK_KISS)   y0 = (int)(36 + NE_OY);        // قلب کوچک بالا می‌رود
  if (P.kind == NK_LISTEN) y0 = (int)(NE_CY + P.fy + 8 - 128);   // هدفون و نت‌ها (نت تا ۱۲۰ پیکسل بالای مرکز می‌رود)
  if (P.kind == NK_HEARTHAND || P.kind == NK_YAWN || P.kind == NK_FEED) y1 = (int)(NE_SMILE_Y + P.fy + 70);
  if (P.kind == NK_FEED) y0 = (int)(NE_SMILE_Y + P.fy - 36 - 44);   // غذا از بالا-راست می‌آید
  if (P.kind == NK_FEED && y0 > (int)(cy - NE_HH * 1.3f - NE_GLOW - 6)) y0 = (int)(cy - NE_HH * 1.3f - NE_GLOW - 6);
  if (y0 < 0) y0 = 0;
  if (y1 > PHYS_H - 1) y1 = PHYS_H - 1;
}

// =====================================================
//   کشیدن — نسخه‌ي ۳ (۱۲ مهر): «برچسب» چشم
//
//   مشکل: محاسبه‌ي درخشش برای هر پیکسل هر فریم روی ESP32 گران است
//   (مهسن: «خیلی روون نیست»). ولی چشم بین دو فریم معمولا عوض نمی‌شود —
//   فقط جابه‌جا می‌شود (نگاه). پس یک بار تصویر یک چشم (با درخشش) در یک
//   بافر کوچک «برچسب» ساخته می‌شود و هر فریم فقط کپی می‌شود.
//   فقط وقتی شکل چشم عوض شود (پلک ، حالت تازه ، مارپیچ چرخان) دوباره ساخته می‌شود.
//   چشم چپ همان برچسب است ؛ در حالت‌هایی که پلک کج دارند (غمگین/عصبانی)
//   و سرگیجه ، چشم چپ آینه‌ي چشم راست است.
//   ترکیب درخشش‌ها: بیشینه‌ي هر کانال رنگ (= همان «نزدیک‌ترین شکل» نسخه‌ي قبل).
// =====================================================

struct EyeCfg {
  NeonKind K;
  float hw, eh, shine, lidK, lidY, spin;
  bool closed;
  const RGBf *top, *glow;
};

// رنگ یک پیکسل از یک چشم (مختصات محلی چشم). ۰ = سیاه.
inline uint16_t eyePixel(const EyeCfg &E, float lx, float ly) {
  float d;
  RGBf c;
  bool inside = false;
  if (E.K == NK_HAPPY) {
    d = sdArcUp(lx, ly + 6, E.hw * 0.95f, 3.2f);
    if (d <= 0) { c = mixc(NE_LINE, NE_RIM, 0.35f); inside = true; }
  } else if (E.K == NK_DIZZY) {
    d = sdSpiral(lx, ly, 2.6f, E.spin, E.hw * 1.05f, 2.0f);
    if (d <= 0) { c = NE_LINE; inside = true; }
  } else if (E.K == NK_LOVE) {
    d = sdHeart(lx, ly, E.hw * 1.08f);
    if (d <= 0) { c = shapeFill(lx, ly, d, E.hw); inside = true; }
  } else if (E.K == NK_EXCITED) {
    d = sdStar5(lx, ly + 2, E.hw * 1.25f, 0.5f);
    if (d <= 0) { c = shapeFill(lx, ly, d, E.hw * 1.1f); inside = true; }
  } else if (E.K >= NK_E_CARET && E.K <= NK_E_CROSS && E.K != NK_E_RING && E.K != NK_E_DOT) {
    float s = E.hw / 24.0f;
    static const float CAx[3] = {10, -10, 10},   CAy[3] = {-14, 0, 14};
    static const float UPx[3] = {-14, 0, 14},    UPy[3] = {8, -8, 8};
    static const float LNx[2] = {-16, 16},       LNy[2] = {0, 0};
    static const float WBx[7] = {-16, -10.7f, -5.3f, 0, 5.3f, 10.7f, 16}, WBy[7] = {2, -2, 2, -2, 2, -2, 2};
    static const float X1x[2] = {-12, 12}, X1y[2] = {-12, 12}, X2x[2] = {-12, 12}, X2y[2] = {12, -12};
    float x = lx / s, y = ly / s;
    if (E.K == NK_E_CARET)       d = sdPoly(x, y, CAx, CAy, 3, 3.4f);
    else if (E.K == NK_E_UP)     d = sdPoly(x, y, UPx, UPy, 3, 3.4f);
    else if (E.K == NK_E_LINE)   d = sdPoly(x, y, LNx, LNy, 2, 3.0f);
    else if (E.K == NK_E_WOBBLE) d = sdPoly(x, y, WBx, WBy, 7, 2.8f);
    else { float a = sdPoly(x, y, X1x, X1y, 2, 3.2f), b = sdPoly(x, y, X2x, X2y, 2, 3.2f); d = a < b ? a : b; }
    d *= s;
    if (d <= 0) { c = NE_LINE; inside = true; }
  } else if (E.K == NK_E_RING) {
    float r = sqrtf(lx * lx + ly * ly);
    d = fabsf(r - E.hw * 0.8f) - 3.0f;
    if (d <= 0) { c = NE_LINE; inside = true; }
  } else if (E.K == NK_E_DOT) {
    float R = E.hw * 0.42f;
    d = sqrtf(lx * lx + ly * ly) - R;
    if (d <= 0) { c = shapeFill(lx, ly, d, R); inside = true; }
  } else if (E.closed) {
    d = sdSmile(lx, ly + 2, E.hw * 0.85f, 4, 2.4f);
    if (d <= 0) { c = NE_LINE; inside = true; }
  } else {
    if (eyeColor(lx, ly, E.hw, E.eh, E.shine, c, d, E.lidK, E.lidY, *E.top)) inside = true;
  }
  if (inside) return neon565(c.r, c.g, c.b);
  if (d >= NE_GLOW) return 0;
  float k = 1 - d * (1.0f / NE_GLOW);
  k = k * k * k * 0.85f;
  return neon565(E.glow->r * k, E.glow->g * k, E.glow->b * k);
}

// ترکیب «بیشینه‌ي هر کانال» دو رنگ big-endian
inline uint16_t maxBlend(uint16_t a, uint16_t b) {
  if (!a) return b;
  if (!b) return a;
  uint16_t A = (a >> 8) | (a << 8), B = (b >> 8) | (b << 8);
  uint16_t r = ((A & 0xF800) > (B & 0xF800)) ? (A & 0xF800) : (B & 0xF800);
  uint16_t g = ((A & 0x07E0) > (B & 0x07E0)) ? (A & 0x07E0) : (B & 0x07E0);
  uint16_t bl = ((A & 0x001F) > (B & 0x001F)) ? (A & 0x001F) : (B & 0x001F);
  uint16_t c = r | g | bl;
  return (c >> 8) | (c << 8);
}

// ---------- برچسب (۲ جا: چشم معمولی ، و چشم کوچک‌ترِ «کنجکاو») ----------
struct Sprite {
  uint16_t *buf = nullptr;
  int cap = 0, w = 0, h = 0;
  bool valid = false;
  EyeCfg key;
};
static Sprite spr[2];

inline bool sameCfg(const EyeCfg &a, const EyeCfg &b) {
  return a.K == b.K && a.hw == b.hw && a.eh == b.eh && a.shine == b.shine &&
         a.lidK == b.lidK && a.lidY == b.lidY && a.spin == b.spin &&
         a.closed == b.closed && a.top == b.top && a.glow == b.glow;
}

// اندازه‌ي برچسب برای یک چشم (نصف عرض/ارتفاع با درخشش)
inline void eyeExtent(const EyeCfg &E, int &hx, int &hy) {
  float rx = E.hw * 1.06f, ry = E.eh;
  if (E.K == NK_DIZZY) ry = rx;
  if (E.K == NK_HAPPY) { rx = E.hw * 0.95f + 3.2f; ry = E.hw * 0.95f + 3.2f; }
  if (E.K == NK_LOVE || E.K == NK_EXCITED) { rx = E.hw * 1.3f; ry = E.hw * 1.3f; }
  if (E.K >= NK_E_CARET && E.K <= NK_E_CROSS) { rx = E.hw * 0.95f; ry = E.hw * 0.95f; }
  if (E.closed) ry = 8;
  hx = (int)(rx + NE_GLOW + 2);
  hy = (int)(ry + NE_GLOW + 8);
}

inline bool buildSprite(Sprite &S, const EyeCfg &E) {
  if (S.valid && sameCfg(E, S.key)) return true;
  int hx, hy;
  eyeExtent(E, hx, hy);
  int w = hx * 2, h = hy * 2;
  if (w * h > S.cap) {
    if (S.buf) free(S.buf);
    S.cap = 0;
    S.buf = (uint16_t *)malloc((size_t)w * h * 2);
    if (!S.buf) { S.valid = false; return false; }
    S.cap = w * h;
  }
  S.w = w; S.h = h;
  uint16_t *p = S.buf;
  for (int sy = 0; sy < h; sy++) {
    float ly = sy + 0.5f - hy;
    for (int sx = 0; sx < w; sx++) {
      float lx = sx + 0.5f - hx;
      *p++ = eyePixel(E, lx, ly);
    }
  }
  S.key = E;
  S.valid = true;
  return true;
}

// کپی برچسب در نوار جاری ، با مرکز (cx , cy) ؛ mirror = آینه‌ي افقی
inline void blitBuf(const uint16_t *buf, int w, int h, int x0, int y0, bool mirror) {
  for (int sy = 0; sy < h; sy++) {
    int Y = y0 + sy;
    if (Y < bandY0 || Y >= bandY0 + bandH) continue;
    uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
    const uint16_t *src = buf + sy * w;
    for (int sx = 0; sx < w; sx++) {
      int X = x0 + (mirror ? (w - 1 - sx) : sx);
      if ((unsigned)X >= PHYS_W) continue;
      uint16_t c = src[sx];
      if (c) row[X] = maxBlend(row[X], c);
    }
  }
}
inline void blitSprite(const Sprite &S, float cx, float cy, bool mirror) {
  blitBuf(S.buf, S.w, S.h, (int)lroundf(cx) - S.w / 2, (int)lroundf(cy) - S.h / 2, mirror);
}
// ۱۲ مهر: کپی چرخیده (برای مارپیچ سرگیجه). قبلا مارپیچ هر فریم از نو حساب می‌شد
// (۹۰ ms روی برد). حالا یک بار ساخته می‌شود و فقط چرخانده می‌شود.
inline void blitRot(const Sprite &S, float cx, float cy, float ang, bool mirror) {
  int hx = S.w / 2, hy = S.h / 2, R = hx > hy ? hx : hy;
  float c = cosf(ang), s = sinf(ang);
  int Ya = (int)cy - R, Yb = (int)cy + R, Xa = (int)cx - R, Xb = (int)cx + R;
  if (Ya < bandY0) Ya = bandY0;
  if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
  if (Xa < 0) Xa = 0;
  if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
  for (int Y = Ya; Y <= Yb; Y++) {
    uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
    float dy = Y + 0.5f - cy;
    for (int X = Xa; X <= Xb; X++) {
      float dx = X + 0.5f - cx;
      float sx = c * dx + s * dy, sy = -s * dx + c * dy;
      if (mirror) sx = -sx;
      int ix = (int)floorf(sx + hx), iy = (int)floorf(sy + hy);
      if ((unsigned)ix >= (unsigned)S.w || (unsigned)iy >= (unsigned)S.h) continue;
      uint16_t col = S.buf[iy * S.w + ix];
      if (col) row[X] = maxBlend(row[X], col);
    }
  }
}

// ---------- برچسب دهان ----------
static uint16_t *nmBuf = nullptr;
static int nmCap = 0, nmW = 0, nmH = 0, nmOX = 0, nmOY = 0;   // OX/OY: جای (۰،۰)ِ دهان در برچسب
static int nmKeyK = -1, nmKeyQ = -1;

// کشیدن مستقیم یک چشم (وقتی برچسب جا نشد ، یا چشم کوچک‌ترِ «کنجکاو»)
inline void directEye(const EyeCfg &E, float cx, float cy, float scale) {
  int hx, hy;
  eyeExtent(E, hx, hy);
  hx = (int)(hx * scale) + 1; hy = (int)(hy * scale) + 1;
  int Ya = (int)cy - hy, Yb = (int)cy + hy;
  if (Ya < bandY0) Ya = bandY0;
  if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
  int Xa = (int)cx - hx, Xb = (int)cx + hx;
  if (Xa < 0) Xa = 0;
  if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
  float inv = 1.0f / scale;
  for (int Y = Ya; Y <= Yb; Y++) {
    uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
    float ly = (Y + 0.5f - cy) * inv;
    for (int X = Xa; X <= Xb; X++) {
      uint16_t c = eyePixel(E, (X + 0.5f - cx) * inv, ly);
      if (c) row[X] = maxBlend(row[X], c);
    }
  }
}

// ---------- کشیدن مستقیم یک شکل با درخشش (برای چیزهای کوچک: گونه ، قلب ، موج ، …) ----------
// sdf(x, y) فاصله‌ي پیکسل از شکل است (منفی = داخل). bright: ضریب روشنایی (برای کم‌رنگ شدن).
template <class F>
inline void glowDraw(int x0, int y0, int x1, int y1, F sdf, const RGBf &core, const RGBf &glow,
                     float gw, float bright = 1) {
  if (y0 < bandY0) y0 = bandY0;
  if (y1 > bandY0 + bandH - 1) y1 = bandY0 + bandH - 1;
  if (x0 < 0) x0 = 0;
  if (x1 > PHYS_W - 1) x1 = PHYS_W - 1;
  if (bright <= 0.02f) return;
  for (int Y = y0; Y <= y1; Y++) {
    uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
    for (int X = x0; X <= x1; X++) {
      float d = sdf(X + 0.5f, Y + 0.5f);
      uint16_t c = 0;
      if (d <= 0) c = neon565(core.r * bright, core.g * bright, core.b * bright);
      else if (d < gw) {
        float k = 1 - d / gw; k = k * k * k * 0.85f * bright;
        c = neon565(glow.r * k, glow.g * k, glow.b * k);
      }
      if (c) row[X] = maxBlend(row[X], c);
    }
  }
}

const RGBf NE_BLUSH  = {150, 30, 95};    // گونه (کم‌رنگ ، چون روی مشکی است)
const RGBf NE_BLUSHG = {255, 40, 150};

// ---------- شکل دهان ----------
struct MouthShape {
  int type = 1;                 // ۰ هیچ ، ۱ خط شکسته ، ۲ بیضی توخالی ، ۳ دهان باز D ، ۵ چهارگوش توخالی
  float xs[13], ys[13]; int n = 0;   // نوع ۱: خود دهان ؛ نوع ۳ و ۵: خط اضافه (دندان)
  float ex = 0, ey = 0, rx = 0, ry = 0;
  bool fill = false; float fx = 0, fy = 0, frx = 0, fry = 0;   // زبان (بیضی پر ، صورتی)
};
inline void mouthSmile(MouthShape &M, float hw, float depth, float yo) {
  M.type = 1; M.n = 13;
  for (int i = 0; i < 13; i++) { float u = -1 + 2.0f * i / 12; M.xs[i] = u * hw; M.ys[i] = yo + depth * (1 - u * u); }
}
inline float mouthSdf(const MouthShape &M, float x, float y, float th) {
  if (M.type == 1) return sdPoly(x, y, M.xs, M.ys, M.n, th);
  float d;
  if (M.type == 2) d = fabsf(sdEllipseA(x - M.ex, y - M.ey, M.rx, M.ry)) - 1.7f;
  else if (M.type == 5) d = fabsf(sdRoundBox(x - M.ex, y - M.ey, M.rx, M.ry, 5)) - 1.7f;
  else {
    float e = sdEllipseA(x - M.ex, y - M.ey, M.rx, M.ry);     // ۳: نیمه‌ي پایین بیضی (D خوابیده)
    float sh = e > (M.ey - y) ? e : (M.ey - y);
    d = fabsf(sh) - 1.7f;
  }
  if (M.n > 1) { float d2 = sdPoly(x, y, M.xs, M.ys, M.n, 1.6f); if (d2 < d) d = d2; }
  return d;
}

// برچسب دهان را (اگر کلیدش عوض شده) بساز و در (cx , sy0) بچسبان
inline void neonMouth(const MouthShape &M, int keyK, int q, float cx, float sy0, const RGBf &glowC) {
  if (M.type == 0) return;
  const float mth = 2.3f;
  if (nmKeyK != keyK || nmKeyQ != q || !nmBuf) {
    float mx0 = 1e9f, mx1 = -1e9f, my0 = 1e9f, my1 = -1e9f;
    auto inc = [&](float x0, float y0, float x1, float y1) {
      if (x0 < mx0) mx0 = x0;
      if (x1 > mx1) mx1 = x1;
      if (y0 < my0) my0 = y0;
      if (y1 > my1) my1 = y1;
    };
    for (int i = 0; i < M.n; i++) inc(M.xs[i], M.ys[i], M.xs[i], M.ys[i]);
    if (M.type != 1) inc(M.ex - M.rx, (M.type == 3 ? M.ey : M.ey - M.ry), M.ex + M.rx, M.ey + M.ry);
    if (M.fill) inc(M.fx - M.frx, M.fy - M.fry, M.fx + M.frx, M.fy + M.fry);
    int pad = (int)(mth + NE_GLOW) + 1;
    nmOX = -(int)floorf(mx0) + pad; nmOY = -(int)floorf(my0) + pad;
    nmW = (int)ceilf(mx1) + nmOX + pad; nmH = (int)ceilf(my1) + nmOY + pad;
    if (nmW * nmH > nmCap) {
      if (nmBuf) free(nmBuf);
      nmBuf = (uint16_t *)malloc((size_t)nmW * nmH * 2);
      nmCap = nmBuf ? nmW * nmH : 0;
    }
    if (!nmBuf) return;
    // جدول درخشش (۴ پله در هر پیکسل فاصله) — به جای حساب رنگ برای هر پیکسل
    uint16_t glut[81];                       // NE_GLOW (۲۰) × ۴ + ۱
    for (int i = 0; i <= 80; i++) {
      float k = 1 - (i + 0.5f) * 0.25f * (1.0f / NE_GLOW); if (k < 0) k = 0; k = k * k * k * 0.85f;
      glut[i] = neon565(glowC.r * k, glowC.g * k, glowC.b * k);
    }
    const uint16_t lineC = neon565(NE_LINE.r, NE_LINE.g, NE_LINE.b);
    uint16_t *p = nmBuf;
    for (int sy = 0; sy < nmH; sy++) {
      float y = sy + 0.5f - nmOY;
      for (int sx = 0; sx < nmW; sx++) {
        float x = sx + 0.5f - nmOX;
        float d = mouthSdf(M, x, y, mth);
        uint16_t c = 0;
        if (d <= 0) c = lineC;
        else if (d < NE_GLOW) c = glut[(int)(d * 4)];
        if (M.fill) {                        // زبان صورتی
          float df = sdEllipseA(x - M.fx, y - M.fy, M.frx, M.fry);
          uint16_t c2 = 0;
          if (df <= 0) { float tt = (y - M.fy + M.fry) / (2 * M.fry); c2 = neon565(255, 70 + 90 * tt, 150 + 60 * tt); }
          else if (df < 10) { float k = 1 - df / 10; k = k * k * 0.7f; c2 = neon565(255 * k, 40 * k, 140 * k); }
          if (df <= 0) c = c2; else c = maxBlend(c, c2);
        }
        *p++ = c;
      }
    }
    nmKeyK = keyK; nmKeyQ = q;
  }
  blitBuf(nmBuf, nmW, nmH, (int)lroundf(cx) - nmOX, (int)lroundf(sy0) - nmOY, false);
}

// ---------- غذاها (۶ نوع) ----------
#ifndef MOOCHI_SDSEG
#define MOOCHI_SDSEG
inline float sdSeg(float px, float py, float ax, float ay, float bx, float by, float r) {
  float vx = bx - ax, vy = by - ay, wx = px - ax, wy = py - ay;
  float L = vx * vx + vy * vy;
  float t = L > 0 ? (wx * vx + wy * vy) / L : 0;
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  float dx = wx - vx * t, dy = wy - vy * t;
  return sqrtf(dx * dx + dy * dy) - r;
}
#endif
const char *FOOD_FA[6] = {"سیب", "کلوچه", "دونات", "توت‌فرنگی", "هندوانه", "هویج"};

inline float circ(float x, float y, float cx, float cy, float r) {
  float a = x - cx, b = y - cy; return sqrtf(a * a + b * b) - r;
}
// رنگ و فاصله‌ي یک پیکسل از غذا (مختصات محلی ، شعاع حدود ۱۲) ؛ bites گاز از سمت چپ (دهان)
inline void foodPix(int type, float x, float y, int bites, RGBf &col, float &d, RGBf &glw) {
  float len = sqrtf(x * x + y * y);
  switch (type) {
    case 0: {                                           // سیب
      // ۱۲ مهر: نسخه‌ي قبل گرد بود و «گوجه‌فرنگی» دیده شد. سیب: بالا پهن با فرورفتگی وسط ،
      // پایین باریک‌تر ، دُم قهوه‌ای بلند و برگ بزرگ.
      float body = fminf(circ(x, y, -4.2f, -1, 8.8f), circ(x, y, 4.2f, -1, 8.8f));
      body = fminf(body, circ(x, y, 0, 3.5f, 8.2f));                       // پایینِ باریک‌تر
      body = fmaxf(body, 3.2f - sqrtf(x * x + (y + 10.5f) * (y + 10.5f))); // فرورفتگی بالا
      float stem = sdSeg(x, y, 0, -7, 2.2f, -15.5f, 1.5f);
      float lu = (x - 6.5f) * 0.85f + (y + 13) * 0.53f, lv = -(x - 6.5f) * 0.53f + (y + 13) * 0.85f;
      float leaf = sdEllipseA(lu, lv, 5.8f, 2.6f);
      d = fminf(body, fminf(stem, leaf));
      glw = {255, 40, 70};
      if (leaf <= 0) { col = {90, 235, 110}; if (fabsf(lv) < 0.5f) col = {50, 170, 70}; }
      else if (stem <= 0) col = {165, 95, 45};
      else {
        float tt = (y + 9) / 20; if (tt < 0) tt = 0; if (tt > 1) tt = 1;
        col = mixc({255, 70, 90}, {190, 15, 50}, tt);
        if (sdEllipseA(x + 5, y + 2, 1.8f, 3.2f) <= 0) col = {255, 205, 215};   // برق کشیده
      }
      break;
    }
    case 1: {                                           // کلوچه
      float a = atan2f(y, x);
      d = len - (12 + 0.7f * sinf(a * 8));
      glw = {255, 160, 60};
      col = {230, 170, 95};
      const float cxs[6] = {-5, 3, 6, -2, -6, 1}, cys[6] = {-4, -6, 2, 1, 5, 7};
      for (int i = 0; i < 6; i++) if (circ(x, y, cxs[i], cys[i], 1.9f) <= 0) col = {105, 55, 25};
      break;
    }
    case 2: {                                           // دونات
      d = fmaxf(len - 12, 4.5f - len);
      glw = {255, 80, 190};
      float a = atan2f(y, x);
      if (len > 5.6f && len < 10.2f + 0.8f * sinf(a * 7)) {
        col = {255, 125, 205};
        const RGBf SP[3] = {{120, 220, 255}, {255, 240, 120}, {140, 255, 150}};
        for (int i = 0; i < 9; i++) {
          float aa = i * 0.698f, rr = 8;
          float sx = rr * cosf(aa), sy = rr * sinf(aa);
          if (sdSeg(x, y, sx - 1.2f * sinf(aa * 3), sy - 1.2f * cosf(aa * 3), sx + 1.2f * sinf(aa * 3), sy + 1.2f * cosf(aa * 3), 0.7f) <= 0) col = SP[i % 3];
        }
      } else col = {230, 170, 100};
      break;
    }
    case 3: {                                           // توت‌فرنگی
      float body = sdHeart(x, y - 1, 11);
      float lf = fminf(sdEllipseA(x + 4, y + 9, 3.5f, 1.8f), fminf(sdEllipseA(x - 4, y + 9, 3.5f, 1.8f), sdEllipseA(x, y + 10.5f, 2, 3)));
      d = fminf(body, lf);
      glw = {255, 40, 80};
      if (lf <= 0) col = {90, 230, 100};
      else {
        col = {255, 55, 85};
        float gx = fmodf(x + 30, 4.5f) - 2.25f, gy = fmodf(y + 30 + ((int)((x + 30) / 4.5f) % 2) * 2.2f, 4.5f) - 2.25f;
        if (gx * gx + gy * gy < 0.8f) col = {255, 230, 130};
      }
      break;
    }
    case 4: {                                           // هندوانه (برش نیم‌دایره)
      d = fmaxf(len - 12.5f, -(y + 1));
      glw = {255, 60, 100};
      if (len > 10.5f) col = {70, 210, 90};
      else if (len > 9.3f) col = {230, 255, 210};
      else {
        col = {255, 75, 115};
        const float sxs[5] = {-5, -1.5f, 2.5f, 5.5f, 0.5f}, sys[5] = {3, 6, 4, 1, 1.5f};
        for (int i = 0; i < 5; i++) if (sdEllipseA(x - sxs[i], y - sys[i], 0.9f, 1.4f) <= 0) col = {60, 20, 30};
      }
      break;
    }
    default: {                                          // هویج (نوک رو به دهان)
      float xc = x < -11 ? -11 : (x > 8 ? 8 : x);
      float body = sqrtf((x - xc) * (x - xc) + y * y) - (1.5f + 4 * (xc + 11) / 19);
      float lv = fminf(sdSeg(x, y, 8, 0, 15, -6, 1.3f), fminf(sdSeg(x, y, 8, 0, 16.5f, 0, 1.3f), sdSeg(x, y, 8, 0, 15, 6, 1.3f)));
      d = fminf(body, lv);
      glw = {255, 130, 30};
      if (lv <= 0 && body > 0) col = {90, 230, 100};
      else {
        col = {255, 145, 35};
        for (int i = 0; i < 3; i++) { float lx = -5 + i * 4.5f; if (fabsf(x - lx) < 0.6f && fabsf(y) < 2.5f + i) col = {200, 90, 20}; }
      }
      break;
    }
  }
  // گازها: دایره‌هایی از سمت چپ (دهان) که از غذا بریده می‌شوند
  for (int k = 0; k < bites && k < 3; k++) {
    float bx = -13 + k * 6.5f, by = -2 + 3 * (k & 1);
    float db = 7 - sqrtf((x - bx) * (x - bx) + (y - by) * (y - by));
    if (db > d) d = db;
  }
}

// ---------- کشیدن کل صورت نئونی در نوار جاری ----------
inline void neonDraw(const NeonParams &P0) {
  for (int i = 0; i < PHYS_W * bandH; i++) bandBuf[i] = 0;

  NeonParams P = P0;
  neonGaze(P);
  const NeonKind K = P.kind;
  const float t = P.t;
  float cx = NE_CX + P.fx, cy = NE_CY + P.fy;
  float hw = NE_HW, hh = NE_HH;
  float open = P.open < 0 ? 0 : (P.open > 1 ? 1 : P.open);
  if (K == NK_SURPRISE) { hw *= 1.12f; hh *= 1.14f; }
  if (K == NK_SLEEPY)   { open *= 0.38f; }
  if (K == NK_ANGRY)    { hh *= 0.92f; }
  float eh = hh * open;
  bool closed = (K == NK_ASLEEP || K == NK_YAWN) || eh < 4;
  if (eh < hw * 0.18f) eh = hw * 0.18f;
  // پلک را پله‌پله کن تا برچسب در هر فریمِ پلک زدن بی‌دلیل دوباره ساخته نشود
  eh = roundf(eh * 2) * 0.5f;
  float shine = open > 0.55f ? 1.0f : roundf(open / 0.55f * 8) / 8;

  float exL = cx - NE_DX + P.gx, exR = cx + NE_DX + P.gx, ey = cy + P.gy;
  float sy0 = NE_SMILE_Y + P.fy;
  const RGBf &glowC = (K == NK_ANGRY) ? NE_ANGRY_GLOW : NE_GLOWC;

  // ---------- دهان (برچسب) ----------
  // فقط وقتی حالت عوض شود (یا دهانِ متحرک یک پله جلو برود) دوباره ساخته می‌شود.
  {
    bool anim = (K == NK_DIZZY || K == NK_YAWN || K == NK_FEED);
    int q = anim ? ((int)(t * 12) % 1000) : 0;     // ۱۲ پله در ثانیه
    if (K == NK_FEED && t < 0.8f) q = 1000;        // «آآآ» — دهان باز منتظر غذا
    float tq = q / 12.0f;
    MouthShape M;
    switch (K) {
      case NK_SURPRISE: M.type = 2; M.ex = 0; M.ey = 2; M.rx = 5.5f; M.ry = 5.5f; break;
      case NK_SAD: case NK_CRY: mouthSmile(M, NE_SMILE_HW, -6, 6); break;
      case NK_ANGRY:    mouthSmile(M, 13, -4, 5); break;
      case NK_CURIOUS:  mouthSmile(M, 7, 1.5f, 3); break;
      case NK_PET:      mouthSmile(M, 24, 9, 0); break;
      case NK_DIZZY:
        M.type = 1; M.n = 13;
        for (int i = 0; i < 13; i++) { float u = -1 + 2.0f * i / 12; M.xs[i] = u * 18; M.ys[i] = 3 + 3.2f * sinf(u * 7.0f + q * 0.583f); }
        break;
      case NK_THINK:    M.type = 1; M.n = 2; M.xs[0] = 2; M.ys[0] = 6; M.xs[1] = 15; M.ys[1] = 3; break;
      case NK_EYEROLL:  M.type = 1; M.n = 2; M.xs[0] = -10; M.ys[0] = 7; M.xs[1] = 10; M.ys[1] = 5; break;
      case NK_SHY:
        M.type = 1; M.n = 9;
        for (int i = 0; i < 9; i++) { float u = -1 + 2.0f * i / 8; M.xs[i] = u * 9; M.ys[i] = 5 + 1.6f * sinf(u * 9.42f); }
        break;
      case NK_EXCITED:  M.type = 3; M.ex = 0; M.ey = -1; M.rx = 13; M.ry = 12; break;
      case NK_YAWN: {
        float tt = tq / 2.4f; if (tt > 1) tt = 1;
        float o = sinf(tt * 3.1416f);
        M.type = 2; M.ex = 0; M.rx = 6 + 3 * o; M.ry = 3 + 11 * o; M.ey = 6 + 6 * o;
        break;
      }
      case NK_FEED: {
        if (q == 1000) { M.type = 2; M.ex = 0; M.ey = 7; M.rx = 8; M.ry = 9; break; }
        float o = 0.5f + 0.5f * sinf(tq * 6.2832f * 2.5f);
        M.type = 2; M.ex = 0; M.ey = 6; M.rx = 9; M.ry = 2 + 6 * o;
        break;
      }
      case NK_LISTEN:   mouthSmile(M, 12, 5, 0); break;
      case NK_KISS: {
        const float kx[7] = {0, 4.5f, 4.5f, 1, 4.5f, 4.5f, 0}, ky[7] = {-6, -5, -2, 0, 2, 5, 6};
        M.type = 1; M.n = 7;
        for (int i = 0; i < 7; i++) { M.xs[i] = kx[i] + 2; M.ys[i] = ky[i] + 6; }
        break;
      }
      case NK_HEARTHAND: M.type = 0; break;
      default:          mouthSmile(M, NE_SMILE_HW, NE_SMILE_D, 0); break;
    }
    neonMouth(M, (int)K, q, cx, sy0, glowC);
  }

  // ---------- چشم‌ها (برچسب) ----------
  EyeCfg E;
  E.K = K; E.hw = hw; E.eh = eh; E.shine = shine; E.closed = closed;
  E.top  = (K == NK_ANGRY) ? &NE_ANGRY_TOP : &NE_TOP;
  E.glow = &glowC;
  E.lidK = 0; E.lidY = -1e9f;
  if (K == NK_SAD || K == NK_CRY) { E.lidK = 0.45f;  E.lidY = -eh * 0.30f; }   // چشم راست
  if (K == NK_ANGRY)              { E.lidK = -0.55f; E.lidY = -eh * 0.25f; }
  if (K == NK_THINK)   E.lidY = -eh * 0.55f;       // پلک صاف: کمی از بالا بسته
  if (K == NK_SHY)     E.lidY = -eh * 0.35f;
  if (K == NK_EYEROLL) E.lidY = -eh * 0.05f;       // نیمه‌بسته (حوصله‌سررفته)
  if (K == NK_PET || K == NK_HEARTHAND || K == NK_LISTEN) E.K = NK_HAPPY;
  if (K == NK_FEED) E.K = (t < 0.8f) ? NK_NORMAL : NK_HAPPY;    // اول به غذا نگاه می‌کند ، بعد با لذت می‌جود
  if (K == NK_LOVE || K == NK_EXCITED) {           // قلب/ستاره که می‌تپد (۰٫۵ پیکسل پله)
    float pul = (K == NK_LOVE) ? 1 + 0.08f * sinf(t * 7.0f) : 1 + 0.06f * sinf(t * 9.0f);
    E.hw = roundf(hw * pul * 2) * 0.5f; E.eh = hw; E.shine = 1; E.closed = false;
  }
  E.spin = 0;                   // سرگیجه: برچسب ثابت ، موقع کپی چرخانده می‌شود
  if (K == NK_DIZZY) { E.eh = hw; E.shine = 1; E.closed = false; }   // پلک زدن برچسب مارپیچ را از نو نسازد
  bool mirrorLeft = (K == NK_SAD || K == NK_CRY || K == NK_ANGRY || K == NK_DIZZY);

  if (K == NK_DIZZY && buildSprite(spr[0], E)) {
    float ang = t * 5.0f;
    blitRot(spr[0], exR, ey, ang, false);
    blitRot(spr[0], exL, ey, ang, true);
  } else if (K == NK_CURIOUS) {                // راست درشت‌تر ، چپ کوچک‌تر — هر دو برچسب
    EyeCfg Er = E; Er.hw = hw * 1.14f; Er.eh = roundf(eh * 1.14f * 2) * 0.5f;
    EyeCfg El = E; El.hw = hw * 0.86f; El.eh = roundf(eh * 0.86f * 2) * 0.5f;
    if (buildSprite(spr[0], Er)) blitSprite(spr[0], exR, ey, false); else directEye(Er, exR, ey, 1);
    if (buildSprite(spr[1], El)) blitSprite(spr[1], exL, ey, false); else directEye(El, exL, ey, 1);
  } else if (K == NK_KISS) {                   // چشمک: چپ بسته
    EyeCfg El = E; El.closed = true;
    if (buildSprite(spr[0], E))  blitSprite(spr[0], exR, ey, false); else directEye(E, exR, ey, 1);
    if (buildSprite(spr[1], El)) blitSprite(spr[1], exL, ey, false); else directEye(El, exL, ey, 1);
  } else if (buildSprite(spr[0], E)) {
    blitSprite(spr[0], exR, ey, false);
    blitSprite(spr[0], exL, ey, mirrorLeft);
  } else {
    directEye(E, exR, ey, 1);
    EyeCfg El = E; El.lidK = -E.lidK;
    directEye(El, exL, ey, 1);
  }

  // ---------- گونه‌ي صورتی ----------
  if (K == NK_PET || K == NK_SHY || K == NK_FEED || K == NK_LOVE) {
    float br = (K == NK_SHY) ? 1.0f : 0.75f;
    for (int e = 0; e < 2; e++) {
      float bx = cx + (e ? 1 : -1) * (NE_DX + 8), by = cy + 44;
      glowDraw((int)bx - 26, (int)by - 18, (int)bx + 26, (int)by + 18,
               [&](float x, float y) { return sdEllipseA(x - bx, y - by, 12, 5.5f); },
               NE_BLUSH, NE_BLUSHG, 10, br);
    }
  }

  // ---------- فکر: «...» بالا-راست که یکی‌یکی می‌آیند ----------
  if (K == NK_THINK) {
    int nd = ((int)(t * 2.2f)) % 4;
#if DISPLAY_ROUND
    const float dx[3] = {50, 61, 73}, dy[3] = {-46, -58, -70}, dr[3] = {3.2f, 4.2f, 5.4f};   // داخل دایره
#else
    const float dx[3] = {66, 80, 96}, dy[3] = {-60, -76, -92}, dr[3] = {3.2f, 4.2f, 5.4f};
#endif
    for (int i = 0; i < nd && i < 3; i++) {
      float px = cx + dx[i], py = cy + dy[i], r = dr[i];
      glowDraw((int)(px - r - 10), (int)(py - r - 10), (int)(px + r + 10), (int)(py + r + 10),
               [&](float x, float y) { float a = x - px, b = y - py; return sqrtf(a * a + b * b) - r; },
               NE_LINE, NE_GLOWC, 10);
    }
  }

  // ---------- گوش دادن به موسیقی: هدفون + نت ----------
  if (K == NK_LISTEN) {
    // کمانِ هدفون روی سر (فقط ردیف‌های حلقه ، نه کل جعبه)
    // ۱۲ مهر: نسخه‌ي قبل نازک بود و خوب دیده نمی‌شد — حالا کمان پهن‌تر با سایه‌ي لوله‌ای
    // و گوشی‌های بزرگ‌تر با بالشتک.
    const RGBf HB = {255, 150, 230}, HD = {190, 40, 170}, HG = {255, 40, 190};
    float ox = cx, oy = cy + 8, R = 86, TH = 6.0f, GW = 11;
    float rIn = R - TH - GW, rOut = R + TH + GW;
    int Ya = (int)(oy - rOut), Yb = (int)oy;
    if (Ya < bandY0) Ya = bandY0;
    if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
    for (int Y = Ya; Y <= Yb; Y++) {
      float b = Y + 0.5f - oy;
      if (rOut * rOut - b * b <= 0) continue;
      float xo = sqrtf(rOut * rOut - b * b), xi = (rIn * rIn - b * b) > 0 ? sqrtf(rIn * rIn - b * b) : 0;
      uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
      for (int side = 0; side < 2; side++) {
        int Xa = side ? (int)(ox + xi) : (int)(ox - xo), Xb = side ? (int)(ox + xo) : (int)(ox - xi);
        if (Xa < 0) Xa = 0;
        if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
        for (int X = Xa; X <= Xb; X++) {
          float a = X + 0.5f - ox, rr = sqrtf(a * a + b * b), u = (rr - R) / TH;   // −۱ داخل … +۱ بیرون
          float d = fabsf(rr - R) - TH;
          uint16_t c = 0;
          if (d <= 0) {
            RGBf cc = mixc(HB, HD, (u + 1) * 0.5f);                 // بیرون روشن ، داخل تیره (لوله)
            if (d > -1.4f) cc = mixc(cc, {255, 220, 250}, 1 + d / 1.4f);   // لبه‌ي روشن
            c = neon565(cc.r, cc.g, cc.b);
          } else if (d < GW) { float k = 1 - d / GW; k = k * k * k * 0.85f; c = neon565(HG.r * k, HG.g * k, HG.b * k); }
          if (c) row[X] = (d <= 0) ? c : maxBlend(row[X], c);
        }
      }
    }
    // دو گوشی: بدنه‌ي پر با شیب + بالشتک سمت سر + لبه‌ي نئونی
    for (int sd = -1; sd <= 1; sd += 2) {
      float ex = ox + sd * (R + 2), ey2 = oy + 6;
      int Xa = (int)ex - 28, Xb = (int)ex + 28, Ya2 = (int)ey2 - 38, Yb2 = (int)ey2 + 38;
      if (Ya2 < bandY0) Ya2 = bandY0;
      if (Yb2 > bandY0 + bandH - 1) Yb2 = bandY0 + bandH - 1;
      if (Xa < 0) Xa = 0;
      if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
      for (int Y = Ya2; Y <= Yb2; Y++) {
        uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
        for (int X = Xa; X <= Xb; X++) {
          float lx = X + 0.5f - ex, ly = Y + 0.5f - ey2;
          float d = sdRoundBox(lx, ly, 12, 22, 10);
          uint16_t c = 0;
          if (d <= 0) {
            float tt = (ly + 22) / 44;
            RGBf cc = mixc({235, 70, 200}, {130, 20, 120}, tt);
            float pad = sdRoundBox(lx + sd * 7, ly, 4, 17, 4);             // بالشتک سمت سر
            if (pad <= 0) cc = mixc({90, 15, 85}, {60, 10, 60}, tt);
            if (d > -1.8f) cc = mixc(cc, {255, 200, 245}, 1 + d / 1.8f);
            c = neon565(cc.r, cc.g, cc.b);
          } else if (d < 11) { float k = 1 - d / 11; k = k * k * k * 0.85f; c = neon565(HG.r * k, HG.g * k, HG.b * k); }
          if (c) row[X] = (d <= 0) ? c : maxBlend(row[X], c);
        }
      }
    }
    // نت‌های موسیقی که از دو طرف بالا می‌روند و محو می‌شوند
    for (int i = 0; i < 2; i++) {
      float ph = fmodf(t * 0.65f + i * 0.5f, 1.0f);
      int sd = i ? 1 : -1;
      float nx = ox + sd * (R - 14 - ph * 22) + 5 * sinf(ph * 9), ny = oy - 34 - ph * 58;
      const RGBf nc = i ? RGBf{255, 235, 120} : RGBf{130, 230, 255};
      const RGBf ng = i ? RGBf{255, 170, 40}  : RGBf{40, 150, 255};
      float br = ph < 0.15f ? ph / 0.15f : (1 - ph) / 0.85f;
      glowDraw((int)nx - 14, (int)ny - 28, (int)nx + 22, (int)ny + 14,
               [&](float x, float y) {
                 float lx = x - nx, ly = y - ny;
                 float u = lx * 0.92f + ly * 0.39f, v = -lx * 0.39f + ly * 0.92f;   // سرِ کج نت
                 float dh = sdEllipseA(u, v, 4.4f, 3.2f);
                 float ds = sdSeg(lx, ly, 3.8f, -1, 3.8f, -16, 1.1f);
                 float df = sdSeg(lx, ly, 3.8f, -16, 9, -11, 1.2f);
                 return fminf(dh, fminf(ds, df));
               }, nc, ng, 9, br);
    }
  }

  // ---------- غذا خوردن: غذا می‌آید ، سه گاز ، تمام ----------
  if (K == NK_FEED && t < 2.2f) {
    float fx, fy;
    if (t < 0.8f) {
      float e = t / 0.8f; e = 1 - (1 - e) * (1 - e);
      fx = cx + 82 + (26 - 82) * e; fy = sy0 - 36 + 46 * e;
    } else { fx = cx + 26; fy = sy0 + 10; }
    int bites = t < 1.0f ? 0 : (t < 1.4f ? 1 : (t < 1.8f ? 2 : 3));
    fx -= bites * 3;
    int food = ((P.food % 6) + 6) % 6;
    const float FS = 1.6f;                              // بزرگی غذا
    int Ya = (int)fy - 42, Yb = (int)fy + 40, Xa = (int)fx - 40, Xb = (int)fx + 48;
    if (Ya < bandY0) Ya = bandY0;
    if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
    if (Xa < 0) Xa = 0;
    if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
    for (int Y = Ya; Y <= Yb; Y++) {
      uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
      for (int X = Xa; X <= Xb; X++) {
        RGBf col, glw; float d;
        foodPix(food, (X + 0.5f - fx) / FS, (Y + 0.5f - fy) / FS, bites, col, d, glw);
        d *= FS;
        uint16_t c = 0;
        if (d <= 0) c = neon565(col.r, col.g, col.b);
        else if (d < 10) { float k = 1 - d / 10; k = k * k * k * 0.8f; c = neon565(glw.r * k, glw.g * k, glw.b * k); }
        if (c) row[X] = (d <= 0) ? c : maxBlend(row[X], c);
      }
    }
  }

  // ---------- بوس: قلب کوچکی که از دهان بالا می‌رود و کم‌رنگ می‌شود ----------
  if (K == NK_KISS) {
    float ph = fmodf(t, 1.6f) / 1.6f;
    // از وسط بین دو چشم بالا می‌رود (از روی چشم رد نشود)
    float px = cx + 4 + 6 * sinf(ph * 6.0f), py = sy0 - 12 - ph * 92, hs = 6 + 3 * ph;
    glowDraw((int)(px - hs - 14), (int)(py - hs - 14), (int)(px + hs + 14), (int)(py + hs + 14),
             [&](float x, float y) { return sdHeart(x - px, y - py, hs); },
             NE_LINE, NE_GLOWC, 12, 1 - ph * 0.8f);
  }

  // ---------- قلب بزرگ پایین (حالت «قلب») ----------
  if (K == NK_HEARTHAND) {
    float hs = roundf(20 * (1 + 0.09f * sinf(t * 6.5f)) * 2) * 0.5f;
    float px = cx, py = sy0 + 14;
    int Ya = (int)(py - hs - NE_GLOW - 4), Yb = (int)(py + hs + NE_GLOW + 4);
    int Xa = (int)(px - hs * 1.1f - NE_GLOW), Xb = (int)(px + hs * 1.1f + NE_GLOW);
    if (Ya < bandY0) Ya = bandY0;
    if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
    if (Xa < 0) Xa = 0;
    if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
    for (int Y = Ya; Y <= Yb; Y++) {
      uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
      for (int X = Xa; X <= Xb; X++) {
        float lx = X + 0.5f - px, ly = Y + 0.5f - py;
        float d = sdHeart(lx, ly, hs);
        uint16_t c = 0;
        if (d <= 0) { RGBf f = shapeFill(lx, ly, d, hs); c = neon565(f.r, f.g, f.b); }
        else if (d < NE_GLOW) {
          float k = 1 - d / NE_GLOW; k = k * k * k * 0.85f;
          c = neon565(NE_GLOWC.r * k, NE_GLOWC.g * k, NE_GLOWC.b * k);
        }
        if (c) row[X] = maxBlend(row[X], c);
      }
    }
  }

  // ---------- رنگین‌کمان ----------
  if (K == NK_RAINBOW) {
    static const RGBf RB[5] = {{255, 60, 110}, {255, 150, 40}, {255, 230, 70}, {70, 235, 130}, {80, 160, 255}};
    const float R0 = 82, STEP = 6, TH = 2.0f, GW = 7;
    float ox = cx, oy = cy + 20;
    float rIn = R0 - TH - GW, rOut = R0 + 4 * STEP + TH + GW;
    int Ya = (int)(oy - rOut), Yb = (int)oy + 4;
    if (Ya < bandY0) Ya = bandY0;
    if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
    for (int Y = Ya; Y <= Yb; Y++) {
      float b = Y + 0.5f - oy;
      if (rOut * rOut - b * b <= 0) continue;
      float xo = sqrtf(rOut * rOut - b * b), xi = (rIn * rIn - b * b) > 0 ? sqrtf(rIn * rIn - b * b) : 0;
      uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
      for (int side = 0; side < 2; side++) {
        int Xa = side ? (int)(ox + xi) : (int)(ox - xo), Xb = side ? (int)(ox + xo) : (int)(ox - xi);
        if (Xa < 0) Xa = 0;
        if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
        for (int X = Xa; X <= Xb; X++) {
          float a = X + 0.5f - ox, r = sqrtf(a * a + b * b);
          int i = (int)lroundf((r - R0) / STEP);
          if (i < 0) i = 0;
          if (i > 4) i = 4;
          float d = fabsf(r - (R0 + i * STEP)) - TH;
          if (b > 0 && b > d) d = b;                       // دو سرِ پایین صاف بریده
          uint16_t c = 0;
          if (d <= 0) { RGBf w = mixc(RB[i], {255, 255, 255}, 0.35f); c = neon565(w.r, w.g, w.b); }
          else if (d < GW) { float k = 1 - d / GW; k = k * k * 0.7f; c = neon565(RB[i].r * k, RB[i].g * k, RB[i].b * k); }
          if (c) row[X] = maxBlend(row[X], c);
        }
      }
    }
  }

  // ---------- اشک ----------
  if (K == NK_CRY) {
    for (int e = 0; e < 2; e++) {
      float ex = e ? exR : exL;
      float ph = fmodf(P.t * 38.0f + e * 23.0f, 52.0f);
      float tx0 = ex + (e ? 1 : -1) * hw * 0.55f, ty0 = ey + eh + 4 + ph;
      int Ya = (int)(ty0 - 9 - 10), Yb = (int)(ty0 + 4 + 10);
      int Xa = (int)(tx0 - 14), Xb = (int)(tx0 + 14);
      if (Ya < bandY0) Ya = bandY0;
      if (Yb > bandY0 + bandH - 1) Yb = bandY0 + bandH - 1;
      if (Xa < 0) Xa = 0;
      if (Xb > PHYS_W - 1) Xb = PHYS_W - 1;
      for (int Y = Ya; Y <= Yb; Y++) {
        uint16_t *row = bandBuf + (Y - bandY0) * PHYS_W;
        for (int X = Xa; X <= Xb; X++) {
          float tx = X + 0.5f - tx0, ty = Y + 0.5f - ty0;
          float d = sqrtf(tx * tx + ty * ty) - 3.8f;
          if (ty < 0 && ty > -9) { float w = 3.8f * (1 + ty / 9.0f); float dt = fabsf(tx) - w; if (dt < d) d = dt; }
          uint16_t c = 0;
          if (d <= 0) c = neon565(NE_TEAR.r, NE_TEAR.g, NE_TEAR.b);
          else if (d < 10) { float k = 1 - d / 10; k = k * k * 0.8f; c = neon565(NE_TEARG.r * k, NE_TEARG.g * k, NE_TEARG.b * k); }
          if (c) row[X] = maxBlend(row[X], c);
        }
      }
    }
  }
}
