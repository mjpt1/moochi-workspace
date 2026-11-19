#pragma once
// =====================================================
//   MOOCHI — دست‌های نئونی (۱۲ مهر ۱۴۰۵ ، بعد از تست)
//
//   مهسن: «دست زدن کند است و روان نیست» و «باید پشت دست‌ها پیدا باشد ،
//   ولی انگار جلوی دست پیداست».
//
//   ۱. کندی: دست‌های قبلی هر فریم از ده‌ها شکل کوچک × ۴ گذر درخشش ساخته
//      می‌شدند. حالا هر ژست یک بار در یک «برچسب» ساخته می‌شود و هر فریم
//      فقط چرخانده و جابه‌جا کپی می‌شود (مثل مارپیچ سرگیجه).
//   ۲. پشت دست: در نمای پشت دست ، شست **بیرون** است (دست راستِ صفحه:
//      شست سمت راست) ، ناخن‌ها سر انگشت‌ها دیده می‌شوند و روی بند انگشت‌ها
//      خط‌های کوچک بند است. در کد قدیمی شست داخل بود = نمای کف دست.
//
//   همه‌ي ژست‌ها برای دستِ سمت راست صفحه ، انگشت‌ها رو به بالا کشیده شده‌اند.
//   دست چپ = آینه.
// =====================================================

enum HandPose { HP_OPEN, HP_FLAT, HP_FIST, HP_THUMBUP, HP_VICTORY, HP_POINT };

// sdSeg در moochi_neon.h تعریف شده

// اجزای یک ژست (واحد: پیکسل واقعی در اندازه‌ي ۱)
struct HandGeo {
  float W = 13, H = 12, R = 6;           // کفِ دست (نصف پهنا/ارتفاع ، شعاع گوشه)
  int nf = 0;                            // انگشت‌ها (با شست)
  float ax[6], ay[6], bx[6], by[6], r[6];
  bool thumb[6], nail[6];
};

inline void handFinger(HandGeo &G, float x, float y, float len, float ang, float r, bool nail = true, bool th = false) {
  int i = G.nf++;
  G.ax[i] = x; G.ay[i] = y;
  G.bx[i] = x + len * sinf(ang); G.by[i] = y - len * cosf(ang);   // ang از رو به بالا ، + = به راست
  G.r[i] = r; G.nail[i] = nail; G.thumb[i] = th;
}

inline void handGeoFor(HandPose p, HandGeo &G) {
  const float FX[4] = {-8.4f, -2.8f, 2.8f, 8.4f};     // کوچک ، حلقه ، وسط ، اشاره (اشاره کنار شست)
  float fy = -G.H + 3;
  switch (p) {
    case HP_OPEN: {
      const float L[4] = {13, 17, 18, 15}, A[4] = {-0.22f, -0.08f, 0.06f, 0.2f};
      for (int k = 0; k < 4; k++) handFinger(G, FX[k], fy, L[k], A[k], 3.4f);
      handFinger(G, G.W - 2, 6, 13, 0.95f, 4.2f, true, true);
      break;
    }
    case HP_FLAT: {
      const float L[4] = {13, 17, 18, 15};
      for (int k = 0; k < 4; k++) handFinger(G, FX[k], fy, L[k], (k - 1.5f) * 0.02f, 3.3f);
      handFinger(G, G.W - 2, 6, 12, 0.45f, 4.0f, true, true);
      break;
    }
    case HP_FIST:
    case HP_THUMBUP:
    case HP_VICTORY:
    case HP_POINT: {
      G.R = 5;
      // بند انگشت‌های خوابیده: برجستگی‌های ردیف بالا (از پشت ، انگشت‌ها زیر مشت پنهان‌اند)
      for (int k = 0; k < 4; k++) {
        bool up = (p == HP_VICTORY && k >= 2) || (p == HP_POINT && k == 3);
        if (up) {
          float len = (k == 3) ? 19 : 18, ang = (p == HP_VICTORY) ? (k == 3 ? 0.26f : -0.12f) : 0.0f;
          handFinger(G, FX[k], fy, len, ang, 3.4f);
        } else {
          handFinger(G, FX[k], -G.H + 1.5f, 0.5f, 0, 3.7f, false);
        }
      }
      if (p == HP_THUMBUP) handFinger(G, G.W - 4, -G.H + 4, 15, 0.0f, 4.4f, true, true);
      else                 handFinger(G, G.W, 7, 10, -0.3f, 4.2f, false, true);      // شستِ خوابیده کنار مشت
      break;
    }
  }
}

