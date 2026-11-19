#pragma once
// =====================================================
//   MOOCHI — ۷۴ چهره‌ي کاتالوگ به سبک نئونی (۱۲ مهر ۱۴۰۵)
//
//   هر چهره‌ي کاتالوگ از چند جزء ساخته شده: ابرو ، چشم ، دهان ، لپ ،
//   «اضافه» (تاج ، عینک ، اشک ، …) و دست. این‌جا:
//   • چشم ، ابرو ، دهان و لپ از نو با همان سبک نئونی چشم‌های اصلی کشیده می‌شوند
//     (هر نوع چشم/دهان قدیمی یک معادل نئونی دارد — جدول پایین).
//   • اضافات و دست‌ها (بیش از ۴۰ شکل) با همان کد قدیمی کشیده می‌شوند ، ولی از
//     «لایه‌ي نئون» (moochi_face.h): اول ۳ گذر درخشش ، بعد خود شکل.
//     پس رنگ و شکلشان همان است ، فقط دورشان درخشش دارد.
// =====================================================

#include "moochi_neon_hands.h"

// اجرای یک تابع کشیدن قدیمی با درخشش نئونی
template <class F>
inline void neonLayer(F draw) {
  // ۳ گذر (۴ گذر روی کامپیوتر ۳۰٪ کندتر بود و فرقش به سختی دیده می‌شد)
  const int   GR[3] = {19, 11, 5};                // بزرگ‌تر شدن شکل (پیکسل مجازی)
  const float KK[3] = {0.13f, 0.30f, 0.56f};
  gfxMax = true;
  for (int i = 0; i < 3; i++) { gNx.glow = true; gNx.grow = GR[i]; gNx.k = KK[i]; draw(); }
  gNx.glow = false; gNx.grow = 0; gNx.k = 1;
  gfxMax = false;
  draw();                                          // خود شکل ، روی همه
}

// ---------- چشم ----------
struct CatEye {
  NeonKind kL = NK_NORMAL, kR = NK_NORMAL;
  float lidF = 0;        // ۰ = بدون پلک صاف ؛ وگرنه پلک صاف در −eh×lidF
  float ehK = 1;         // ضریب ارتفاع چشم (چشم ریز کرده)
  float szK = 1;         // ضریب اندازه
  bool closed = false, angry = false, mirrorL = false, spin = false;
  float spinSpeed = 5, gyAdd = 0;
};

inline CatEye catEyeFor(EyeKind k) {
  CatEye e;
  switch (k) {
    case EK_ROUND:   break;
    case EK_ROUND2:  e.szK = 1.08f; break;
    case EK_HALF:    e.lidF = 0.05f; break;
    case EK_ARC:     e.kL = e.kR = NK_HAPPY; break;
    case EK_CARET:   e.kL = e.kR = NK_E_CARET; e.mirrorL = true; break;
    case EK_HEART:   e.kL = e.kR = NK_LOVE; break;
    case EK_RING:    e.kL = e.kR = NK_E_RING; break;
    case EK_LINE:    e.kL = e.kR = NK_E_LINE; break;
    case EK_WOBBLE:  e.kL = e.kR = NK_E_WOBBLE; break;
    case EK_UP:      e.kL = e.kR = NK_E_UP; break;
    case EK_SPIRAL:  e.kL = e.kR = NK_DIZZY; e.spin = true; e.spinSpeed = 6; break;
    case EK_SWIRL:   e.kL = e.kR = NK_DIZZY; e.spin = true; e.spinSpeed = 2.5f; break;
    case EK_SLEEP:   e.closed = true; break;
    case EK_FLAME:   e.kL = e.kR = NK_ANGRY; e.angry = true; e.mirrorL = true; break;
    case EK_WINK_L:  e.kL = NK_E_CARET; e.mirrorL = true; break;   // چپ «>» ، راست باز
    case EK_WINK_R:  e.kR = NK_E_CARET; break;                     // راست «<» ، چپ باز
    case EK_STAR:    e.kL = e.kR = NK_EXCITED; break;
    case EK_DOT:     e.kL = e.kR = NK_E_DOT; break;
    case EK_SQUINT:  e.ehK = 0.30f; break;
    case EK_CROSS:   e.kL = e.kR = NK_E_CROSS; break;
    case EK_CROSS2:  e.kL = e.kR = NK_E_CROSS; break;
    case EK_ROLLUP:  e.lidF = 0.05f; e.gyAdd = -9; break;
  }
  return e;
}