const RGBf NH_HI   = {246, 238, 255};   // روشن (بالا)
const RGBf NH_LO   = {182, 160, 236};   // تیره‌تر (پایین)
const RGBf NH_RIM  = {255, 252, 255};
const RGBf NH_SEAM = { 78,  56, 130};   // شکاف بین انگشت‌ها
const RGBf NH_NAIL = {255, 212, 240};   // ناخن (فقط از پشت دست پیداست)
const RGBf NH_GLOW = {205, 140, 255};

// بیت ۰ آبی = «پیکسل توپر» (پوشاننده) ؛ درخشش این بیت را ندارد
inline uint16_t handSolid(const RGBf &c) {
  int R = c.r > 255 ? 255 : (int)c.r, G = c.g > 255 ? 255 : (int)c.g, B = c.b > 255 ? 255 : (int)c.b;
  uint16_t v = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | ((B >> 3) | 1);
  return (v >> 8) | (v << 8);
}
inline uint16_t handGlowPx(float k) {
  int R = (int)(NH_GLOW.r * k), G = (int)(NH_GLOW.g * k), B = (int)(NH_GLOW.b * k);
  uint16_t v = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | ((B >> 3) & 0x1E);
  return (v >> 8) | (v << 8);
}

struct HandSprite { uint16_t *buf = nullptr; int cap = 0, w = 0, h = 0, key = -1; };
static HandSprite hspr;

inline bool buildHand(HandPose p, float sc) {
  int key = (int)p * 1000 + (int)(sc * 100);
  if (hspr.buf && hspr.key == key) return true;
  HandGeo G; handGeoFor(p, G);
  const float GW = 12;                                  // پهنای درخشش
  int hx = (int)((G.W + 14) * sc + GW + 2), hy = (int)((G.H + 22) * sc + GW + 2);
  int w = hx * 2, h = hy * 2;
  if (w * h > hspr.cap) {
    if (hspr.buf) free(hspr.buf);
    hspr.buf = (uint16_t *)malloc((size_t)w * h * 2);
    hspr.cap = hspr.buf ? w * h : 0;
    if (!hspr.buf) { hspr.key = -1; return false; }
  }
  hspr.w = w; hspr.h = h;
  float top = -G.H - 20, bot = G.H + 10;
  uint16_t *o = hspr.buf;
  for (int sy = 0; sy < h; sy++) {
    float y = (sy + 0.5f - hy) / sc;
    for (int sx = 0; sx < w; sx++) {
      float x = (sx + 0.5f - hx) / sc;
      float dPalm = sdRoundBox(x, y, G.W, G.H, G.R);
      float dWr = sdSeg(x, y, 0, G.H - 3, 0, G.H + 4, 5.5f);   // مچ کوتاه
      float d = fminf(dPalm, dWr);
      float b1 = 1e9f, b2 = 1e9f; int i1 = -1, i2 = -1;
      for (int i = 0; i < G.nf; i++) {
        float di = sdSeg(x, y, G.ax[i], G.ay[i], G.bx[i], G.by[i], G.r[i]);
        if (di < b1) { b2 = b1; i2 = i1; b1 = di; i1 = i; } else if (di < b2) { b2 = di; i2 = i; }
      }
      d = fminf(d, b1);
      bool thumbNear = (i2 >= 0 && G.thumb[i2]);
      float dpx = d * sc;                               // فاصله به پیکسل
      uint16_t c = 0;
      if (dpx <= 0) {
        float tt = (y - top) / (bot - top); if (tt < 0) tt = 0; if (tt > 1) tt = 1;
        RGBf col = mixc(NH_HI, NH_LO, tt);
        if (dpx > -1.8f) col = mixc(col, NH_RIM, 1 + dpx / 1.8f);
        // شکاف بین دو انگشتِ چسبیده
        if (i1 >= 0 && b1 <= 0 && b2 * sc < 1.1f && b1 < dPalm + 0.5f && !G.thumb[i1] && !thumbNear) col = NH_SEAM;
        // ناخن سر انگشت
        if (i1 >= 0 && b1 <= 0 && G.nail[i1]) {
          float ux = G.bx[i1] - G.ax[i1], uy = G.by[i1] - G.ay[i1], L = sqrtf(ux * ux + uy * uy);
          if (L > 4) {
            ux /= L; uy /= L;
            float nx = G.bx[i1] - ux * 1.6f, ny = G.by[i1] - uy * 1.6f;
            float qx = x - nx, qy = y - ny;
            float along = qx * ux + qy * uy, across = -qx * uy + qy * ux;
            float rr = G.r[i1];
            if ((along * along) / (rr * rr * 0.55f) + (across * across) / (rr * rr * 0.30f) <= 1) col = NH_NAIL;
          }
        }
        // خط بندِ انگشت روی پشت دست (کوتاه ، افقی ، بالای کف)
        if (!(i1 >= 0 && b1 <= 0 && b1 < dPalm)) {
          for (int k = 0; k < G.nf; k++) {
            if (G.thumb[k]) continue;
            float kx = G.ax[k], ky = -G.H + 4.5f;
            if (fabsf(x - kx) < 2.0f && fabsf(y - ky) < 0.45f + 0.5f / sc) col = mixc(col, NH_SEAM, 0.45f);
          }
        }
        c = handSolid(col);
      } else if (dpx < GW) {
        float k = 1 - dpx / GW; k = k * k * k * 0.8f;
        c = handGlowPx(k);
      }
      *o++ = c;
    }
  }
  hspr.key = key;
  return true;
}

// کپی چرخیده: پیکسل توپر روی صورت را می‌پوشاند ، درخشش با بیشینه ترکیب می‌شود
inline void blitHand(float cx, float cy, float ang, bool mirror) {
  if (!hspr.buf || hspr.key < 0) return;
  int hx = hspr.w / 2, hy = hspr.h / 2;
  float c = cosf(ang), s = sinf(ang);
  // جعبه‌ي دقیقِ برچسبِ چرخیده (نه دایره‌ي دورش — ۲ برابر پیکسل کمتر)
  int ex = (int)(fabsf(c) * hx + fabsf(s) * hy) + 1, ey = (int)(fabsf(s) * hx + fabsf(c) * hy) + 1;
  int Ya = (int)cy - ey, Yb = (int)cy + ey, Xa = (int)cx - ex, Xb = (int)cx + ex;
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
      if ((unsigned)ix >= (unsigned)hspr.w || (unsigned)iy >= (unsigned)hspr.h) continue;
      uint16_t col = hspr.buf[iy * hspr.w + ix];
      if (!col) continue;
      if (col & 0x0100) row[X] = col;                  // بیت ۰ آبی (big-endian ← بایت دوم)
      else row[X] = maxBlend(row[X], col);
    }
  }
}

// یک دست: ang = چرخش (رادیان ، + = ساعت‌گرد) ، left = دستِ سمت چپ صفحه (آینه)
inline void neonHand(HandPose p, float sc, float x, float y, float ang, bool left) {
  if (!buildHand(p, sc)) return;
  // blitHand با زاویه‌ي مثبت پادساعت‌گرد می‌چرخاند (روی شبیه‌ساز دیده شد) — این‌جا + = ساعت‌گرد
  blitHand(x, y, left ? ang : -ang, left);
}