inline EyeCfg catCfg(NeonKind K, const CatEye &e, float hw, float eh, float shine, const RGBf &glowC, float t) {
  EyeCfg E;
  E.K = K; E.hw = hw; E.eh = eh; E.shine = shine;
  E.closed = e.closed || (K == NK_NORMAL && eh < 4);
  E.top = e.angry ? &NE_ANGRY_TOP : &NE_TOP;
  E.glow = &glowC;
  E.lidK = 0; E.lidY = -1e9f;
  if (e.lidF > 0 && K == NK_NORMAL) E.lidY = -eh * e.lidF;
  if (K == NK_ANGRY) { E.lidK = -0.55f; E.lidY = -eh * 0.25f; }
  E.spin = 0;
  if (K != NK_NORMAL && K != NK_ANGRY) {           // شکل‌های ثابت: پلک زدن برچسب را از نو نسازد
    E.eh = hw; E.shine = 1; E.closed = false;
  }
  if (K == NK_LOVE || K == NK_EXCITED) {
    float pul = (K == NK_LOVE) ? 1 + 0.08f * sinf(t * 7.0f) : 1 + 0.06f * sinf(t * 9.0f);
    E.hw = roundf(hw * pul * 2) * 0.5f;
  }
  return E;
}

// ---------- ابرو ----------
inline void catBrows(BrowShape b, float exL, float exR, float by) {
  if (b == BR_NONE) return;
  // نقاط برای چشم راست (x+ = بیرون). چشم چپ آینه است.
  float xs[5], ys[5]; int n;
  auto set = [&](std::initializer_list<float> X, std::initializer_list<float> Y) {
    n = 0; auto yi = Y.begin();
    for (float x : X) { xs[n] = x; ys[n] = *yi++; n++; }
  };
  for (int side = 0; side < 2; side++) {          // ۰ چپ ، ۱ راست
    BrowShape sh = b;
    float lift = 0;
    if (b == BR_ONE_UP) { sh = BR_CALM; if (side == 1) lift = -8; }
    switch (sh) {
      case BR_UP:    set({-15, -7, 0, 7, 15}, {-3, -7, -8, -7, -3}); break;
      case BR_SAD:   set({-14, 14}, {-4, 4}); break;
      case BR_ANGRY: set({-14, 14}, {4, -4}); break;
      case BR_FLAT:  set({-15, 15}, {0, 0}); break;
      default:       set({-15, -7, 0, 7, 15}, {3, -1, -2, -1, 3}); break;   // آرام
    }
    float ox = side ? exR : exL, oy = by + lift;
    float X[5], Y[5];
    for (int i = 0; i < n; i++) { X[i] = ox + (side ? xs[i] : -xs[i]); Y[i] = oy + ys[i]; }
    float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
    for (int i = 0; i < n; i++) { x0 = fminf(x0, X[i]); x1 = fmaxf(x1, X[i]); y0 = fminf(y0, Y[i]); y1 = fmaxf(y1, Y[i]); }
    glowDraw((int)x0 - 14, (int)y0 - 14, (int)x1 + 14, (int)y1 + 14,
             [&](float x, float y) { return sdPoly(x, y, X, Y, n, 2.0f); }, NE_LINE, NE_GLOWC, 12);
  }
}

// ---------- دهان ----------
inline void catMouth(MouthKind k, float t, float cx, float sy0, const RGBf &glowC) {
  MouthShape M;
  int q = 0;
  switch (k) {
    case MK_NONE:     M.type = 0; break;
    case MK_TINY:     mouthSmile(M, 11, 5, 0); break;
    case MK_WIDE:     mouthSmile(M, 17, 8, 0); break;
    case MK_FLAT:     M.type = 1; M.n = 2; M.xs[0] = -12; M.ys[0] = 4; M.xs[1] = 12; M.ys[1] = 4; break;
    case MK_OPEN:     M.type = 2; M.ex = 0; M.ey = 6; M.rx = 10; M.ry = 11; break;
    case MK_BLOB:     M.type = 2; M.ex = 0; M.ey = 5; M.rx = 13; M.ry = 9; break;
    case MK_SMALL_O:  M.type = 2; M.ex = 0; M.ey = 5; M.rx = 5.5f; M.ry = 6.5f; break;
    case MK_OVAL:     M.type = 2; M.ex = 0; M.ey = 7; M.rx = 6; M.ry = 11; break;
    case MK_TONGUE:   mouthSmile(M, 12, 5, 0); M.fill = true; M.fx = 3; M.fy = 8; M.frx = 5; M.fry = 5; break;
    case MK_TONGUEOUT: {
      q = (int)(t * 12) % 1000;
      mouthSmile(M, 13, 4, 0); M.fill = true; M.fx = 1.5f * sinf(q * 0.5f); M.fy = 10; M.frx = 6; M.fry = 8;
      break;
    }
    case MK_KISS3: {
      const float kx[7] = {0, 4.5f, 4.5f, 1, 4.5f, 4.5f, 0}, ky[7] = {-6, -5, -2, 0, 2, 5, 6};
      M.type = 1; M.n = 7;
      for (int i = 0; i < 7; i++) { M.xs[i] = kx[i] + 2; M.ys[i] = ky[i] + 6; }
      break;
    }
    case MK_SADWAVE:
      M.type = 1; M.n = 13;
      for (int i = 0; i < 13; i++) { float u = -1 + 2.0f * i / 12; M.xs[i] = u * 16; M.ys[i] = 7 - 6 * (1 - u * u) + 1.3f * sinf(u * 9.4f); }
      break;
    case MK_WAVE:
      q = (int)(t * 12) % 1000;
      M.type = 1; M.n = 13;
      for (int i = 0; i < 13; i++) { float u = -1 + 2.0f * i / 12; M.xs[i] = u * 17; M.ys[i] = 4 + 3 * sinf(u * 7.0f + q * 0.583f); }
      break;
    case MK_SHOUT: {
      q = (int)(t * 12) % 1000;
      float mh = 9 + 1.5f * sinf(q * 1.2f);
      M.type = 5; M.ex = 0; M.ey = 8; M.rx = 15; M.ry = mh;
      const float tx[7] = {-11, -7.3f, -3.7f, 0, 3.7f, 7.3f, 11};
      M.n = 7;
      for (int i = 0; i < 7; i++) { M.xs[i] = tx[i]; M.ys[i] = 8 - mh + 2 + ((i & 1) ? 5 : 0); }
      break;
    }
    case MK_SQUIGGLE:
      M.type = 1; M.n = 13;
      for (int i = 0; i < 13; i++) { float u = -1 + 2.0f * i / 12; M.xs[i] = u * 12; M.ys[i] = 5 + 2 * sinf(u * 9.42f); }
      break;
    case MK_ZIGZAG: {
      M.type = 1; M.n = 7;
      for (int i = 0; i < 7; i++) { M.xs[i] = -13 + i * 13.0f / 3; M.ys[i] = (i & 1) ? 0 : 6; }
      break;
    }
    case MK_CROOKED: {
      const float cxs[4] = {-11, -2, 6, 12}, cys[4] = {4, 5, 3, -2};
      M.type = 1; M.n = 4;
      for (int i = 0; i < 4; i++) { M.xs[i] = cxs[i]; M.ys[i] = cys[i]; }
      break;
    }
    case MK_BIGSMILE:
      M.type = 3; M.ex = 0; M.ey = 0; M.rx = 17; M.ry = 12;
      M.n = 2; M.xs[0] = -14; M.ys[0] = 4; M.xs[1] = 14; M.ys[1] = 4;     // خط دندان
      break;
  }
  neonMouth(M, 2000 + (int)k, q, cx, sy0, glowC);
}

// ---------- لپ ----------
inline void catBlush(BlushKind k, float cx, float cy, float t) {
  if (k == BK_NONE) return;
  float rx = 12, br = 0.75f;
  if (k == BK_BIG) { rx = 14 + 1.5f * sinf(t * 3.0f); br = 0.95f; }
  for (int e = 0; e < 2; e++) {
    float bx = cx + (e ? 1 : -1) * (NE_DX + 8), by = cy + 44;
    if (k == BK_HATCH) {                 // سه خط کج کوچک
      for (int i = -1; i <= 1; i++) {
        float X[2] = {bx + i * 7 - 3, bx + i * 7 + 3}, Y[2] = {by + 4, by - 4};
        glowDraw((int)bx - 24, (int)by - 16, (int)bx + 24, (int)by + 16,
                 [&](float x, float y) { return sdPoly(x, y, X, Y, 2, 1.4f); }, NE_BLUSHG, NE_BLUSHG, 7, 0.8f);
      }
    } else {
      glowDraw((int)(bx - rx - 14), (int)by - 18, (int)(bx + rx + 14), (int)by + 18,
               [&](float x, float y) { return sdEllipseA(x - bx, y - by, rx, 5.5f); },
               NE_BLUSH, NE_BLUSHG, 10, br);
    }
  }
}