// =====================================================
//   ژست‌های کاتالوگ (همان ۱۲ ژست قبلی ، با جای تازه در مختصات واقعی)
// =====================================================
inline void neonHands(HandKind k, float t, float cx, float cy, float sy0) {
  switch (k) {
    case HK_NONE: break;
    case HK_WAVE:
      neonHand(HP_OPEN, 1.5f, cx + 70, cy + 14, 0.28f * sinf(t * 6.0f), false);
      break;
    case HK_THUMBUP:
      neonHand(HP_THUMBUP, 1.45f, cx + 70, cy + 46, 0, false);
      break;
    case HK_VICTORY:
      neonHand(HP_VICTORY, 1.45f, cx + 68, cy + 44, 0.1f, false);
      break;
    case HK_POINT:
      neonHand(HP_POINT, 1.4f, cx + 72, cy + 44, -0.25f, false);
      break;
    case HK_OVERMOUTH:
      neonHand(HP_FLAT, 1.35f, cx + 18, sy0 + 26 + 1.5f * sinf(t * 4.0f), -0.35f, false);
      break;
    case HK_OVEREYES: {
      float peek = 3 * sinf(t * 1.6f);
      for (int sd = -1; sd <= 1; sd += 2)
        neonHand(HP_OPEN, 1.5f, cx + sd * NE_DX, cy + 22 + peek, 0, sd < 0);
      break;
    }
    case HK_OPEN:
      for (int sd = -1; sd <= 1; sd += 2)
        neonHand(HP_OPEN, 1.15f, cx + sd * 84, cy + 30, sd * 0.35f + 0.15f * sinf(t * 5.0f + sd), sd < 0);
      break;
    case HK_CLAP: {
      // فاصله‌ي دو دست با یک موج نرم (قبلا قدر مطلقِ سینوس بود = برخورد تیز)
      float ph = 0.5f + 0.5f * cosf(t * 9.0f);          // ۱ باز ، ۰ به هم خورده
      float gap = 2 + 16 * ph * ph;
      float hy = cy + 64;
      for (int sd = -1; sd <= 1; sd += 2)
        neonHand(HP_FLAT, 1.1f, cx + sd * (gap + 14), hy, -sd * 0.55f, sd < 0);
      if (ph < 0.12f) {                                   // جرقه‌ي برخورد
        for (int i = 0; i < 3; i++) {
          float a = -2.2f + i * 0.7f;
          float X0 = cx + 26 * cosf(a), Y0 = hy - 30 + 26 * sinf(a);
          float X1 = cx + 36 * cosf(a), Y1 = hy - 30 + 36 * sinf(a);
          glowDraw((int)fminf(X0, X1) - 10, (int)fminf(Y0, Y1) - 10, (int)fmaxf(X0, X1) + 10, (int)fmaxf(Y0, Y1) + 10,
                   [&](float x, float y) { return sdSeg(x, y, X0, Y0, X1, Y1, 1.6f); },
                   {255, 235, 120}, {255, 200, 60}, 8);
        }
      }
      break;
    }
    case HK_HEARTHAND: {
      // ۱۲ مهر (بعد از تست): قبلا دست‌ها بالای سر و روی ابروها می‌افتادند.
      // حالا دو دست یک قلب بزرگ را جلوی سینه از دو طرف نگه داشته‌اند.
      float hs = 23 * (1 + 0.07f * sinf(t * 4.0f));
      float hx = cx, hy = cy + 64;
      glowDraw((int)(hx - hs - 18), (int)(hy - hs - 18), (int)(hx + hs + 18), (int)(hy + hs + 18),
               [&](float x, float y) { return sdHeart(x - hx, y - hy, hs); },
               {255, 110, 185}, NE_GLOWC, 14);
      for (int sd = -1; sd <= 1; sd += 2)
        neonHand(HP_FLAT, 0.85f, hx + sd * (hs + 16), hy + 6, -sd * 0.35f, sd < 0);
      break;
    }
    case HK_SALUTE:
      neonHand(HP_FLAT, 1.35f, cx + 82, cy - 44, -1.0f, false);
      break;
    case HK_SHRUG: {
      // شانه بالا انداختن: دو دستِ باز کنار صورت ، انگشت‌ها رو به بیرون-بالا ،
      // که با هم کمی بالا و پایین می‌روند (همان حرکت «نمی‌دونم»)
      float bob = 5 * fabsf(sinf(t * 3.2f));
      for (int sd = -1; sd <= 1; sd += 2)
        neonHand(HP_OPEN, 1.2f, cx + sd * 82, cy + 34 - bob, sd * 1.15f, sd < 0);
      break;
    }
    case HK_SHY: {
      float wig = 0.12f * sinf(t * 5.0f);
      for (int sd = -1; sd <= 1; sd += 2)
        neonHand(HP_POINT, 1.0f, cx + sd * 18, cy + 72, -sd * (0.6f + wig), sd < 0);
      break;
    }
  }
}