// =====================================================
//   کشیدن یک چهره‌ي کاتالوگ در نوار جاری
// =====================================================
inline void neonDrawCatalog(const FaceRecipe &r, NeonParams P) {
  for (int i = 0; i < PHYS_W * bandH; i++) bandBuf[i] = 0;
  const float t = P.t;
  if (r.flags & FR_SHAKE) { P.fx += 1.7f * sinf(t * 31.0f); P.fy += 1.0f * sinf(t * 27.0f); }

  CatEye ce = catEyeFor(r.eye);
  P.gy += ce.gyAdd;
  float cx = NE_CX + P.fx, cy = NE_CY + P.fy;
  float hw = NE_HW * ce.szK, hh = NE_HH * ce.szK * ce.ehK;
  float open = P.open < 0 ? 0 : (P.open > 1 ? 1 : P.open);
  float eh = hh * open;
  if (eh < hw * 0.18f) eh = hw * 0.18f;
  eh = roundf(eh * 2) * 0.5f;
  float shine = open > 0.55f ? 1.0f : roundf(open / 0.55f * 8) / 8;
  if (ce.ehK < 1) shine = 0;                       // چشم ریز کرده: برق جا نمی‌شود
  float exL = cx - NE_DX + P.gx, exR = cx + NE_DX + P.gx, ey = cy + P.gy;
  float sy0 = NE_SMILE_Y + P.fy;
  const RGBf &glowC = ce.angry ? NE_ANGRY_GLOW : NE_GLOWC;

  // اضافات و دست‌های قدیمی در مختصات مجازی ۳۶۰ — جای چشم‌ها را با چشم‌های نئونی یکی کن
  int ox = (int)(P.fx * 1.5f), oy = (int)(P.fy * 1.5f) - 6;
  int savedDX = EYE_DX;
  EYE_DX = 78;                                     // ۵۲ پیکسل واقعی = فاصله‌ي چشم نئونی

  // ۱) اضافات پشت (تاج ، گوش ، کلاه ، خورشید ، ماه)
  neonLayer([&] { faceExtraBack(ox, oy, r.extra, t); });

  // ۲) چشم‌ها
  {
    EyeCfg Er = catCfg(ce.kR, ce, hw, eh, shine, glowC, t);
    EyeCfg El = catCfg(ce.kL, ce, hw, eh, shine, glowC, t);
    // اگر حافظه برای برچسب نبود ، مستقیم کشیده می‌شود (کندتر ولی چشم گم نمی‌شود)
    if (ce.spin && buildSprite(spr[0], Er)) {
      float ang = t * ce.spinSpeed;
      blitRot(spr[0], exR, ey, ang, false);
      blitRot(spr[0], exL, ey, ang, true);
    } else if (ce.kL == ce.kR && buildSprite(spr[0], Er)) {
      blitSprite(spr[0], exR, ey, false); blitSprite(spr[0], exL, ey, ce.mirrorL);
    } else {
      if (buildSprite(spr[0], Er)) blitSprite(spr[0], exR, ey, false); else directEye(Er, exR, ey, 1);
      if (ce.mirrorL && ce.kL == NK_ANGRY) El.lidK = -El.lidK;
      if (buildSprite(spr[1], El)) blitSprite(spr[1], exL, ey, ce.mirrorL); else directEye(El, exL, ey, 1);
    }
  }

  // ۳) ابرو — نه وقتی سرپوش (تاج ، عینک ، …) جای ابرو را گرفته (همان قاعده‌ي کد قدیمی)
  bool headgear = (r.extra == XK_HORNS)     || (r.extra == XK_CROWN)    ||
                  (r.extra == XK_CATEARS)   || (r.extra == XK_GLASSES)  ||
                  (r.extra == XK_WIZARDHAT) || (r.extra == XK_PARTYHAT) ||
                  (r.extra == XK_EYEPATCH);
  if (!headgear) catBrows(r.brow, exL, exR, cy - 52 + P.gy * 0.5f - (r.eye == EK_ROUND2 ? 4 : 0));

  // ۴) لپ و دهان
  catBlush(r.blush, cx, cy, t);
  catMouth(r.mouth, t, cx, sy0, glowC);

  // ۵) اضافات جلو ، دست ، پاپیون
  // نیش ، دماسنج و آبنبات برای دهان قدیمی (پایین‌تر) جا داده شده بودند — بالا بیایند
  int oyM = oy + ((r.extra == XK_THERMO || r.extra == XK_CANDY) ? -64 : 0);
  if (r.extra == XK_FANGS) oyM = oy - 40;          // دهان نیش‌دار پهن است: زیر فاصله‌ي دو چشم
  neonLayer([&] {
    faceExtraFront(ox, oyM, r.extra, t);
    if (r.flags & FR_BOW) drawBow(CX - EYE_DX - 34 + ox, CY - 66 + oy, bowPhase);
  });
  // ۶) دست‌ها — نئونی ، پشت دست ، از برچسب (روی همه)
  neonHands(r.hand, t, cx, cy, sy0);

  EYE_DX = savedDX;
}
