// =====================================================
//   MOOCHI  —  موتور احساسات
//
//   موچي دختر است. صورتش روي نمايشگر گرد ۳۶۰×۳۶۰ است
//   و به لمس و حرکت واکنش نشان مي‌دهد.
//
//   ┌─ چه کاري ← چه واکنشي ───────────────────────────┐
//   │ تکان دادن محکم       →  سرگيجه                  │
//   │ ضربه‌ي ناگهاني        →  دردش مي‌گيرد ، ناراحت     │
//   │ نوازش (کشيدن انگشت)  →  لوس مي‌شود ، خودش را     │
//   │                          به دستت مي‌چسباند       │
//   │ نوازش طولاني         →  عاشق مي‌شود (قلب)        │
//   │ سقلمه وقتي آرام است  →  جا مي‌خورد               │
//   │ سقلمه وقتي خوشحال    →  ذوق مي‌کند                │
//   │ سقلمه‌ي پياپي         →  اذيت مي‌شود ، ناراحت      │
//   │ بي‌توجهي طولاني       →  خوابش مي‌آيد ← مي‌خوابد   │
//   │ لمس در خواب          →  از خواب مي‌پرد            │
//   └─────────────────────────────────────────────────┘
//
//   قاعده‌ي معماري: هيچ‌جاي حلقه‌ي اصلي delay ندارد.
//   هر فريم: حواس خوانده مي‌شود → حالت به‌روز مي‌شود →
//   يک فريم کشيده مي‌شود. اگر delay بگذاري، ربات وسط
//   انيميشن کر و کور مي‌شود و لمس را از دست مي‌دهد.
// =====================================================

#include <Arduino.h>
#include <Wire.h>
#include "moochi_lcd.h"
#include "moochi_face.h"
#include "moochi_sense.h"
#include "moochi_mic.h"
#include "moochi_soul.h"
#include "moochi_voice.h"
#include "moochi_catalog.h"

uint16_t *fb = nullptr;

// =====================================================
//  تنظيم‌هاي رفتاري — همه‌ي عددهاي قابل تنظيم اينجا
// =====================================================
const uint32_t T_SLEEPY_AFTER   = 60000;  // بي‌توجهي تا خواب‌آلودگي
const uint32_t T_SLEEP_AFTER    = 12000;  // خواب‌آلودگي تا خواب
const uint32_t T_SURPRISE       =   900;
const uint32_t T_DIZZY          =  5200;
const uint32_t T_HURT           =  4200;
const uint32_t T_LOVE           =  2800;
const uint32_t T_PET_LINGER     =  1300;  // چقدر بعد از قطع نوازش لوس بماند
const uint32_t T_PET_TO_LOVE    =  6000;  // نوازش پيوسته تا عاشق شدن

const float  SHAKE_TRIGGER      =   1.75f; // انرژي تکان
const float  IMPACT_G           =   2.50f; // شدت ضربه بر حسب g
const float  CALM_BEFORE_IMPACT =   0.60f; // ضربه فقط وقتي قبلش آرام بوده

const int    POKE_MAX_MS        =   200;   // لمس کوتاه‌تر از اين = سقلمه
const int    POKE_MAX_MOVE      =    25;
const int    POKES_TO_ANNOY     =     3;
const uint32_t POKE_WINDOW      =  2500;

const int    STROKE_MIN_MS      =   400;   // نوازش
const int    STROKE_MIN_PATH    =   100;
const int    HOLD_MIN_MS        =   900;   // نگه داشتن آرام

const int    BL_AWAKE           =   420;
const int    BL_SLEEPY          =   230;
const int    BL_ASLEEP          =    80;

// =====================================================
//  حالت‌ها
// =====================================================
enum Mood {
  M_IDLE, M_HAPPY, M_PET, M_LOVE,
  M_DIZZY, M_HURT, M_SURPRISE,
  M_SLEEPY, M_ASLEEP,
  M_CURIOUS, M_THINK, M_SHY, M_EXCITED, M_YAWN,
  M_LISTEN, M_ANGRY,
  M_KISS, M_CRY, M_EYEROLL, M_HEARTHAND, M_FEED, M_RAINBOW
};

const uint32_t T_LISTEN  = 3600;
const uint32_t T_ANGRY   = 3400;
const uint32_t T_KISS    = 3000;
const uint32_t T_CRY     = 4600;
const uint32_t T_EYEROLL = 1500;
const uint32_t T_HEART   = 3600;
const uint32_t T_FEED    = 3000;
const uint32_t T_RAINBOW = 3600;
const uint32_t T_CURIOUS = 3200;
const uint32_t T_THINK   = 4200;
const uint32_t T_SHY     = 2800;
const uint32_t T_EXCITED = 2400;
const uint32_t T_YAWN    = 3000;

Mood     mood       = M_IDLE;
uint32_t moodStart  = 0;
uint32_t moodUntil  = 0;         // ۰ يعني بي‌پايان
uint32_t lastTouchOrMove = 0;    // آخرين تعامل

const char *moodName(Mood m) {
  switch (m) {
    case M_IDLE:     return "عادي";
    case M_HAPPY:    return "خوشحال";
    case M_PET:      return "نوازش — لوس شده";
    case M_LOVE:     return "عاشق";
    case M_DIZZY:    return "سرگيجه";
    case M_HURT:     return "ناراحت";
    case M_SURPRISE: return "جا خورد";
    case M_SLEEPY:   return "خواب‌آلود";
    case M_ASLEEP:   return "خواب";
    case M_CURIOUS:  return "کنجکاو";
    case M_THINK:    return "در فکر";
    case M_SHY:      return "خجالت‌زده";
    case M_EXCITED:  return "ذوق‌زده";
    case M_YAWN:     return "خميازه";
    case M_LISTEN:   return "گوش مي‌دهد";
    case M_ANGRY:    return "عصباني";
    case M_KISS:     return "بوس";
    case M_CRY:      return "گريه";
    case M_EYEROLL:  return "چشم‌غره";
    case M_HEARTHAND:return "قلبِ دو انگشتي";
    case M_FEED:     return "غذا";
    case M_RAINBOW:  return "رنگين‌کمان";
  }
  return "?";
}

// =====================================================
//  گذر نرم بين حالت‌ها
//
//  مشکل: تعويض حالت پرشي بود — يک فريم چهره‌ي قبلي ، فريم
//  بعد چهره‌ي جديد. چشم آدم اين را به عنوان «خطا» مي‌بيند.
//
//  راه‌حلي که انيماتورها استفاده مي‌کنند: **برشِ ناگهاني را
//  پشت يک پلک پنهان کن.** صفحه سريع بسته مي‌شود ، حالت عوض
//  مي‌شود ، و دوباره باز مي‌شود. مغز آن را «پلک زد» مي‌بيند
//  نه «پرید».
//
//  مزيتش: يک تابع براي همه‌ي حالت‌ها کار مي‌کند. لازم نيست
//  براي هر جفت حالت يک گذرِ جدا بنويسيم.
// =====================================================
//
// ⚠️ باگي که در لاگ پيدا شد:
//   نيمه‌ي اول گذر ، چهره‌ي **قبلي** کشيده مي‌شود. ولي moodStart
//   همان لحظه صفر شده بود ، پس آن چهره از **ابتداي** انيميشن
//   خودش کشيده مي‌شد. در خميازه يعني دهان دوباره باز مي‌شد و
//   دست‌ها برمي‌گشتند — يک پرشِ صد ميلي‌ثانيه‌اي.
//
//   راه‌حل: زمانِ شروعِ حالت قبلي را هم نگه دار.
// ---------- چهره‌ي کاتالوگ ----------
// ‎−۱ يعني خاموش و موتورِ حالت‌ها کار مي‌کند.
// اين‌جا تعريف مي‌شود چون setMood لازمش دارد.
int      catFace    = -1;
uint32_t catUntil   = 0;

Mood     prevMood      = M_IDLE;
uint32_t prevMoodStart = 0;
uint32_t transStart    = 0;
const uint32_t TRANS_MS = 230;

// =====================================================
//   حال → صدا
//
//   ☠️ اين نگاشت عمدا **ناقص** است. بعضي حال‌ها صدا ندارند و
//      نبايد داشته باشند:
//        M_IDLE   ساکت — رباتي که بي‌کار هم جيک‌جيک کند آزاردهنده
//                 مي‌شود ، و کاربر خاموشش مي‌کند
//        M_ASLEEP ساکت — خواب صدا ندارد
//        M_THINK  ساکت — فکر کردن بي‌صداست
//
//      سکوت هم بخشي از شخصيت است.
int voiceForMood(Mood m) {
  switch (m) {
    case M_HAPPY:     return V_HAPPY;
    case M_EXCITED:   return V_EXCITED;
    case M_PET:       return V_GIGGLE;
    case M_LOVE:      return V_LOVE;
    case M_KISS:      return V_LOVE;
    case M_HEARTHAND: return V_LOVE;
    case M_CURIOUS:   return V_CURIOUS;
    case M_LISTEN:    return V_LISTEN;
    case M_SURPRISE:  return V_SURPRISE;
    case M_HURT:      return V_HURT;
    case M_CRY:       return V_CRY;
    case M_ANGRY:     return V_ANGRY;
    case M_EYEROLL:   return V_GRUMPY;
    case M_DIZZY:     return V_DIZZY;
    case M_YAWN:      return V_YAWN;
    case M_SLEEPY:    return V_SLEEP;
    case M_FEED:      return V_EAT;
    case M_SHY:       return V_CHIRP;
    case M_RAINBOW:   return V_OK;
    default:          return V_NONE;      // ساکت
  }
}

void setMood(Mood m, uint32_t durMs) {
  // ☠️ حالتِ تازه بايد چهره‌ي کاتالوگِ قبلي را ببندد ، وگرنه
  //    مثلا بعد از «عصباني» چهره‌ي عصبانيت روي «عادي» مي‌ماند.
  //    catFace پايين‌تر تعريف شده ، پس مستقيم دست مي‌زنيم.
  catFace = -1;
  if (mood != m) {
    Serial.printf("[حالت] %s\n", moodName(m));
    //   صدا فقط روي **تغييرِ** حال ، نه روي تمديدِ همان حال.
    //   وگرنه موچي در حالِ خوشحال بي‌وقفه قهقهه مي‌زند.
    int v = voiceForMood(m);
    if (v != V_NONE) voiceSay(v);
    prevMood      = mood;
    prevMoodStart = moodStart;     // ← زمانِ خودش ، نه زمانِ تازه
    transStart    = millis();
  }
  mood      = m;
  moodStart = millis();
  moodUntil = durMs ? moodStart + durMs : 0;
}

void showFace(int idx, uint32_t durMs) {
  // 🛡 ‎−۱ (نام پيدا نشد) را به «آخرين چهره» تبديل نکن — هيچ نشان نده
  if (idx < 0) { catFace = -1; return; }
  catFace  = (idx + CATALOG_N) % CATALOG_N;
  catUntil = durMs ? millis() + durMs : 0;
  moodStart = millis();
  Serial.printf("[چهره] %d/%d  %s\n", catFace + 1, CATALOG_N,
                CATALOG[catFace].name);
}
inline void hideFace() { catFace = -1; }

// يکي از يک دسته را نشان بده
#define SHOW_SET(setName, ms) showFace(pickFrom(setName, SET_N(setName)), ms)

// دو پرده از بالا و پايين بسته مي‌شوند
//   s = 0 کاملا باز ، s = 1 کاملا بسته
void drawShutter(float s) {
  if (s <= 0.0f) return;
  if (s > 1.0f) s = 1.0f;
  int half = (int)(s * (LCD_H / 2 + 2));
  fbRect(0, 0, LCD_W, half, C_BG);
  fbRect(0, LCD_H - half, LCD_W, half, C_BG);
}

// ثانيه‌هاي گذشته از شروع حالت
float moodT() { return (millis() - moodStart) / 1000.0f; }

// =====================================================
//  پاپيون — کدام حالت‌ها داشته باشند
//  فقط true/false را عوض کن. ترتيب دقيقا مثل enum Mood است.
// =====================================================
const bool BOW_IN[] = {
  /* عادي     */ false,
  /* خوشحال   */ false,
  /* لوس شده  */ false,
  /* عاشق     */ true,     // اينجا اولين بار خواستيش
  /* سرگيجه   */ false,
  /* ناراحت   */ false,
  /* جا خورد  */ false,
  /* خواب‌آلود */ false,
  /* خواب     */ false,
  /* کنجکاو   */ false,
  /* در فکر   */ false,
  /* خجالت‌زده */ true,     // پاپيون به خجالت مي‌آيد
  /* ذوق‌زده  */ true,     // و به ذوق
  /* خميازه   */ false,
  /* گوش‌دادن  */ false,
  /* عصباني   */ false,
  /* بوس      */ true,     // بوس با پاپيون قشنگ‌تر است
  /* گريه     */ false,
  /* چشم‌غره  */ false,
  /* قلبِ‌دستي */ true,
  /* غذا      */ false,
  /* رنگين‌کمان*/ false,
};

// =====================================================
//  پلک زدن و نگاه — هر دو بدون انسداد
// =====================================================
bool     blinking  = false;
uint32_t blinkAt   = 0, blinkT0 = 0;
const uint32_t BLINK_MS = 170;

int eyeOpen() {
  uint32_t now = millis();
  if (!blinking) {
    if (now >= blinkAt) { blinking = true; blinkT0 = now; }
    return 100;
  }
  uint32_t d = now - blinkT0;
  if (d >= BLINK_MS) {
    blinking = false;
    blinkAt  = now + random(2200, 5400);
    return 100;
  }
  float t = (float)d / BLINK_MS;
  float v = (t < 0.5f) ? (1.0f - t * 2.0f) : ((t - 0.5f) * 2.0f);
  int o = (int)(v * 100);
  return o < 0 ? 0 : o;
}
void forceBlinkSoon() { blinking = false; blinkAt = millis() + 120; }

float gazeX = 0, gazeY = 0, gazeTX = 0, gazeTY = 0;
uint32_t gazeAt = 0;

void gazeIdle() {
  if (millis() >= gazeAt) {
    gazeTX = random(-24, 25);
    gazeTY = random(-8, 17);
    gazeAt = millis() + random(1200, 3200);
  }
}
void gazeTowards(float tx, float ty) { gazeTX = tx; gazeTY = ty; }
void gazeStep() {                       // نرم کردن حرکت چشم
  gazeX += (gazeTX - gazeX) * 0.14f;
  gazeY += (gazeTY - gazeY) * 0.14f;
}

// جابه‌جايي کل صورت (تکيه دادن ، لرزيدن)
float faceX = 0, faceY = 0, faceTX = 0, faceTY = 0;
void faceStep() {
  faceX += (faceTX - faceX) * 0.16f;
  faceY += (faceTY - faceY) * 0.16f;
}

float bowPhase = 0;

// =====================================================
//  چيزهاي شناور — قلب‌ها و حرف‌هاي Z
// =====================================================
struct Floater { bool on = false; int x0; float y0; uint32_t t0; int kind; };
Floater floaters[4];

void spawnFloater(int x, int y, int kind) {      // kind 0=قلب  1=حرف Z
  for (auto &f : floaters)
    if (!f.on) { f.on = true; f.x0 = x; f.y0 = y; f.t0 = millis(); f.kind = kind; return; }
}
void drawFloaters() {
  uint32_t now = millis();
  for (auto &f : floaters) {
    if (!f.on) continue;
    float t = (now - f.t0) / 1400.0f;            // عمر ۱٫۴ ثانيه
    if (t >= 1.0f) { f.on = false; continue; }
    int x = f.x0 + (int)(14.0f * sinf(t * 5.0f));
    int y = (int)(f.y0 - t * 80.0f);
    int s = (int)((1.0f - t * 0.45f) * (f.kind == 0 ? 30 : 26));
    if (f.kind == 0) fbHeart(x, y, s, C_RED);
    else             fbZ(x, y, s, 3, C_WHITE);
  }
}

// =====================================================
//  تشخيص تعامل
// =====================================================
bool     tDown      = false;
uint32_t tDownAt    = 0;
int      tX0 = 0, tY0 = 0, tLX = 0, tLY = 0;
float    tPath      = 0;
bool     strokeNow  = false;   // همين الان دارد نوازش مي‌شود
uint32_t strokeSince= 0;

uint32_t pokeT[POKES_TO_ANNOY] = {0};
int      pokeIdx = 0;

float shakeEnergy = 0, prevMag = 1.0f;

// خروجي‌هاي اين فريم
bool evPoke = false, evAnnoy = false, evShake = false, evImpact = false;
bool evClap = false;
uint8_t evGesture = TG_NONE;   // حرکتِ چيپ لمسي ، يک بار مصرف

// آيا اين ناراحتي فروکشِ يک عصبانيت است؟ (تا حلقه نسازد)
bool angryCooling = false;

// =====================================================
//   پنجره‌ي شنيدن
//
//   ☠️ مشکلي که کاربر گزارش داد: «نسبت به صدا خيلي حساسه ،
//      همش واکنش نشون مي‌ده.»
//
//   و علتش يک تصميمِ طراحيِ غلط بود ، نه يک باگ: من موچي
//   را طوري ساخته بودم که **هميشه** گوش بدهد. ولي حيوانِ
//   خانگي هم هميشه به هر صدايي واکنش نشان نمي‌دهد — اول
//   بايد **صدايش کني**.
//
//   پس حالا گوشش بسته است. يک دست که بزني باز مي‌شود و
//   بيست ثانيه به حرفت گوش مي‌دهد. بعد دوباره مي‌بندد.
//
//   دو سودِ ديگر هم دارد که اولش به آن فکر نکرده بودم:
//     ۱ دست زدن **معني** پيدا مي‌کند — قبلا فقط «جا خورد»
//       بود ، حالا «حواسم به توست»
//     ۲ خودِ باز شدنِ گوش يک رويدادِ ديدني است
const uint32_t EARS_MS = 20000;      // چقدر بعد از دست زدن گوش بدهد
uint32_t earsUntil = 0;
inline bool earsOpen() { return earsUntil && millis() < earsUntil; }

// ---------- دو دست زدن ----------
//
// ☠️ حتي با آزمونِ دُم ، يک صداي تکي ممکن است رد شود. ولي
//    **دو** صداي کوتاهِ پياپي در نيم‌ثانيه تصادفي نيست.
//
//    اين همان کاري است که کليدهاي صوتيِ تجاري مي‌کنند ، و
//    دليلش رياضي است: اگر احتمالِ خطاي يک تشخيص ۵٪ باشد ،
//    احتمالِ دو خطاي پياپي در پنجره‌ي نيم‌ثانيه‌اي به هزارم
//    مي‌رسد.
//
//    هزينه‌اش اين است که بايد **دو بار** دست بزني. ولي براي
//    کاري که يعني «حواست به من باشد» ، دو بار طبيعي است.
const uint32_t DBL_MIN = 120;        // زودتر از اين ، طنينِ همان دست است
const uint32_t DBL_MAX = 900;        // ديرتر از اين ، دو رويدادِ جدا
uint32_t lastSingleClap = 0;

// ☠️ وقتي حرکتي اجرا شد ، انگشت هنوز روي صفحه است. فريمِ
//    بعد «نوازش» فعال مي‌شود و حالتِ تازه را مي‌خورد:
//        18:47:55.520  کشيدن به پايين → قلبِ دو انگشتي
//        18:47:55.719  [حالت] نوازش        ← ۲۰۰ms بعد
//    پس بعد از هر حرکت ، نوازش را تا **بلند شدنِ انگشت**
//    قفل مي‌کنيم. يک لمس ، يک معني.
bool gestureLock = false;

// عيب‌يابي: هر لمس را در Serial چاپ کن
// وقتي همه چيز درست شد ، اين را 0 کن
#define DEBUG_TOUCH 1

void senseStep() {
  evPoke = evAnnoy = evShake = evImpact = false;
  uint32_t now = millis();

  // ---------- گوش ----------
  // پرچم را کارگرِ ميکروفن روي هسته‌ي ديگر بالا برده
  // ---------- گوش ----------
  //
  // ☠️ در لاگ ديده شد که **صداي تقّه‌ي انگشت روي شيشه** دست
  //    زدن حساب مي‌شود:
  //        18:24:38.906  👏 دست!
  //        18:24:38.935  [لمس] x=295      ← ۲۹ms بعد
  //        18:24:38.966  [حرکت] يک ضربه
  //    يعني هر ضربه دو واکنش مي‌ساخت — يکي به صدا ، يکي به
  //    لمس — و موچي بين «جا خورد» و «خوشحال» مي‌پريد.
  //
  //    منطقي هم هست: ضربه روي شيشه دقيقا همان امضاي صوتيِ
  //    دست زدن را دارد (بلند و تيز و ناگهاني). هيچ تنظيمِ
  //    آستانه‌اي جدايشان نمي‌کند.
  //
  //    راه‌حل از جنسِ ديگري است: **وقتي انگشت روي صفحه است ،
  //    گوش را باور نکن.** حسِ نزديک‌تر برنده مي‌شود.
  //    ☠️ اصلاحِ اولم غلط بود و لاگ دوباره گرفتش:
  //        18:28:12.086  👏 دست!
  //        18:28:12.141  [حرکت] يک ضربه   ← ۵۵ms **بعد**
  //
  //    من نوشته بودم «اگر تازگي لمس شده ، صدا را ناديده
  //    بگير» — يعني رو به **گذشته** نگاه مي‌کردم. ولي صدا
  //    زودتر از لمس مي‌رسد: موج صوتي فوري است و حسگر خازني
  //    چند ده ميلي‌ثانيه تاخير دارد.
  //
  //    پس بايد **صبر کرد**: دست زدن را نگه مي‌داريم ، و اگر
  //    در ۱۸۰ ميلي‌ثانيه‌ي بعد انگشتي رسيد ، دورش مي‌ريزيم.
  //    قيمتش ۱۸۰ ميلي‌ثانيه تاخير در واکنش به دست زدن است
  //    که اصلا حس نمي‌شود.
  //
  //    و نکته‌ي دوم: ضربه‌هاي خيلي کوتاه هيچ‌وقت
  //    touch.pressed را روشن نمي‌کنند (در لاگ `[لمس]` ندارند ،
  //    فقط `[حرکت]`). پس حرکت را هم بايد نشانه گرفت.
  static uint32_t lastTouchAt  = 0;
  static uint32_t clapPending  = 0;      // ۰ يعني چيزي در انتظار نيست
  const  uint32_t CLAP_HOLD_MS = 180;

  if (touch.pressed || touch.gesture != TG_NONE) lastTouchAt = now;

  evClap = false;
  if (micTakeClap()) clapPending = now;

  if (clapPending) {
    if (lastTouchAt >= clapPending) {
      clapPending = 0;                   // انگشت رسيد — صداي خودش بود
    } else if (now - clapPending >= CLAP_HOLD_MS) {
      clapPending = 0;
      evClap = true;
      // ☠️ اين‌جا `lastTouchOrMove = now` بود و باگِ بدي ساخت:
      //
      //    moodStep دستِ **تکي** را با return رد مي‌کند و
      //    هيچ حالتي عوض نمي‌شود — درست. ولي lastTouchOrMove
      //    همين‌جا صفر شده بود ، پس **فريمِ بعد** بلوکِ
      //    عموميِ بيدار شدن مي‌ديد که idle < 300 است و
      //    موچي را از خواب بلند مي‌کرد.
      //
      //    کاربر گزارش داد «دست نزدم و بيدار شد» — و صدايي
      //    که رد شده بود ، از **راهِ ديگري** اثر گذاشت.
      //
      //    درس: وقتي يک ورودي را رد مي‌کني ، مطمئن شو
      //    **همه‌ي** ردپاهايش را هم پاک کرده‌اي. يک پرچم را
      //    نديده گرفتن کافي نيست اگر همان ورودي متغيرِ
      //    ديگري را هم دست زده باشد.
      //
      //    حالا moodStep خودش lastTouchOrMove را مي‌گذارد ،
      //    فقط وقتي که واقعا روي دست زدن کاري کرد.
      soulClap();
    }
  }

  // ---------- لمس ----------
  touchRead();
  evGesture = touch.gesture;
  if (evGesture != TG_NONE) {
    lastTouchOrMove = now;
    gestureLock = true;             // تا انگشت بلند نشده ، نوازش نه
    Serial.printf("[حرکت] %s\n", gestureName(evGesture));
  }
#if DEBUG_TOUCH
  {
    static bool wasDown = false;
    if (touch.pressed != wasDown) {
      wasDown = touch.pressed;
      if (touch.pressed) Serial.printf("[لمس] x=%d  y=%d\n", touch.x, touch.y);
      else               Serial.println("[لمس] رها شد");
    }
  }
#endif
  if (touch.pressed) {
    if (!tDown) {                       // تازه انگشت گذاشته
      tDown = true; tDownAt = now;
      tX0 = tLX = touch.x; tY0 = tLY = touch.y;
      tPath = 0;
    } else {
      float dx = touch.x - tLX, dy = touch.y - tLY;
      tPath += sqrtf(dx * dx + dy * dy);
      tLX = touch.x; tLY = touch.y;
    }
    uint32_t held = now - tDownAt;
    bool isStroke = (held > STROKE_MIN_MS && tPath > STROKE_MIN_PATH);
    bool isHold   = (held > HOLD_MIN_MS);
    if (isStroke || isHold) {
      if (!strokeNow) strokeSince = now;
      strokeNow = true;
    }
    lastTouchOrMove = now;
  } else if (tDown) {                   // انگشت برداشته شد
    uint32_t held = now - tDownAt;
    if (held < POKE_MAX_MS && tPath < POKE_MAX_MOVE) {
      evPoke = true;
      soulPoke();
      pokeT[pokeIdx] = now;
      pokeIdx = (pokeIdx + 1) % POKES_TO_ANNOY;
      int cnt = 0;
      for (uint32_t p : pokeT) if (p && now - p < POKE_WINDOW) cnt++;
      // ☠️ آستانه ديگر ثابت نيست — شخصيتش تعيينش مي‌کند.
      //    موچيِ دل‌خور با دو سقلمه اذيت مي‌شود ، موچيِ
      //    آرام با سه تا.
      if (cnt >= soulPokesToAnnoy(POKES_TO_ANNOY)) {
        evAnnoy = true;
        soulAnnoy();
        for (auto &p : pokeT) p = 0;    // شمارنده صفر شود
      }
    }
    tDown = false;
    strokeNow = false;
    gestureLock = false;            // انگشت رفت — قفل باز
    lastTouchOrMove = now;
  }

  // ---------- حرکت ----------
  if (imu.kind != IMU_NONE) {
    imuRead();
    float d = fabsf(imu.mag - prevMag);
    prevMag = imu.mag;

    // ضربه = يک جهش تند وقتي قبلش آرام بوده
    if (imu.mag > IMPACT_G && shakeEnergy < CALM_BEFORE_IMPACT) evImpact = true;

    shakeEnergy = shakeEnergy * 0.90f + d;
    if (shakeEnergy > SHAKE_TRIGGER) { evShake = true; shakeEnergy = 0; }

    if (d > 0.18f) lastTouchOrMove = now;
  }
}

// =====================================================
//  کشيدن هر حالت
// =====================================================

// پاپيون فقط اگر اين حالت اجازه داشته باشد
void maybeBow(int ox, int oy, float speed) {
  if (!BOW_IN[mood]) return;
  drawBow(CX - EYE_DX - 34 + ox, CY - 66 + oy, bowPhase * speed);
}

// جفتِ چشمِ گرد با نگاهِ جاري — پرمصرف‌ترين ترکيب
inline void drawnEyesRound(int open, int r = 52, int stars = 1) {
  eyeRound(-1, (int)faceX, (int)faceY, (int)gazeX, (int)gazeY, open, r, stars);
  eyeRound(+1, (int)faceX, (int)faceY, (int)gazeX, (int)gazeY, open, r, stars);
}

void drawIdle(int open) {
  fbFill(C_BG);
  // نسلِ دوم: بيضيِ درشت با مردمکِ براق. نگاه و دنبالِ
  // انگشت رفتن حالا معنا دارد.
  // چشمِ گردِ سفيد با مردمکِ درشت و جرقه‌ي ستاره‌اي
  drawnEyesRound(open);
  drawBrows((int)faceX, (int)faceY, BR_CALM);
  // دهانِ فيروزه‌ايِ خيلي کوچک — تضادِ رنگ ، نه هم‌رنگي
  mouthTiny((int)faceX, (int)faceY, 15, 8);
  maybeBow((int)faceX, (int)faceY, 1.0f);
}

void drawHappy(int open) {
  fbFill(C_BG);
  // ⌒ ⌒ — چشمِ خندان. مردمک رفته ، يعني احساس جدي است.
  int ox = (int)faceX, oy = (int)faceY;
  drawBrushEye(-1, ox, oy, ES_ARC_UP, 0);
  drawBrushEye(+1, ox, oy, ES_ARC_UP, 0);
  drawBrows(ox, oy, BR_CALM, 82);
  mouthTiny(ox, oy, 17, 9);
  blushHatch(ox, oy);                    // گونه‌ي سه‌خطي
  fbSparkle(CX + ox - EYE_DX - 52, CY + oy - 40, 10, C_STAR);
  fbSparkle(CX + ox + EYE_DX + 52, CY + oy - 32, 8,  C_STAR);
  maybeBow(ox, oy, 1.0f);
}

// لوس شدن: چشم‌ها از لذت جمع مي‌شوند ، گونه گل مي‌اندازد ،
// صورت به سمت انگشت تکيه مي‌دهد و آرام تاب مي‌خورد
void drawPet(float t) {
  fbFill(C_BG);
  int sway = (int)(5.0f * sinf(t * 3.4f));
  int ox = (int)faceX + sway;
  int oy = (int)faceY + (int)(3.0f * sinf(t * 2.2f));

  int open = 62 + (int)(10.0f * sinf(t * 2.6f));   // چشم نيمه‌بسته‌ي راضي
  drawBrushEye(-1, ox, oy, ES_ARC_UP, t, C_WHITE, 92);
  drawBrushEye(+1, ox, oy, ES_ARC_UP, t, C_WHITE, 92);
  drawBrows(ox, oy, BR_CALM, 80);

  int wide = 28 + (int)(4.0f * sinf(t * 3.0f));    // گونه‌ي نبض‌دار
  blushOval(ox, oy, wide);

  // لبخند راضي که با تاب صورت کمي بالا و پايين مي‌رود
  mouthTiny(ox, oy, 14, 8 + (int)(2.0f * sinf(t * 2.6f)));

  maybeBow(ox, oy, 1.6f);                          // پاپيون تندتر تاب مي‌خورد
  drawFloaters();
}

// ۴ — عاشقانه
//
// ⚠️ بازطراحي شد. قبلا دو قلب کوچک کنار هم بود ، مثل دو چشم.
//   حالا **يک قلب تمام‌صفحه** است و چشم‌ها روي خودِ قلب‌اند.
//   پر کردن فضا حس را خيلي قوي‌تر مي‌کند تا دو شکل کوچک وسط
//   يک صفحه‌ي سياه.
void drawLove(float t) {
  fbFill(C_BG);
  // ☠️ اندازه ۲۵۰ بود و نوکِ بالايي از صفحه مي‌زد بيرون.
  //    حساب: fbHeart با size=250 → sc=104 ، لُپ‌هاي بالا
  //    تا 1.3×sc=135 پيکسل بالاتر از مرکز مي‌روند و تا
  //    ۱۲۰ پيکسل پهنا دارند. ولي صفحه **گرد** است: در آن
  //    ارتفاع فقط ۱۱۴ پيکسل پهنا ديده مي‌شود. پس بريده شد.
  //
  //    درس: روي صفحه‌ي گرد ، «جا شدن در عرض و طول» کافي
  //    نيست — بايد در **دايره** جا شود.
  int pulse = (int)(12.0f * fabsf(sinf(t * 4.2f)));

  fbHeart(CX + (int)faceX, CY + 10 + (int)faceY, 196 + pulse, C_RED);

  // چشم‌ها خودشان قلب‌اند — و تپش‌شان با قلبِ بزرگ هم‌فاز
  // نيست ، که ريتمِ دوگانه حس را زنده مي‌کند.
  // ☠️ «بالاي قلب ديده نمي‌شود» — و بريده نشده بود ، **پوشانده**
  //    شده بود. چشم‌هاي سفيد دقيقا روي دو لُپِ بالاييِ قلب
  //    مي‌نشستند و آن‌ها را تکه‌تکه مي‌کردند.
  //
  //    درس: وقتي چيزي «نيست» ، قبل از اينکه اندازه‌اش را
  //    عوض کني ببين چيزِ ديگري رويش نيفتاده باشد. من دو بار
  //    اندازه را کم کردم در حالي که مشکل ترتيبِ کشيدن بود.
  //
  //    حالا چشم‌ها روي **تنه‌ي** قلب‌اند نه لُپ‌هايش.
  drawBrushEye(-1, (int)faceX - 40, (int)faceY + 30, ES_HEART, t, C_WHITE, 66);
  drawBrushEye(+1, (int)faceX + 40, (int)faceY + 30, ES_HEART, t, C_WHITE, 66);

  fbCurve(CX + (int)faceX, CY + 76 + (int)faceY, 15, 8, 5, C_WHITE);  // دهان

  // برق‌هاي ريز دور قلب
  for (int i = 0; i < 3; i++) {
    float a = t * 1.6f + i * 2.094f;
    fbSparkle(CX + (int)(128 * cosf(a)), CY + (int)(118 * sinf(a)), 11, C_STAR);
  }

  maybeBow((int)faceX, (int)faceY, 1.6f);
}

// سرگيجه: مردمک‌ها مارپيچ مي‌شوند ، صورت گيج مي‌خورد ،
// و چند ستاره دورِ سر مي‌چرخند
void drawDizzy(float t) {
  fbFill(C_BG);
  int ox = (int)(11.0f * sinf(t * 8.5f));
  int oy = (int)( 7.0f * sinf(t * 6.1f));

  // ☠️ تنها حالتي بود که هنوز چشمِ **مستطيليِ نسلِ اول** را
  //    مي‌کشيد (fbRoundRect با EYE_W/EYE_H). کنارِ بقيه‌ي
  //    حالت‌ها که چشمِ گردِ سفيد دارند ، وصله به نظر مي‌رسيد.
  for (int sd = -1; sd <= 1; sd += 2) {
    int ex = CX + sd * EYE_DX + ox;
    int ey = CY + oy;
    fbCircle(ex, ey, 50, C_WHITE);
    fbSpiral(ex, ey, 2.4f, 34, 4, t * (sd > 0 ? 7.0f : -7.0f), C_PUPIL);
  }

  for (int i = 0; i < 3; i++) {                    // ستاره‌هاي دور سر
    float a = t * 3.2f + i * 2.094f;
    fbSparkle(CX + (int)(96 * cosf(a)) + ox,
              CY - 118 + (int)(20 * sinf(a)) + oy, 11, C_STAR);
  }
  drawBrows(ox, oy, BR_SAD, 76);
  // دهانِ موجي — حالِ بد
  mouthWave(ox, oy, 30, 6, 4, C_TEAL);
  maybeBow(ox, oy, 2.2f);
}

// ناراحتي: پلک از سمت بيرون افتاده ، نگاه پايين ، اشک مي‌چکد
// ⚠️ open از بيرون مي‌آيد — eyeOpen() عوارض جانبي دارد و
//    نبايد در يک فريم دو بار صدا زده شود
void drawHurt(float t, int open) {
  fbFill(C_BG);
  int oy = (int)faceY + 7;
  int ox = (int)faceX;

  // در طرحِ مرجع ، ناراحتْ چشمِ **گردِ باز** دارد نه بسته:
  // مردمک‌ها بالا و به هم نزديک ، ابروي نازکِ افتاده رويشان ،
  // و يک قطره. چشمِ باز غم را «التماس‌آميز» مي‌کند ، چشمِ
  // بسته فقط «خسته».
  int wob = (int)(2.0f * sinf(t * 14.0f));
  setBrowGaze(0, -12);
  eyeRound(-1, ox + wob, oy, -6, -12, 96, 50, 2);
  eyeRound(+1, ox - wob, oy,  6, -12, 96, 50, 2);
  drawBrows(ox, oy, BR_SAD, 74);

  float ct = fmodf(t, 2.0f);                       // چرخه‌ي دو ثانيه‌اي اشک
  if (ct > 0.7f) {
    float d = (ct - 0.7f) / 1.0f;
    if (d <= 1.0f)
      fbEllipse(CX - EYE_DX + ox - 20, CY + 40 + oy + (int)(d * 95), 7, 11, C_TEAR);
  }
  mouthSadWave(ox, oy);                             // دهانِ موجيِ آبي
  maybeBow(ox, oy, 0.5f);                          // پاپيون هم بي‌حال مي‌شود
}

void drawSurprise(float t) {
  fbFill(C_BG);
  // ○ ○ — حلقه‌ي گشاد. جا خوردن.
  int pop = (t < 0.18f) ? (int)(t / 0.18f * 100) : 100;   // باز شدن ناگهاني
  // نامتقارني شديدتر در اين حالت — يک چشم آشکارا گشادتر
  // چشمِ گشادِ گرد با مردمکِ کوچکِ وسط — جا خوردن
  eyeRound(-1, (int)faceX, (int)faceY, 0, 0, pop, 56, 0);
  eyeRound(+1, (int)faceX, (int)faceY, 0, 0, pop, 56, 0);
  drawBrows((int)faceX, (int)faceY, BR_UP, 84);
  // دهانِ باز — با چشم‌ها همزمان باز مي‌شود
  mouthOpen((int)faceX, (int)faceY, (16 * pop) / 100 + 4,
                                     (22 * pop) / 100 + 4, true);
  maybeBow((int)faceX, (int)faceY, 2.0f);
}

// ۶ — خواب‌آلود
//
// ⚠️ بازطراحي شد. قبلا چشم را از وسط جمع مي‌کرديم (open=38).
//   ولي خواب‌آلودگي يعني **پلک افتاده** ، نه چشمِ کوچک.
//   حالا چشم کامل کشيده مي‌شود و يک پلکِ واقعي رويش مي‌آيد.
void drawSleepy(float t) {
  fbFill(C_BG);
  int oy  = (int)faceY + 10 + (int)(4.0f * sinf(t * 0.8f));
  int lid = 46 + (int)(7.0f * sinf(t * 1.1f));     // پلک بالا و پايين مي‌رود

  eyeRound(-1, (int)faceX, oy, 0, 8, 100, 52, 0);
  eyeRound(+1, (int)faceX, oy, 0, 8, 100, 52, 0);
  // پلکِ تقريبا صاف — کمي از سمت بيرون پايين‌تر
  slantLid(-1, (int)faceX, oy, 0, 8, 104, 104, lid - 6, lid);
  slantLid(+1, (int)faceX, oy, 0, 8, 104, 104, lid - 6, lid);

  drawBrows((int)faceX, oy, BR_SAD, 72);
  mouthLine((int)faceX, oy, 15, 3, C_TEAL);        // دهانِ بي‌حال
  maybeBow((int)faceX, oy, 0.35f);   // oy جابه‌جايي است ، نه مختصات مطلق
}

// خواب: چشم‌ها خط باريک ، نفس کشيدن آرام ، Z شناور
void drawAsleep(float t) {
  fbFill(C_BG);
  int breathe = (int)(5.0f * sinf(t * 0.9f));      // بالا و پايين رفتن نفس
  int oy = CY + 16 + breathe;
  int h  = 11 + (int)(2.0f * sinf(t * 0.9f));

  for (int sd = -1; sd <= 1; sd += 2)
    fbRoundRect(CX + sd * EYE_DX - 30, oy - h / 2, 60, h, h / 2, C_WHITE);

  // دهانِ کوچکِ باز که با نفس باز و بسته مي‌شود
  int mo = 7 + (int)(3.0f * sinf(t * 0.9f));
  int mx = CX, my = CY + MOUTH_Y + breathe;
  fbEllipse(mx, my, mo, mo, C_MOUTH);

  // ---------- آبِ دهان ----------
  //
  // ⚠️ نسخه‌ي اول يک رشته‌ي باريک با يک گلوله ته‌اش بود و
  //   اصلا شبيه قطره نمي‌شد. حالا از fbDrop استفاده مي‌کند:
  //   نوک‌تيز بالا ، گِرد و چاق پايين — شکلي که چشم به عنوان
  //   «قطره» مي‌شناسد.
  //
  // چرخه‌ي ۶ ثانيه‌اي:
  //   ۰ تا ۴٫۵   قطره از گوشه‌ي دهان آويزان است و بزرگ مي‌شود ،
  //              گردنِ باريکي نگهش داشته و کم‌کم کشيده مي‌شود
  //   ۴٫۵ تا ۶   گردن پاره مي‌شود و قطره شتاب‌دار مي‌افتد
  {
    float ct = fmodf(t, 6.0f);
    int   cx = mx + mo + 4;                  // گوشه‌ي راستِ دهان
    int   cy = my + 2;

    if (ct < 4.5f) {
      float f    = ct / 4.5f;
      int   neck = 5 + (int)(f * 15);        // گردن کشيده مي‌شود
      int   r    = 3 + (int)(f * 5);         // قطره سنگين‌تر مي‌شود

      // گردنِ باريک — هرچه قطره سنگين‌تر ، گردن نازک‌تر
      int nw = 3 - (int)(f * 2);
      if (nw < 1) nw = 1;
      fbRect(cx - nw, cy, nw * 2, neck, C_TEAR);

      fbDrop(cx, cy + neck + r, r, (int)(r * 1.6f), C_TEAR);
    } else {
      float f = (ct - 4.5f) / 1.5f;
      // f*f يعني شتاب‌دار — اول آرام ، بعد تند. مثل چيزي که
      // واقعا مي‌افتد ، نه با سرعت ثابت.
      int y = cy + 28 + (int)(f * f * 78);
      fbDrop(cx, y, 8, 14, C_TEAR);
      fbDrop(cx, cy + 4, 3, 5, C_TEAR);      // ته‌مانده‌ي گوشه‌ي دهان
    }
  }

  maybeBow(0, breathe, 0.25f);
  drawFloaters();
}

// =====================================================
//  حالت‌هاي تازه
// =====================================================

// ۱۰ — کنجکاو
//
// ايده: يک چشم بالاتر از ديگري ، مثل بالا انداختن يک ابرو.
// نگاه تند و کوتاه اين‌طرف و آن‌طرف مي‌رود ، و دهان يک
// «اوه»ي کوچک است. سر هم کمي کج مي‌شود.
void drawCurious(float t, int open) {
  fbFill(C_BG);
  int tilt = (int)(4.0f * sinf(t * 1.4f));
  int ox = (int)faceX, oy = (int)faceY;

  // نگاه پرشي — هر ۰٫۷ ثانيه يک جاي تازه
  int step = (int)(t / 0.7f);
  int lx = ((step % 3) - 1) * 18;
  int ly = ((step % 2) == 0) ? -6 : 4;
  setBrowGaze(lx, ly);            // ابرو هم با نگاهِ پرشي مي‌رود

  eyeRound(-1, ox, oy - 9 - tilt, lx, ly, open, 52, 2);   // اين يکي بالاتر
  eyeRound(+1, ox, oy + 2 + tilt, lx, ly, open, 52, 2);

  drawBrows(ox, oy, BR_ONE_UP, 78);          // يک ابرو بالا — «هوم؟»
  fbEllipse(CX + ox, CY + MOUTH_Y + oy - 14, 9, 12, C_TEAL);  // «اوه»ي کوچک
  maybeBow(ox, oy, 1.3f);
}

// =====================================================
//  فحشِ کارتوني — @ # $ % ! ※
//
//  در کارتون‌ها فحش را با اين نشانه‌ها نشان مي‌دهند. اسم
//  فني‌اش grawlix است. مزيتش اين است که همه مي‌فهمند چه
//  خبر است بدون اينکه کلمه‌اي گفته شود.
//
//  هر کدام را با خط و دايره مي‌سازيم ، چون فونت نداريم.
// =====================================================
void glyphHash(int x, int y, int s, uint16_t c) {      // #
  fbLine(x - s, y - s/3, x + s, y - s/3, 3, c);
  fbLine(x - s, y + s/3, x + s, y + s/3, 3, c);
  fbLine(x - s/3, y - s, x - s/2, y + s, 3, c);
  fbLine(x + s/2, y - s, x + s/3, y + s, 3, c);
}
void glyphStar(int x, int y, int s, uint16_t c) {      // *
  for (int k = 0; k < 3; k++) {
    float a = k * 3.14159f / 3.0f;
    fbLine(x - (int)(s*cosf(a)), y - (int)(s*sinf(a)),
           x + (int)(s*cosf(a)), y + (int)(s*sinf(a)), 3, c);
  }
}
void glyphBang(int x, int y, int s, uint16_t c) {      // !
  fbLine(x, y - s, x, y + s/3, 4, c);
  fbCircle(x, y + s - 1, 3, c);
}
void glyphAt(int x, int y, int s, uint16_t c) {        // @
  fbCircle(x, y, s, c);
  fbCircle(x, y, s - 3, C_BG);
  fbCircle(x, y, s/3, c);
}
void glyphPct(int x, int y, int s, uint16_t c) {       // %
  fbCircle(x - s/2, y - s/2, s/3, c);
  fbCircle(x + s/2, y + s/2, s/3, c);
  fbLine(x + s/2, y - s, x - s/2, y + s, 3, c);
}

// ۱۷ — بوس
//
// يک چشم `>` بسته (چشمک) و يکي `○` باز. همين عدمِ تقارن
// است که «بوس» را از «چشمکِ ساده» جدا مي‌کند.
// دهان يک لبِ غنچه‌ي کوچک ، و قلب‌هايي که بالا مي‌روند.
void drawKiss(float t) {
  fbFill(C_BG);
  int ox = (int)faceX + (int)(3.0f * sinf(t * 2.4f));
  int oy = (int)faceY;

  drawBrushEye(-1, ox, oy, ES_CARET, t);       // چشمک
  drawBrushEye(+1, ox, oy, ES_RING,  t);       // باز

  drawBrows(ox, oy, BR_CALM, 80);
  mouthKiss3(ox, oy, t);        // دهانِ «۳» خوابيده — از طرح
  blushHatch(ox, oy);
  int py = CY + oy + MOUTH_Y - 8;

  // قلب‌هايي که از کنارِ لب بالا مي‌روند و کوچک مي‌شوند
  for (int k = 0; k < 3; k++) {
    float ph = fmodf(t * 0.9f + k * 0.33f, 1.0f);
    int hx = CX + ox + 16 + (int)(ph * 52);
    int hy = py - (int)(ph * 118);
    int hr = 22 - (int)(ph * 13);
    if (hr > 4) fbHeart(hx, hy, hr, C_RED);
  }
  fbSparkle(CX + ox - EYE_DX - 46, CY + oy - 46, 11, C_STAR);
  maybeBow(ox, oy, 1.4f);
}

// ۱۸ — گريه
//
// فرقش با «ناراحت» شدت است: آن‌جا بغض بود ، اين‌جا گريه.
//   چشم‌ها بيشتر مي‌لرزند
//   اشک از هر دو چشم ، پشتِ سر هم
//   و کلِ صورت هق‌هق مي‌کند — پرشِ ريزِ نامنظم
void drawCry(float t) {
  fbFill(C_BG);

  // هق‌هق: دو موجِ با فرکانسِ نامرتبط روي هم. اگر يک موجِ
  // ساده باشد ، منظم و مکانيکي به نظر مي‌رسد.
  int sob = (int)(4.0f * sinf(t * 9.0f) + 2.0f * sinf(t * 3.7f));
  int ox = (int)faceX + (int)(2.0f * sinf(t * 11.0f));
  int oy = (int)faceY + 6 + sob;

  int j = (int)(2.4f * sinf(t * 16.0f));
  setBrowGaze(0, -14);
  eyeRound(-1, ox + j, oy, -7, -14, 94, 50, 2);
  eyeRound(+1, ox - j, oy,  7, -14, 94, 50, 2);
  drawBrows(ox, oy, BR_SAD, 72);

  // ---------- اشک‌ها ----------
  // هر چشم دو قطره با فازِ متفاوت ، تا جريان پيوسته به نظر
  // برسد نه قطره‌ي تکي.
  for (int sd = -1; sd <= 1; sd += 2) {
    for (int k = 0; k < 2; k++) {
      float ph = fmodf(t * 1.25f + k * 0.5f + (sd > 0 ? 0.27f : 0.0f), 1.0f);
      int tx = CX + sd * EYE_DX + ox + sd * 14;
      // شتابِ سقوط — سرعتش با فاصله بيشتر مي‌شود
      int ty = CY + oy + 14 + (int)(ph * ph * 150);
      int tr = 9 - (int)(ph * 3);
      if (tr > 3) fbDrop(tx, ty, tr, tr + 5, C_TEAR);
    }
  }

  mouthSadWave(ox, oy);
  maybeBow(ox, oy, 0.4f);
}

// ۱۹ — چشم‌غره
//
// نگاه به بالا ، آن حرکتي که يعني «باز هم؟». تکِ حرکتي
// است: چشم‌ها بالا مي‌روند ، يک لحظه مي‌مانند ، برمي‌گردند.
void drawEyeRoll(float t) {
  fbFill(C_BG);
  int ox = (int)faceX, oy = (int)faceY;

  // سه پرده در ۱٫۴ ثانيه
  float k = (t < 0.35f) ? (t / 0.35f)
          : (t < 0.95f) ? 1.0f
          : (t < 1.30f) ? (1.0f - (t - 0.95f) / 0.35f) : 0.0f;
  if (k < 0) k = 0;

  if (k > 0.75f) {
    // اوجِ چشم‌غره — فقط قوسِ رو به بالا ديده مي‌شود
    drawBrushEye(-1, ox, oy, ES_UP, t);
    drawBrushEye(+1, ox, oy, ES_UP, t);
  } else {
    // در راه ، مردمک‌ها بالا مي‌روند
    int up = -(int)(k * 26);
    int lid = 100 - (int)(k * 42);
    setBrowGaze(0, up);
    eyeRound(-1, ox, oy, 0, up, lid, 52, 2);
    eyeRound(+1, ox, oy, 0, up, lid, 52, 2);
  }

  drawBrows(ox, oy, BR_FLAT, 80);           // ابروي صاف — بي‌حوصلگي
  mouthLine(ox, oy, 22, 4, C_TEAL);         // دهانِ صافِ بي‌حوصله
  maybeBow(ox, oy, 0.8f);
}

// ۲۰ — قلبِ دو انگشتي
//
// موچي با دستکشش قلبِ دو انگشتي نشان مي‌دهد — شست و
// اشاره که سرشان به هم مي‌رسد و يک قلبِ کوچک مي‌سازند.
//
// دستکش را از خميازه داريم ، ولي اين‌جا انگشت‌ها فرق دارند
// پس جدا مي‌کشيمش.
void drawHeartHand(float t) {
  fbFill(C_BG);
  int ox = (int)faceX, oy = (int)faceY - 10;

  drawBrushEye(-1, ox, oy, ES_HEART, t, C_RED, 82);
  drawBrushEye(+1, ox, oy, ES_HEART, t, C_RED, 82);
  drawBrows(ox, oy, BR_CALM, 80);
  mouthTiny(ox, oy - 14, 16, 9);

  // ---------- دستِ قلب‌ساز ----------
  //
  // دو بار اشتباه کردم تا فهميدم نکته کجاست:
  //   بار اول : دست بزرگ و تپل بود ، قلب گم مي‌شد
  //   بار دوم : بهتر شد ولي هنوز «دست» خوانده نمي‌شد
  //
  // مشکل اين بود که دست را از اجزاي **جدا** مي‌ساختم و
  // اميد داشتم کنار هم دست شوند. ولي آن ژست يک چيز را
  // بايد بگويد: **دو انگشتِ خميده که نوکشان تقريبا به هم
  // مي‌رسد و بينشان يک شکافِ قلبي مي‌ماند.**
  //
  // پس حالا برعکس مي‌سازمش: اول يک دستِ يک‌پارچه‌ي گِرد ،
  // بعد با رنگِ پس‌زمينه **شکافِ بينِ دو انگشت را مي‌بُرم**.
  // بريدن ، کشيدن را خواناتر مي‌کند — چون فضاي خالي است
  // که شکل را مي‌سازد.
  int hx = CX + ox + 92;
  int hy = CY + oy + 86 + (int)(4.0f * sinf(t * 3.0f));

  // ساعد ، رو به پايين-چپ (از بدنِ فرضي مي‌آيد)
  fbEllipseRot(hx + 16, hy + 54, 11, 30, -0.30f, C_WHITE);
  // مشت — بدنه‌ي اصليِ دست
  fbCircle(hx, hy + 14, 25, C_WHITE);
  // انگشتِ اشاره: از بالاي مشت به سمت بالا-چپ
  fbEllipseRot(hx - 13, hy - 12, 9, 23, -0.42f, C_WHITE);
  // شست: از کنارِ مشت به سمت بالا-راست
  fbEllipseRot(hx + 15, hy - 6,  9, 19,  0.62f, C_WHITE);

  // ✂️ شکافِ بينِ دو انگشت — همان چيزي که ژست را مي‌سازد
  fbEllipseRot(hx + 1, hy - 24, 8, 13, 0.15f, C_BG);

  // قلبِ تپنده ، نشسته در همان شکاف و کمي بالاتر
  int hr = 30 + (int)(4.0f * sinf(t * 5.5f));
  fbHeart(hx + 1, hy - 34, hr, C_HOT);

  // دو قلبِ بالارونده
  for (int k = 0; k < 2; k++) {
    float ph = fmodf(t * 0.8f + k * 0.5f, 1.0f);
    int r = 19 - (int)(ph * 12);
    if (r > 5) fbHeart(hx + 14 + (int)(ph * 26), hy - 66 - (int)(ph * 74), r, C_HOT);
  }
  maybeBow(ox, oy, 1.5f);
}

// ۲۱ — غذا خوردن
//
// سه پرده: دهان باز مي‌شود ، مي‌جَوَد ، راضي مي‌شود.
void drawFeed(float t) {
  fbFill(C_BG);
  int ox = (int)faceX, oy = (int)faceY;

  if (t < 1.5f) {
    // جويدن — دهان باز و بسته ، چشم‌ها ⌒
    drawBrushEye(-1, ox, oy, ES_ARC_UP, t);
    drawBrushEye(+1, ox, oy, ES_ARC_UP, t);
    drawBrows(ox, oy, BR_CALM, 80);
    int chew = (int)(9.0f * fabsf(sinf(t * 8.0f)));
    fbEllipse(CX + ox, CY + MOUTH_Y + oy - 14, 20, 10 + chew, C_HOT);
  } else {
    // راضي — چشمِ > < و لبخندِ پهن
    drawBrushEye(-1, ox, oy, ES_CARET, t);
    drawBrushEye(+1, ox, oy, ES_CARET, t);
    drawBrows(ox, oy, BR_UP, 82);
    mouthTiny(ox, oy, 18, 10);
    for (int i = 0; i < 3; i++) {
      float a = t * 2.4f + i * 2.094f;
      fbSparkle(CX + ox + (int)(120 * cosf(a)),
                CY + oy - 20 + (int)(96 * sinf(a)), 11, C_STAR);
    }
  }
  blushOval(ox, oy, 28);
  maybeBow(ox, oy, 1.4f);
}

// ۲۲ — رنگين‌کمان
//
// حالتِ «واو!» — چشم‌هاي گشاد با ابروي بالا رفته و يک
// آبشارِ رنگين‌کمان از دهان. در طرحِ مرجع اين بامزه‌ترين
// حالت است.
void drawRainbow(float t) {
  fbFill(C_BG);
  int ox = (int)faceX, oy = (int)faceY - 18;

  eyeRound(-1, ox, oy, 0, -4, 100, 56, 3);
  eyeRound(+1, ox, oy, 0, -4, 100, 56, 3);
  drawBrows(ox, oy, BR_UP, 82);

  // ---------- آبشار ----------
  // هفت نوارِ رنگي که از دهان پايين مي‌ريزند و پهن مي‌شوند.
  // مثل شعله ، موجِ هر نوار فازِ خودش را دارد تا جريانْ
  // زنده به نظر برسد نه يک بلوکِ رنگيِ ثابت.
  const uint16_t RB[7] = {
    rgb(228, 40, 44), rgb(255,140, 20), rgb(255,220, 40),
    rgb( 60,200, 70), rgb( 40,150,235), rgb( 70, 70,200),
    rgb(150, 60,190)
  };
  int top = CY + oy + 76;
  int botY = LCD_H - 6;
  // 🛡 اگر صورت خيلي پايين برود ، top از botY رد مي‌شود و
  //    مخرجِ کسر صفر يا منفي مي‌شود.
  if (botY - top < 8) top = botY - 8;

  for (int y = top; y < botY; y++) {
    float u = (float)(y - top) / (float)(botY - top);   // ۰ بالا ، ۱ پايين
    // پهنا از ۱۸ به ۶۰ باز مي‌شود
    int hw = 18 + (int)(u * 42);
    // تابِ ملايمِ کلِ جريان
    int sway = (int)(7.0f * sinf(u * 3.4f + t * 2.6f));
    int x0 = CX + ox + sway - hw;
    for (int k = 0; k < 7; k++) {
      int xa = x0 + (hw * 2) * k / 7;
      int xb = x0 + (hw * 2) * (k + 1) / 7;
      for (int x = xa; x < xb; x++) px(x, y, swap16(RB[k]));
    }
  }

  // چند قطره‌ي پراکنده در دو طرفِ جريان
  for (int k = 0; k < 4; k++) {
    float ph = fmodf(t * 1.3f + k * 0.25f, 1.0f);
    int sgn = (k % 2) ? 1 : -1;
    int dx = CX + ox + sgn * (52 + (int)(ph * 34));
    int dy = top + 26 + (int)(ph * 150);
    fbCircle(dx, dy, 7 - (int)(ph * 4), RB[(k * 2) % 7]);
  }
  maybeBow(ox, oy, 1.8f);
}

// ۱۶ — عصباني
//
// سه چيز عصبانيت را مي‌سازد ، و هر سه لازم‌اند:
//   ۱ ابروهاي شيب‌دار به سمت **داخل-پايين**
//     (دقيقا برعکسِ ناراحت که بيرون-پايين است)
//   ۲ دودِ دماغ — دو ستون که بالا مي‌روند و پخش مي‌شوند
//   ۳ فحشِ کارتوني که دور سر مي‌چرخد
//
// و لرزشِ ريزِ کلِ صورت. عصبانيت ساکن نيست.
void drawAngry(float t, int open) {
  fbFill(C_BG);

  // لرزش — سريع و کوچک. بزرگ‌تر از ۳ پيکسل مسخره مي‌شود.
  int sh = (int)(2.5f * sinf(t * 31.0f));
  int ox = (int)faceX + sh;
  int oy = (int)faceY + (int)(1.5f * sinf(t * 27.0f));

  // 🔥 چشمِ شعله‌اي — از ورقِ طراحي. خيلي گوياتر از دودِ
  //    دماغ است ، چون **خودِ چشم** عصبانيت را مي‌گويد نه
  //    يک تزيينِ کنارش.
  eyeFlame(-1, ox, oy, t);
  eyeFlame(+1, ox, oy, t);

  // ابروي اخم — شيب به سمت داخل-پايين.
  // ☠️ جهتش مهم است: بيرون-پايين مي‌شود غم ، داخل-پايين
  //    مي‌شود اخم. اگر برعکس بزني ، به جاي عصباني
  //    بغض‌کرده به نظر مي‌رسد.
  // ابروي اخمِ سرخ ، هم‌رنگِ شعله
  eyeBrowShape(-1, ox, oy, BR_ANGRY, 96);
  eyeBrowShape(+1, ox, oy, BR_ANGRY, 96);

  // دهانِ فرياد — مستطيلِ گِرد با دندان‌هاي اره‌اي
  int mw = 46, mh = 30 + (int)(6 * sinf(t * 14.0f));   // باز و بسته مي‌شود
  fbRoundRect(CX + ox - mw/2, CY + oy + MOUTH_Y - mh/2, mw, mh, 10, C_MOUTH);
  for (int k = -1; k <= 1; k++) {                       // سه دندان بالا
    int tx = CX + ox + k * 15;
    int ty = CY + oy + MOUTH_Y - mh/2;
    fbLine(tx - 6, ty + 1, tx, ty + 9, 4, C_WHITE);
    fbLine(tx, ty + 9, tx + 6, ty + 1, 4, C_WHITE);
  }

  // ❌ دودِ دماغ حذف شد.
  //    دو دليل: اول اينکه پف‌ها **توخالي** کشيده مي‌شدند
  //    (دايره‌ي پُر ، بعد دايره‌ي کوچک‌تر با رنگِ پس‌زمينه)
  //    و همان دايره‌ي سياه روي شعله‌ها سوراخ مي‌زد.
  //    دوم اينکه حالا شعله خودش حرف را مي‌زند و دود فقط
  //    شلوغي بود.

  // ---------- فحش ----------
  // پنج نشانه که دورِ سر مي‌چرخند. هرکدام با سرعتِ کمي
  // متفاوت ، وگرنه مثل يک حلقه‌ي سفت به نظر مي‌رسد.
  for (int k = 0; k < 5; k++) {
    float a = t * (1.5f + k * 0.13f) + k * 1.257f;
    int   r = 132 + (int)(8 * sinf(t * 3.0f + k));
    int   gx = CX + ox + (int)(r * cosf(a));
    int   gy = CY + oy - 30 + (int)(r * 0.42f * sinf(a));
    int   gs = 11 + (int)(2 * sinf(t * 5.0f + k));
    uint16_t c = (k % 2) ? C_RED : rgb(255, 190, 60);
    switch (k) {
      case 0: glyphHash(gx, gy, gs, c); break;
      case 1: glyphStar(gx, gy, gs, c); break;
      case 2: glyphBang(gx, gy, gs, c); break;
      case 3: glyphAt  (gx, gy, gs, c); break;
      case 4: glyphPct (gx, gy, gs, c); break;
    }
  }

  maybeBow(ox, oy, 2.2f);      // پاپيون هم تکان تندي مي‌خورد
}

// ۱۵ — گوش مي‌دهد
//
// تفاوتش با کنجکاو عمدي است:
//   کنجکاو  → نگاه مي‌پرد ، سر تکان مي‌خورد ، دنبال چيزي مي‌گردد
//   گوش‌دادن → **بي‌حرکت** مي‌شود ، سر را کج مي‌کند ، خيره مي‌ماند
// وقتي آدم دارد دقيق گوش مي‌دهد ، ساکن مي‌شود. حرکتِ کم
// اينجا خودش پيام است.
//
// سه چيز آن را مي‌سازد:
//   ۱ کجيِ ثابتِ سر — انگار گوشش را جلو آورده
//   ۲ بدنِ صورت کمي به همان سمت سُر مي‌خورد
//   ۳ سه کمانِ موجِ صدا که يکي‌يکي از کنار مي‌آيند
void drawListen(float t, int open) {
  fbFill(C_BG);

  // کجي ثابت است ، فقط نفسِ خيلي آرامي دارد
  int tilt = 7 + (int)(1.5f * sinf(t * 0.9f));
  int lean = 6;                       // تمايل به سمت صدا
  int ox = (int)faceX + lean, oy = (int)faceY;

  // نگاه به همان سمت ، و **ثابت** — نه پرشي مثل کنجکاو
  int lx = 12, ly = -3;
  setBrowGaze(lx, ly);

  // پلکِ کمي پايين‌افتاده: نشانه‌ي تمرکز ، نه خواب‌آلودگي.
  // ۸۵٪ باز ، نه کمتر — وگرنه خواب‌آلود به نظر مي‌رسد.
  int lid = open - (open * 15) / 100;

  eyeRound(-1, ox, oy - tilt, lx, ly, lid, 52, 2);
  eyeRound(+1, ox, oy + tilt, lx, ly, lid, 52, 2);

  drawBrows(ox, oy, BR_ONE_UP, 76);
  // دهانِ کوچکِ بسته و صاف — حواسش جاي ديگري است
  mouthLine(ox, oy + 2, 22, 4, C_TEAL);

  // ---------- موج‌هاي صدا ----------
  // سه کمان که يکي‌يکي از سمتِ صدا مي‌آيند و محو مي‌شوند.
  // هر کدام ۰٫۴۵ ثانيه عقب‌تر از قبلي ، پس حس مي‌شود که
  // موج دارد نزديک مي‌شود.
  for (int k = 0; k < 3; k++) {
    float ph = fmodf(t * 1.1f - k * 0.45f, 1.6f);
    if (ph < 0 || ph > 1.0f) continue;          // بيرون از نوبتش
    int   r  = 96 + k * 22 + (int)(ph * 10);
    // کمان فقط وقتي پررنگ است که تازه آمده باشد
    uint16_t c = (ph < 0.75f) ? C_WHITE : C_BG;
    if (c == C_BG) continue;
    // يک ربعِ کمان در سمت راست ، از بالا تا پايين
    for (int a = -34; a <= 34; a += 2) {
      float rad = a * 3.14159f / 180.0f;
      int x = ox + (int)(r * cosf(rad));
      int y = oy + (int)(r * sinf(rad));
      fbCircle(x, y, 2, c);
    }
  }

  maybeBow(ox, oy, 1.0f);
}

// ۱۱ — در فکر
//
// ايده: نگاه به بالا-چپ (همان کاري که آدم موقع فکر کردن
// مي‌کند) ، پلک نيمه‌افتاده ، و سه نقطه که يکي‌يکي ظاهر
// مي‌شوند — مثل حباب فکر.
void drawThink(float t, int open) {
  fbFill(C_BG);
  int ox = (int)faceX, oy = (int)faceY;

  setBrowGaze(-20, -13);          // نگاهِ بالا-چپِ فکر کردن
  eyeRound(-1, ox, oy, -20, -13, open, 52, 2);
  eyeRound(+1, ox, oy, -20, -13, open, 52, 2);
  slantLid(-1, ox, oy, -20, -13, 104, 104, 22, 26);
  slantLid(+1, ox, oy, -20, -13, 104, 104, 22, 26);

  // سه نقطه ، هر ۰٫۵ ثانيه يکي
  int dots = ((int)(t / 0.5f)) % 4;
  for (int i = 0; i < dots; i++)
    fbCircle(CX + 60 + i * 24 + ox, CY - 112 + oy, 7, C_WHITE);

  drawBrows(ox, oy, BR_ONE_UP, 76);
  mouthLine(ox + 10, oy, 13, 3, C_TEAL);          // دهانِ جمع‌شده به يک طرف
  maybeBow(ox, oy, 0.7f);
}

// ۱۲ — خجالت‌زده
//
// ايده: نگاه پايين و به کنار ، پلک نيمه‌افتاده ، گونه‌هاي
// پررنگ و پهن ، دهان خيلي کوچک ، و صورت کمي رو برگردانده.
void drawShy(float t, int open) {
  fbFill(C_BG);
  int ox = (int)faceX + 8 + (int)(3.0f * sinf(t * 1.8f));
  int oy = (int)faceY + 6;

  // ⌒ کوچک — چشمِ جمع‌شده‌ي خجالت
  drawBrushEye(-1, ox, oy, ES_ARC_UP, t, C_WHITE, 74);
  drawBrushEye(+1, ox, oy, ES_ARC_UP, t, C_WHITE, 74);

  drawBrows(ox, oy, BR_SAD, 78);
  blushHatch(ox, oy, 118);                        // گونه‌ي سه‌خطيِ پررنگ
  mouthTiny(ox, oy, 11, 6);                       // لبخند خيلي کوچک
  maybeBow(ox, oy, 1.1f);
}

// ۱۳ — ذوق‌زده
//
// ايده: چشم‌هاي گشاد و براق ، دهانِ باز خندان ، و کل صورت
// بالا و پايين مي‌پرد. برق‌ها دور سر مي‌چرخند.
void drawExcited(float t) {
  fbFill(C_BG);
  int bounce = -(int)(11.0f * fabsf(sinf(t * 7.0f)));
  int ox = (int)faceX, oy = (int)faceY + bounce;

  // > < — نوکِ زاويه رو به بيرون. چشمِ خنده‌ي محکم.
  drawBrushEye(-1, ox, oy, ES_CARET, t);
  drawBrushEye(+1, ox, oy, ES_CARET, t);

  drawBrows(ox, oy, BR_UP, 84);
  mouthTongue(ox, oy, t);                         // زبانِ بيرون — شيطنت

  for (int i = 0; i < 4; i++) {                   // ستاره‌هاي دور سر
    float a = t * 4.0f + i * 1.571f;
    fbSparkle(CX + (int)(134 * cosf(a)) + ox,
              CY - 30 + (int)(100 * sinf(a)) + oy, 12, C_STAR);
  }
  maybeBow(ox, oy, 2.0f);
}

// ۱۴ — خميازه و کش آمدن
//
// وقتي از خوابِ عميق بيدارش کني. چهار پرده:
//   ۰٫۰ تا ۰٫۵  دهان باز مي‌شود ، دست‌ها شروع به کش آمدن
//   ۰٫۵ تا ۱٫۷  اوج — دهان کاملا باز ، دست‌ها کشيده ، چشم‌ها
//                محکم بسته (قوسِ رو به بالا = فشرده شده)
//   ۱٫۷ تا ۲٫۳  جمع شدن
//   ۲٫۳ تا ۳٫۰  چشم‌ها آرام باز مي‌شوند
//
// «دست» روي اين چهره واقعي نيست ، ولي حرکتِ کش آمدن از دو
// طرف ، حسِ خميازه را کامل مي‌کند.
void drawYawn(float t) {
  fbFill(C_BG);

  float openAmt, stretch;
  int   eo = 0;                       // بازشدگيِ چشم

  if (t < 0.5f)       { float f = t / 0.5f;          openAmt = f * 0.7f; stretch = f * 0.5f; }
  else if (t < 1.7f)  {                              openAmt = 1.0f;     stretch = 1.0f;     }
  else if (t < 2.3f)  { float f = (t - 1.7f) / 0.6f; openAmt = 1 - f;    stretch = 1 - f;    }
  else                { float f = (t - 2.3f) / 0.7f;
                        if (f > 1) f = 1;
                        openAmt = 0; stretch = 0; eo = (int)(f * 100); }

  int oy = (int)(6.0f * openAmt);     // موقع خميازه کمي جمع مي‌شود

  // ---------- دست‌ها ----------
  //
  // ⚠️ نسخه‌ي اول يک خطِ صاف با توپ ته‌اش بود — شبيه آبنبات‌چوبي.
  //   دستِ طبيعي سه چيز دارد که آن نداشت:
  //     ۱. **قوس** — بازو صاف نيست
  //     ۲. **باريک شدن** — شانه کلفت ، مچ نازک
  //     ۳. **دستِ کشيده و زاويه‌دار** — نه يک دايره
  //
  //   مسير بازو يک منحنيِ درجه‌دو (بزيه) است: از شانه ، با يک
  //   نقطه‌ي کنترل که بالاتر از وسط گذاشته شده تا قوس بگيرد ،
  //   تا مچ. روي آن دايره‌هايي با شعاعِ کم‌شونده مي‌گذاريم.
  if (stretch > 0.02f) {
    for (int sd = -1; sd <= 1; sd += 2) {
      float x0 = CX + sd * 52, y0 = CY + 94;             // شانه
      float x2 = x0 + sd * stretch * 72;                 // مچ
      float y2 = y0 - stretch * 92;
      float x1 = (x0 + x2) * 0.5f + sd * 14;             // نقطه‌ي کنترل
      float y1 = (y0 + y2) * 0.5f - 24;

      const int N = 20;
      for (int i = 0; i <= N; i++) {
        float u = (float)i / N, v = 1.0f - u;
        float x = v*v*x0 + 2*v*u*x1 + u*u*x2;
        float y = v*v*y0 + 2*v*u*y1 + u*u*y2;
        fbCircle((int)x, (int)y, 9 - (int)(u * 4.0f), C_WHITE);  // ۹ → ۵
      }

      // زاويه‌ي مچ = جهتِ انتهاي منحني
      float ang = atan2f(y2 - y1, x2 - x1);
      drawGlove(x2, y2, ang, sd, C_WHITE);
    }
  }

  // ---------- چشم‌ها ----------
  if (eo == 0) {
    // محکم بسته — قوسِ رو به بالا ، عمقِ منفي
    int dw, dh, dy, dp;
    for (int sd = -1; sd <= 1; sd += 2) {
      eyeSkew(sd, dw, dh, dy, dp);
      fbCurve(CX + sd * EYE_DX, CY + oy + dy, 30 + dw, -14, 7, C_WHITE);
    }
  } else {
    eyeRound(-1, 0, oy, 0, 4, eo, 52, 0);
    eyeRound(+1, 0, oy, 0, 4, eo, 52, 0);
  }

  // ---------- دهان ----------
  int rx = 11 + (int)(openAmt * 16);
  int ry =  6 + (int)(openAmt * 32);
  mouthOpen(0, oy, rx, ry, openAmt > 0.5f);

  maybeBow(0, oy, 0.6f);
}

// =====================================================
//  منطق حالت‌ها
// =====================================================
uint32_t lastZ = 0, lastPetHeart = 0;
int blNow = BL_AWAKE;      // نور فعلي — بيرون از تابع ، تا بيدار شدن هم بتواند عوضش کند

// خوشحالي بي‌دليل — با زمان کنترل مي‌شود ، نه با احتمالِ هر فريم
//
// ⚠️ درسي که از اولين تست گرفتيم:
//   اول نوشته بودم  if (random(1000) < 3)
//   ولي اين هر فريم اجرا مي‌شود ، نه هر ثانيه. با ۳۵ فريم
//   در ثانيه يعني حدود ۱۰٪ شانس در هر ثانيه — موچي هر چند
//   ثانيه بي‌دليل ذوق مي‌کرد و مصنوعي به نظر مي‌رسيد.
//   هر «شانس»ي که در حلقه‌ي اصلي مي‌گذاري بايد بر حسب زمان
//   باشد ، نه بر حسب فريم.
uint32_t nextHappyAt = 0;
const uint32_t HAPPY_GAP_MIN = 35000;   // کمينه فاصله‌ي دو خوشحالي بي‌دليل
const uint32_t HAPPY_GAP_MAX = 90000;   // بيشينه

void moodStep() {
  uint32_t now = millis();
  uint32_t idle = now - lastTouchOrMove;

  // ---------- واکنش‌هاي فوري (اولويت با شديدترين) ----------
  if (evImpact || evAnnoy) {
    // پله‌ي احساسي: بار اول ناراحت مي‌شود ، ولي اگر وقتي
    // هنوز ناراحت است باز اذيتش کني ، عصباني مي‌شود.
    // همان الگوي «همان ورودي ، دو واکنش» که در سقلمه و
    // دست زدن هم هست.
    // ☠️ پله‌ي عصبانيت دو بار درست نشد ، و حالا علتِ واقعي را
    //    مي‌فهمم: شرط را به **حالتِ لحظه‌اي** بسته بودم
    //    (`mood == M_HURT`). ولي ناراحتي فقط ۴٫۲ ثانيه طول
    //    مي‌کشد و سه سقلمه‌ي بعدي بايد داخل همان پنجره‌ي تنگ
    //    جا شوند ، وگرنه mood رفته روي M_IDLE و همه‌چيز از
    //    صفر شروع مي‌شود. کاربر گزارش داد «سه تاي بعدي
    //    خوشحال مي‌شه» — دقيقا همين.
    //
    //    راهِ درست: پله را به **خاطره** ببند ، نه به حالت.
    //    يعني «آيا تازگي ناراحتش کرده‌ام؟» نه «آيا همين
    //    الان ناراحت است؟»
    static uint32_t lastHurtAt = 0;
    const uint32_t GRUDGE_WINDOW = 15000;

    if (evImpact) soulImpact();
    if (mood == M_ANGRY) {
      setMood(M_ANGRY, T_ANGRY);                       // تمديد
    // ☠️ و يک اشتباهِ کلاسيک که همين‌جا افتاد:
    //      lastHurtAt با صفر شروع مي‌شود. در ثانيه‌هاي اولِ
    //      روشن شدن ، millis() هم کوچک است ، پس
    //          now - 0  <  15000   ←  درست!
    //      يعني موچي **در همان لحظه‌ي روشن شدن** فکر مي‌کرد
    //      تازه ناراحتش کرده‌اي. در لاگ: سه ضربه‌ي اول
    //      مستقيم «عصباني» شد ، بدون «ناراحت».
    //
    //      درس: «تا حالا اتفاق نيفتاده» را با صفر نشان نده ،
    //      وقتي صفر خودش يک زمانِ معتبر است.
    } else if (mood == M_HURT
               || (lastHurtAt && (now - lastHurtAt) < GRUDGE_WINDOW)) {
      soulAngry();
      setMood(M_ANGRY, T_ANGRY);
      SHOW_SET(SET_ANGRY, T_ANGRY);
      forceBlinkSoon();
    } else {
      lastHurtAt = now;
      setMood(M_HURT, T_HURT);
      SHOW_SET(SET_HURT, T_HURT);
      forceBlinkSoon();
    }
    faceTX = faceTY = 0;
    return;
  }

  // ☠️ اين بلوک **جابه‌جا شد** ، و علتش يکي از آن باگ‌هايي
  //    است که فقط با ديدنِ ترتيبِ کد پيدا مي‌شود نه با
  //    خواندنِ تک‌تکِ بخش‌ها:
  //
  //      ترتيبِ قبلي:  اذيت → نوازش → سقلمه → حرکت‌ها
  //
  //    و کشيدنِ انگشت روي صفحه **هم نوازش است هم حرکت**.
  //    نوازش زودتر بررسي مي‌شد و return مي‌کرد ، پس نوبتِ
  //    حرکت‌ها هرگز نمي‌رسيد. کشيدن به پايين (قلبِ دو
  //    انگشتي) و نگه داشتن (بوس) هيچ‌وقت اجرا نمي‌شدند.
  //
  //    و چرا گاهي کار مي‌کرد؟ اگر خيلي سريع مي‌کشيدي
  //    (کمتر از ۴۰۰ ميلي‌ثانيه) نوازش حساب نمي‌شد و حرکت
  //    جان سالم به در مي‌برد. همين تصادفي بودن ، پيدا
  //    کردنش را سخت کرد.
  //
  //    قاعده: حرکت‌هاي **گسسته و عمدي** بايد قبل از
  //    حالت‌هاي **پيوسته** بررسي شوند. چيپ خودش حرکت را
  //    تشخيص داده و نبايد رايش را دور بريزيم.
  // ---------- حرکت‌هاي چيپ لمسي ----------
  //
  //   يک ضربه      → توجه مي‌کند (بسته به رابطه‌تان)
  //   دو ضربه      → غذا! ذوق مي‌کند و مي‌خورد
  //   نگه داشتن    → بوس
  //   کشيدن به بالا → چشم‌غره ــ «باز هم؟»
  //   کشيدن به پايين → قلبِ دو انگشتي
  // ☠️ باگي که خودم ساختم و نزديک بود «عصباني» را
  //    دست‌نيافتني کند:
  //      M_HURT در اين فهرست نبود. پس وقتي موچي ناراحت بود
  //      و سقلمه‌ي چهارم مي‌خورد ، اين بلوک اول اجرا مي‌شد و
  //      حالت را به «کنجکاو» مي‌برد. سقلمه‌ي ششم آن وقت
  //      مي‌ديد که mood ديگر M_HURT نيست ، پس دوباره
  //      M_HURT مي‌گذاشت نه M_ANGRY.
  //      يعني هر چقدر هم سقلمه مي‌زدي ، هيچ‌وقت عصباني
  //      نمي‌شد.
  //
  //    قاعده‌ي کلي‌تر: حالت‌هاي «پله‌اي» بايد در برابر
  //    ورودي‌هاي عموميِ ديگر محافظت شوند ، وگرنه پله‌شان
  //    هيچ‌وقت بالا نمي‌رود.
  if (evGesture != TG_NONE && mood != M_DIZZY && mood != M_YAWN
                           && mood != M_ANGRY && mood != M_CRY
                           && mood != M_HURT) {
    // اگر خواب است ، هر حرکتي اول بيدارش مي‌کند
    if (mood == M_ASLEEP || mood == M_SLEEPY) {
      setMood(mood == M_ASLEEP ? M_YAWN : M_SURPRISE,
              mood == M_ASLEEP ? T_YAWN : T_SURPRISE);
      blNow = BL_AWAKE;  lcdBacklight(BL_AWAKE);
      return;
    }
    switch (evGesture) {
      case TG_TAP:
        // همان ورودي ، چند واکنش — بسته به حالِ قبلي و رابطه‌تان
        if (mood == M_HAPPY || mood == M_EXCITED) setMood(M_EXCITED, T_EXCITED);
        else if (soul.grudge > 500)               setMood(M_EYEROLL, T_EYEROLL);
        else if (soul.bond   > 400)               setMood(M_HAPPY,   2200);
        else { setMood(M_SURPRISE, T_SURPRISE);
               SHOW_SET(SET_SURPRISE, T_SURPRISE); }
        return;
      case TG_DOUBLE: {
        soulPet(1500);                       // غذا هم محبت است
        setMood(M_FEED, T_FEED);
        //   ☠️ اين آرايه‌ي محلي شش چهره‌ي ديگرِ خوراک را دور
        //      مي‌زد. هر چهره بايد از **يک** جا انتخاب شود ،
        //      وگرنه سيستمِ توزيعِ منصفانه بي‌اثر مي‌شود.
        SHOW_SET(SET_FOOD, T_FEED);
        return;
      }
      case TG_HOLD:
        soulLove();
        setMood(M_KISS, T_KISS);
        SHOW_SET(SET_LOVE, T_KISS);
        return;
      case TG_UP:
        setMood(M_EYEROLL, T_EYEROLL);
        showFace(faceByName(soul.grudge > 400 ? "چشم‌غره" : "بي‌حوصله"), T_EYEROLL);
        return;
      case TG_DOWN:
        soulLove();
        setMood(M_HEARTHAND, T_HEART);
        return;
      case TG_LEFT:
        // کشيدنِ تند روي صورتش — مثل هُل دادن. عصباني مي‌شود.
        //
        // ☠️ اول soulAngry() هم صدا مي‌زدم. ولي در لاگ ديدم
        //    که همين ، رنجش را در دو دقيقه به ۹۹۳ رساند —
        //    چون اين حرکت در عمل **ابزارِ آزمايش** است نه
        //    آزار ، و کاربر ده بار پشت سر هم مي‌زندش.
        //    حالا فقط چهره عوض مي‌شود ، خاطره نه.
        setMood(M_ANGRY, T_ANGRY);
        return;
      case TG_RIGHT:
        setMood(M_RAINBOW, T_RAINBOW);
        SHOW_SET(SET_DRESSUP, T_RAINBOW);   // لباسِ تازه
        return;
      default: break;
    }
  }

  if (evShake) {
    setMood(M_DIZZY, T_DIZZY);
    return;
  }
  // خميازه هم مثل سرگيجه و درد قطع نمي‌شود — وگرنه اگر
  // انگشتت را روي صفحه نگه داري تا بيدارش کني ، وسط خميازه
  // مي‌پرد به حالت نوازش
  // ☠️ M_CRY اين‌جا نبود و در لاگ ديده شد:
  //      18:35:25.256  [حالت] گريه
  //      18:35:25.291  [حالت] نوازش       ← ۳۵ ميلي‌ثانيه بعد!
  //    انگشت هنوز روي صفحه بود و گريه را فوري خورد.
  // 🛡 M_ASLEEP و M_SLEEPY هم اضافه شدند. بدونشان ، نوازشِ
  //    موچيِ خواب مستقيم «لوس شده» مي‌شد و بلوکِ بيدار شدن
  //    (که پايين‌تر است) هيچ‌وقت نوبتش نمي‌رسيد — يعني نه
  //    خميازه مي‌کشيد نه نورِ صفحه برمي‌گشت.
  if (strokeNow && !gestureLock
                && mood != M_DIZZY && mood != M_HURT && mood != M_YAWN
                && mood != M_ANGRY && mood != M_CRY
                && mood != M_ASLEEP && mood != M_SLEEPY) {
    // نوازش پيوسته‌ي طولاني → عاشق مي‌شود
    if (mood != M_LOVE && now - strokeSince > soulPetToLove(T_PET_TO_LOVE)) {
      soulLove();
      setMood(M_LOVE, T_LOVE);
      SHOW_SET(SET_LOVE, T_LOVE);
      return;
    }
    if (mood != M_PET && mood != M_LOVE) {
      setMood(M_PET, T_PET_LINGER);
      SHOW_SET(SET_PET, 0);                       // چهره‌ي نوازشِ اين بار
    }
    else moodUntil = now + T_PET_LINGER;          // تا وقتي ادامه دارد بماند

    // صورت به سمت انگشت تکيه مي‌دهد
    faceTX = (touch.x - LCD_W / 2) * 0.10f;
    faceTY = (touch.y - LCD_H / 2) * 0.05f;
    if (now - lastPetHeart > 900) {
      lastPetHeart = now;
      soulPet(900);                    // هر ~ثانيه نوازش ، کمي پيوند
      spawnFloater(CX + random(-70, 71), CY - 40, 0);
    }
    return;
  }
  // سقلمه وقتي آرام است → جا مي‌خورد
  // سقلمه وقتي **قبلا خوشحال بوده** → ذوق مي‌کند ، نه ترس
  // همان ورودي ، دو واکنش — بسته به حالِ قبلي‌اش
  // ❌ واکنشِ سقلمه از اين‌جا برداشته شد.
  //
  //    يک ضربه‌ي انگشت **دو بار** پردازش مي‌شد:
  //      ۱ سقلمه‌ي خودمان (لحظه‌ي رها شدنِ انگشت)
  //      ۲ حرکتِ TG_TAP خودِ چيپ (يکي دو فريم بعد)
  //    و چون در دو فريمِ جدا مي‌افتادند ، هر دو اجرا مي‌شدند.
  //    نتيجه در لاگ: «جا خورد» و بلافاصله «خوشحال».
  //
  //    تشخيصِ خودِ چيپ بهتر است (نويزگيري شده و شامل دو
  //    ضربه و نگه داشتن هم مي‌شود) ، پس واکنش را به آن
  //    سپرديم. `evPoke` هنوز هست ولي فقط شمارنده‌ي
  //    «اذيت شدن» را بالا مي‌برد ، چهره را عوض نمي‌کند.
  if (evClap) {
    // ---------- جفت شدن ----------
    uint32_t gap = now - lastSingleClap;
    bool doubleClap = lastSingleClap && gap > DBL_MIN && gap < DBL_MAX;
    lastSingleClap = now;

    bool wasOpen = earsOpen();

    // ☠️ اين‌جا بزرگ‌ترين تغيير است ، و از يک بازبينيِ صادقانه
    //    آمد: **من نمي‌توانم دست زدن را از صداي در و فنجان
    //    جدا کنم.** سه آزمون نوشتم و هر سه در عمل فقط
    //    «صداي بلندِ تيز» را گرفتند.
    //
    //    پس به جاي جنگيدن با فيزيک ، طراحي را عوض کردم:
    //    **يک صداي تکي هيچ کاري نمي‌کند.** فقط به ياد
    //    سپرده مي‌شود تا شايد با بعدي جفت شود.
    //
    //    نتيجه‌اش اين است که خطاي تشخيص **نامرئي** مي‌شود.
    //    اگر بستنِ در را دست زدن بشنود ، هيچ اتفاقي نمي‌افتد
    //    و تو هرگز نمي‌فهمي اشتباه کرده. و دو بار بستنِ در
    //    در نيم‌ثانيه ، تقريبا هرگز پيش نمي‌آيد.
    //
    //    درس: وقتي نمي‌تواني ورودي را دقيق‌تر کني ، خروجي را
    //    محتاط‌تر کن.
    if (!doubleClap && !wasOpen) return;         // يک صداي تکي — بي‌خيال

    // از اين‌جا به بعد صدا **پذيرفته** است ، پس تعامل حساب
    // مي‌شود. (قبلا اين در senseStep بود و صداهاي رد شده هم
    // آن را صفر مي‌کردند.)
    lastTouchOrMove = now;
    Serial.println("[گوش] 👏 پذيرفته شد");

    if (doubleClap) {
      lastSingleClap = 0;                        // جفت مصرف شد
      earsUntil = now + EARS_MS;
      if (!wasOpen) Serial.printf("[گوش] 👂 دو دست (%lums) — باز شد\n",
                                  (unsigned long)gap);
      else          earsUntil = now + EARS_MS;
    } else {
      earsUntil = now + EARS_MS;                 // گوش باز بود — تمديد
    }

    // ---------- بيدار شدن با دو دست ----------
    // اين بخش قبلا نبود و کاربر گزارش داد: «دست زدم چند بار
    // اما بيدار نشد.» صدا بايد مثل لمس بيدارش کند.
    if (mood == M_ASLEEP || mood == M_SLEEPY) {
      setMood(mood == M_ASLEEP ? M_YAWN : M_SURPRISE,
              mood == M_ASLEEP ? T_YAWN : T_SURPRISE);
      // اگر دل‌بسته باشد ، بيدار که شد سلام هم مي‌کند
      if (soul.bond > 450) SHOW_SET(SET_GREET, 2400);
      blNow = BL_AWAKE;  lcdBacklight(BL_AWAKE);
      lastTouchOrMove = now;
      return;
    }

    if (mood == M_SURPRISE || mood == M_EXCITED) {
      setMood(M_EXCITED, T_EXCITED);
      SHOW_SET(SET_CHEER, T_EXCITED);
      return;
    }
    if (mood == M_IDLE || mood == M_HAPPY || mood == M_CURIOUS || mood == M_THINK) {
      if (doubleClap && !wasOpen) {
        setMood(M_LISTEN, T_LISTEN);
        //   چهره‌ي «گوش مي‌دهد» در پاکسازيِ ۳ مهر حذف شد. حالتِ
        //   M_LISTEN نقاشيِ اختصاصيِ خودش را دارد ، همان کافي است.
      } else {
        setMood(M_SURPRISE, T_SURPRISE);
        SHOW_SET(SET_SURPRISE, T_SURPRISE);
      }
      return;
    }
  }
  // بيدار شدن — دو جور ، بسته به اينکه چقدر عميق خواب بوده
  //   خوابِ عميق   → خميازه مي‌کشد و کش مي‌آيد
  //   فقط خواب‌آلود → جا مي‌خورد
  // 🛡 اين بلوک بعد از دست زدن آمد. قبلا بالاتر بود و
  //    وقتي «گوش دادن» را فعال مي‌کرد `return` مي‌زد ، پس
  //    اگر در همان فريم دست هم زده بودي ، دست زدن گم مي‌شد.
  //    ورودي‌هاي **عمدي** بايد بر رفتارِ خودجوش مقدم باشند.
  // ---------- کسي دارد حرف مي‌زند ----------
  // موچي کلمه نمي‌فهمد ، ولي مي‌فهمد که کسي دارد حرف مي‌زند.
  // گاهي گوش مي‌دهد ، گاهي کنجکاو مي‌شود — قرعه‌اي ، تا هر
  // بار يک‌جور نباشد.
  //
  // ☠️ فاصله‌ي ۲۵ تا ۶۰ ثانيه زيادي بلند بود — حتي وقتي کار
  //    مي‌کرد ، کاربر نمي‌ديدش. ۷ تا ۱۸ ثانيه محسوس است ولي
  //    هنوز آن‌قدر نيست که وسط مکالمه‌ي تلفني اذيت کند.
  {
    static uint32_t nextListenAt = 0;
    static bool     wasListening = false;

    // ☠️ شرطِ کليدي: **فقط وقتي گوشش باز است**.
    //    بدون اين ، موچي به صداي تلويزيون و مکالمه‌ي تلفني و
    //    هر چيزي واکنش نشان مي‌داد.
    if (earsOpen()) {
      wasListening = true;
      // فاصله را کوتاه‌تر کرديم (۳ تا ۷ ثانيه) چون حالا اين
      // يک گفت‌وگوي **عمدي** است ، نه مزاحمتِ پس‌زمينه.
      if (mood == M_IDLE && now >= nextListenAt && micTalkingFor(700)) {
        nextListenAt = now + random(3000, 7000);
        soulTalk();
        setMood(M_LISTEN, T_LISTEN);
        SHOW_SET(SET_LISTEN, T_LISTEN);
        return;
      }
      if (mood != M_IDLE && now >= nextListenAt)
        nextListenAt = now + random(3000, 7000);
    }
    else if (wasListening) {
      // لحظه‌ي بسته شدنِ گوش — يک نشانه‌ي کوچک
      wasListening = false;
      Serial.println("[گوش] 👂 بسته شد");
      if (mood == M_IDLE) {
        setMood(M_HAPPY, 1600);
        showFace(faceByName("سر تکان داد"), 1600);
      }
    }
  }

  // دست زدن در بيداري
  //   بار اول  → جا مي‌خورد
  //   بار دوم (تا وقتي هنوز جا خورده) → ذوق مي‌کند
  // يعني يک دست زدن مي‌ترساندش ، ولي دست زدنِ پشت سر هم
  // برايش تشويق است. همان الگوي «همان ورودي ، دو واکنش».
  if ((mood == M_ASLEEP || mood == M_SLEEPY) && idle < 300) {
    setMood(mood == M_ASLEEP ? M_YAWN : M_SURPRISE,
            mood == M_ASLEEP ? T_YAWN : T_SURPRISE);
    blNow = BL_AWAKE;
    lcdBacklight(BL_AWAKE);      // بيدار شدن ناگهاني است ، نه محو
    return;
  }

  // ---------- پايان يافتن حالت زمان‌دار ----------
  if (moodUntil && now >= moodUntil) {
    faceTX = faceTY = 0;
    // بعد از عاشقانه مستقيم به عادي برنمي‌گردد — خجالت مي‌کشد.
    // زنجيره‌ي احساسي طبيعي‌تر از پرش ناگهاني به حالت خنثي است.
    if (mood == M_LOVE)          setMood(M_SHY, T_SHY);
    else if (mood == M_EXCITED)  setMood(M_HAPPY, 2000);   // ذوق آرام مي‌گيرد
    // عصبانيت که خوابيد ، دلخوري مي‌ماند — و اگر خيلي
    // رنجيده باشد ، بغضش مي‌ترکد.
    else if (mood == M_ANGRY) {
      angryCooling = true;               // اين ناراحتي «فروکش» است
      if (soul.grudge > 700) setMood(M_CRY,  T_CRY);
      else                   setMood(M_HURT, 2200);
    }
    else if (mood == M_KISS)      setMood(M_SHY,   T_SHY);
    else if (mood == M_HEARTHAND) setMood(M_SHY,   T_SHY);
    else if (mood == M_FEED)      setMood(M_HAPPY, 2000);
    else if (mood == M_CRY)       setMood(M_HURT,  2400);
    // کينه: ناراحتي‌اش تمام شد ، ولي اگر از قبل دل‌خور باشد
    // ممکن است دوباره بالا بزند. هرچه رنجش بيشتر ، احتمال
    // بيشتر. اين همان چيزي است که حافظه را **ديدني** مي‌کند.
    // ☠️ اين حلقه‌ي بي‌پايان مي‌ساخت:
    //      ناراحت → عصباني → ناراحت → عصباني → ...
    //    چون عصبانيت که تمام مي‌شد ناراحتي مي‌آمد ، و
    //    ناراحتي که تمام مي‌شد دوباره قرعه‌ي کينه مي‌افتاد.
    //    در لاگ سه دور پشت سر هم ديده شد و شبيهِ خرابي بود.
    //
    //    حالا: ناراحتي‌اي که **بعد از** عصبانيت آمده ، فروکشِ
    //    آن است و حق ندارد دوباره شعله‌ور شود.
    else if (mood == M_HURT && !angryCooling && soulHoldsGrudge()) {
      Serial.printf("[دل] هنوز دل‌خور است (رنجش %d)\n", soul.grudge);
      setMood(M_ANGRY, T_ANGRY);
    }
    else                       { setMood(M_IDLE, 0); angryCooling = false; }
    forceBlinkSoon();
  }

  // ---------- نشانگر بي‌کاري ----------
  // هر ۵ ثانيه مي‌گويد چند ثانيه است کسي کاري نکرده.
  // اگر اين عدد مدام صفر مي‌شود ، يعني چيزي دارد بيخودي
  // «تعامل» گزارش مي‌کند. براي خاموش کردن ، ۰ بگذار.
  #define DEBUG_IDLE 0
#if DEBUG_IDLE
  {
    static uint32_t lastTick = 0;
    if (now - lastTick > 5000) {
      lastTick = now;
      Serial.printf("[بي‌کاري] %lu ثانيه   (خواب‌آلود در %lu ، خواب در %lu)\n",
                    (unsigned long)(idle / 1000),
                    (unsigned long)(T_SLEEPY_AFTER / 1000),
                    (unsigned long)((T_SLEEPY_AFTER + T_SLEEP_AFTER) / 1000));
    }
  }
#endif

  // ⚠️ زمان‌سنجِ خوشحاليِ خودبه‌خودي فقط وقتي معنا دارد که در
  //   حالت عادي باشيم. اگر موچي مدتي خواب يا مشغول بوده ،
  //   اين زمان عقب مي‌افتد و به محض برگشتن به عادي فورا شليک
  //   مي‌کند — در لاگ ديديم: «عادي» و يک ميلي‌ثانيه بعد
  //   «خوشحال». پس تا وقتي عادي نيستيم ، جلو مي‌بريمش.
  if (mood != M_IDLE && now >= nextHappyAt)
    nextHappyAt = now + random(soulHappyGap(HAPPY_GAP_MIN), soulHappyGap(HAPPY_GAP_MAX));

  // ---------- چرخه‌ي بي‌کاري ----------
  if (mood == M_IDLE) {
    uint32_t sleepy = soulSleepyAfter(T_SLEEPY_AFTER);
    if (idle > sleepy + T_SLEEP_AFTER) {
      setMood(M_ASLEEP, 0);
      SHOW_SET(SET_SLEEP, 0);                     // هر شب يک‌جور مي‌خوابد
    }
    else if (idle > sleepy)                 setMood(M_SLEEPY, 0);
    else if (now >= nextHappyAt) {
      // گاهي خودبه‌خود حالش عوض مي‌شود — هميشه خوشحال نه.
      // يک ربات که فقط يک کار خودبه‌خودي بلد است ، قابل
      // پيش‌بيني مي‌شود و قابلِ پيش‌بيني يعني بي‌روح.
      nextHappyAt = now + random(soulHappyGap(HAPPY_GAP_MIN), soulHappyGap(HAPPY_GAP_MAX));

      // ☠️ اين‌جا بود که چهره‌هاي کاتالوگ معني پيدا مي‌کنند.
      //    بدون اين ، فقط يک فهرست بودند.
      //
      //    دسته را **شخصيتش** انتخاب مي‌کند:
      //      دل‌خور  → چهره‌هاي بدخُلق
      //      دل‌بسته → چهره‌هاي شاد
      //      وسط    → کنجکاوي
      //    و گاهي هم بي‌دليل لباس مي‌پوشد.
      //    دسته‌هاي تازه هم قرعه‌ي کوچکي دارند: شيطنت فقط
      //    وقتي دل‌بسته است (به غريبه شيطنت نمي‌کند) ، و
      //    «حالِ بد» نادر است تا ارزشش را نگه دارد.
      //   ☠️ ايرادِ دومِ «فقط يک‌سري چهره مي‌آيد» اين‌جا بود:
      //      شرطِ آخر `else SHOW_SET(SET_CURIOUS)` بود ، ولي
      //      شرطِ قبلي‌اش `bond > 400`. موچيِ دل‌بسته هميشه از
      //      ۴۰۰ بالاتر است ، پس **کلِ دسته‌ي کنجکاوي هرگز اجرا
      //      نمي‌شد**. يک else که هيچ‌وقت نمي‌رسد ، مُرده است.
      //
      //      حالا قرعه بر حسبِ بازه است نه زنجيره‌ي شرط‌ها ، تا
      //      سهمِ هر دسته صريح و قابلِ خواندن باشد.
      setMood(M_HAPPY, 3000);
      int roll = random(100);
      if (soul.grudge > 450)             SHOW_SET(SET_GRUMPY,   3000);
      else if (roll < 10)                SHOW_SET(SET_DRESSUP,  3600);
      else if (roll < 16)                SHOW_SET(SET_UNWELL,   2800);
      else if (roll < 26)                SHOW_SET(SET_CURIOUS,  3000);
      else if (roll < 34)                SHOW_SET(SET_IDLE,     3200);
      else if (roll < 42)                SHOW_SET(SET_PLAY,     3000);
      else if (roll < 50)                SHOW_SET(SET_FOOD,     3000);
      else if (soul.bond > 500 && roll < 62)
                                         SHOW_SET(SET_MISCHIEF, 3000);
      else                               SHOW_SET(SET_HAPPY,    3000);
    }
  }
  if (mood == M_SLEEPY && idle > soulSleepyAfter(T_SLEEPY_AFTER) + T_SLEEP_AFTER) setMood(M_ASLEEP, 0);

  // ---------- نور پس‌زمينه ----------
  int blTarget = (mood == M_ASLEEP) ? BL_ASLEEP
               : (mood == M_SLEEPY ? BL_SLEEPY : BL_AWAKE);
  if (blNow != blTarget) {
    blNow += (blTarget > blNow) ? 6 : -6;
    if (abs(blNow - blTarget) < 7) blNow = blTarget;
    lcdBacklight(blNow);
  }

  // ---------- Z هاي خواب ----------
  if (mood == M_ASLEEP && now - lastZ > 1500) {
    lastZ = now;
    spawnFloater(CX + 96, CY - 40, 1);
  }
}

// =====================================================
void setup() {
  Serial.begin(115200);
  delay(3500);
  Serial.println("\n#####################################");
  Serial.println("#   MOOCHI  —  موتور احساسات        #");
  Serial.println("#####################################\n");

  randomSeed(esp_random());

  fb = (uint16_t *)heap_caps_malloc(LCD_W * LCD_H * 2, MALLOC_CAP_SPIRAM);
  if (!fb) { Serial.println("❌ بافر PSRAM ساخته نشد"); while (true) delay(1000); }

  // ☠️ پيش از هر چيز ، ببين باسِ اصلي اصلا سالم است يا نه.
  //
  //    اگر خطِ SDA يا SCL پايين گير کرده باشد ، هيچ چيپي جواب
  //    نمي‌دهد — از جمله TCA9554 که پايه‌ي ريستِ نمايشگر را
  //    نگه مي‌دارد. نتيجه: «TCA9554 جواب نداد» و توقفِ کامل.
  //
  //    آن پيام درست بود ولي **بي‌فايده** — نمي‌گفت چرا. و علتش
  //    تقريبا هميشه يک چيز است: ماژولي که روي کانکتورِ کناري
  //    برعکس وصل شده و زمينش را روي خطِ SDA گذاشته.
  //
  //    ده خط کد ، در برابرِ يک ساعت سردرگمي.
  {
    pinMode(I2C_SDA, INPUT_PULLUP);
    pinMode(I2C_SCL, INPUT_PULLUP);
    delayMicroseconds(200);
    bool sda = digitalRead(I2C_SDA), scl = digitalRead(I2C_SCL);
    if (!sda || !scl) {
      Serial.println("\n☠️  باسِ اصليِ I2C گير کرده!");
      Serial.printf("    SDA(GPIO%d)=%s   SCL(GPIO%d)=%s\n",
                    I2C_SDA, sda ? "بالا" : "پايين ⚠️",
                    I2C_SCL, scl ? "بالا" : "پايين ⚠️");
      Serial.println("    در حالتِ سالم هر دو بايد بالا باشند.");
      Serial.println("");
      Serial.println("    شايع‌ترين علت: ماژولي روي کانکتورِ کناريِ J6");
      Serial.println("    که برعکس وصل شده و زمينش روي خطِ SDA افتاده.");
      Serial.println("    ترتيبِ درستِ J6:  ۱=SCL  ۲=SDA  ۳=3V3  ۴=GND");
      Serial.println("");
      Serial.println("    ⇒ ماژول را بکش و ريست کن. برد سالم است.");
      Serial.println("");
    }
  }

  Wire.begin(I2C_SDA, I2C_SCL, 400000);
#if USE_SECOND_BUS
  // ☠️ هرگز با پين ۱۹ و ۲۰ روشنش نکن — آن‌ها USB هستند
  Wire1.begin(I2C2_SDA, I2C2_SCL, 400000);
#endif

  if (!lcdInit(40000000)) { Serial.println("❌ نمايشگر"); while (true) delay(1000); }
  fbFill(C_BG);
  fbPush();

  Serial.println("🔎 حواس:");
  i2cScanReport();

  if (touchInit()) Serial.printf("   ✅ لمس: CST816 @0x15 (شناسه‌ي چيپ 0x%02X)\n", touch.chipId);
  else             Serial.println("   ⚠️ لمس پيدا نشد — نوازش و سقلمه کار نمي‌کند");

  if (micInit()) {
    micStart();
    Serial.println("   ✅ گوش: ES7210 @0x40 → I2S");
    Serial.println("      MCLK=2 · BCK=48 · WS=38 · DIN=39   (نسخه ۲ برد)");
    //   بايد **بعد از** micInit باشد — کانالِ بلندگو آن‌جا ساخته مي‌شود
    if (!voiceBegin())
      Serial.println("   ⚠️ صدا راه نيفتاد — بلندگو وصل است؟");
  } else {
    Serial.println("   ⚠️ ميکروفن راه نيفتاد — دست زدن کار نمي‌کند");
  }

  if (!imuInit()) {
    // ☠️ اين پيام تا امروز مي‌گفت «USE_SECOND_BUS را ۱ کن» ، در
    //    حالي که خودش ۱ شده بود. پيامِ راهنمايي که از واقعيت
    //    عقب بماند ، بدتر از نبودنش است — آدم را دنبالِ نخودِ
    //    سياه مي‌فرستد. حالا يک فهرستِ عيب‌يابيِ واقعي است.
    Serial.println("   ⚠️ شتاب‌سنج پيدا نشد — تکان و ضربه کار نمي‌کند");
    Serial.println("      هر دو باس و هر چهار آدرس (0x68/0x69) چک شدند.");
    Serial.println("      اگر MPU6050 وصل است ، به ترتيب اين‌ها را ببين:");
    Serial.println("      ۱ تغذيه — VCC به 3V3 و GND به GND. با مولتي‌متر");
    Serial.println("        روي خودِ پايه‌ي ماژول اندازه بگير ، نه روي برد.");
    Serial.println("      ۲ جابه‌جاييِ SDA و SCL — شايع‌ترين اشتباه.");
    Serial.println("        SDA→GPIO13 · SCL→GPIO12");
    Serial.println("      ۳ لحيم يا جامپرِ شل — ماژول را تکان بده و");
    Serial.println("        برد را ريست کن ؛ اگر گاهي پيدا شد ، اتصال است.");
    Serial.println("      ۴ خودِ ماژول — روي باسِ اصلي (11/10) هم امتحانش کن.");
    Serial.println("      ☠️ هرگز GPIO19/20 — آن‌ها پين‌هاي USB هستند.");
  }

  soulLoad();

  Serial.println("\n▶ آماده. لمسش کن ، نوازشش کن ، تکانش بده.\n");

  blinkAt = millis() + 1500;
  gazeAt  = millis() + 900;
  nextHappyAt = millis() + random(soulHappyGap(HAPPY_GAP_MIN), soulHappyGap(HAPPY_GAP_MAX));

  // ---------- سلامِ آغازين ----------
  // اولين چيزي که مي‌بيني به اين بستگي دارد که ديروز با او
  // چه کرده‌اي. اين ارزان‌ترين راه براي ديدني کردنِ حافظه
  // است — لازم نيست منتظر بماني تا اثرش را حس کني.
  if (soul.boots > 1) {
    if (soul.grudge > 500)      { setMood(M_HURT,    2600);
                                  SHOW_SET(SET_GRUMPY, 2600); }
    else if (soul.bond > 500)   { setMood(M_EXCITED, 2600);
                                  SHOW_SET(SET_GREET, 2600); }
    else if (soul.bond > 200)   { setMood(M_HAPPY,   2200);
                                  SHOW_SET(SET_HAPPY, 2200); }
    else                        { setMood(M_CURIOUS, T_CURIOUS);
                                  SHOW_SET(SET_CURIOUS, T_CURIOUS); }
  }
  lastTouchOrMove = millis();
  setMood(M_IDLE, 0);

  for (int b = 0; b <= BL_AWAKE; b += 6) { lcdBacklight(b); delay(6); }
}

// =====================================================
//  حالتِ نمايش — رژه‌ي همه‌ي چهره‌ها
//
//  چرا لازم شد: خيلي از حالت‌ها راه‌اندازيِ سختي دارند
//  (عصباني شش سقلمه مي‌خواهد ، گريه رنجشِ بالا ، بوس
//  نگه داشتنِ انگشت). نتيجه اين بود که کاربر نصفِ کارِ
//  انجام‌شده را اصلا نديد.
//
//  ابزارِ ديدن به اندازه‌ي خودِ کار مهم است.
//
//  روشن کردن: در Serial Monitor حرف `p` را بفرست.
//  جلو/عقب : `n` و `b` ، خاموش: دوباره `p`
// =====================================================
// =====================================================
//   حالتِ کاتالوگ — چهره‌ي دلخواه از روي نسخه
//
//   وقتي catFace >= 0 است ، به جاي موتورِ حالت‌ها همان
//   نسخه کشيده مي‌شود. اين‌طور همه‌ي چهره‌ها روي همان موتورِ
//   موجود سوار مي‌شوند بي‌آنکه enum Mood شلوغ شود.
// =====================================================

// =====================================================
//   آزمايشِ زنده‌ي شتاب‌سنج   (کليد i)
//
//   ☠️ چرا حالتِ جدا و نه فقط يک چاپ در حلقه‌ي اصلي:
//      عددِ خام به تنهايي بي‌معني است. چيزي که لازم داري اين
//      است که **آستانه‌ها را کنارِ عدد ببيني** — وگرنه نمي‌داني
//      ۱٫۸ زياد است يا کم. پس نوار هم مي‌کشم.
bool     imuTest   = false;
uint32_t imuTestAt = 0;

void imuTestStep() {
  if (millis() - imuTestAt < 120) return;
  imuTestAt = millis();
  imuRead();

  static float prevM = 1.0f, energy = 0;
  float d = fabsf(imu.mag - prevM);
  prevM  = imu.mag;
  energy = energy * 0.86f + d * 14.0f;

  // نوارِ انرژي با نشانه‌ي آستانه
  char bar[33];
  int  fill = (int)(energy / SHAKE_TRIGGER * 20.0f);
  if (fill > 32) fill = 32;
  for (int i = 0; i < 32; i++) bar[i] = (i < fill) ? '#' : '.';
  bar[20] = (fill > 20) ? '#' : '|';        // جاي آستانه‌ي تکان
  bar[32] = 0;

  Serial.printf("[شتاب] x=%+5.2f y=%+5.2f z=%+5.2f  |v|=%4.2f  "
                "انرژي=%4.2f [%s] %s%s\n",
                imu.ax, imu.ay, imu.az, imu.mag, energy, bar,
                (energy > SHAKE_TRIGGER) ? "تکان! " : "",
                (imu.mag > IMPACT_G)     ? "ضربه!" : "");
}

bool     parade     = false;
int      paradeIdx  = 0;
uint32_t paradeAt   = 0;
const uint32_t PARADE_MS = 2600;

const Mood PARADE[] = {
  M_IDLE, M_HAPPY, M_EXCITED, M_LOVE, M_HEARTHAND, M_KISS,
  M_FEED, M_RAINBOW, M_CURIOUS, M_LISTEN, M_THINK, M_SHY,
  M_SURPRISE, M_EYEROLL, M_HURT, M_CRY, M_ANGRY,
  M_DIZZY, M_PET, M_SLEEPY, M_ASLEEP, M_YAWN
};
const int PARADE_N = sizeof(PARADE) / sizeof(PARADE[0]);

// رژه دو حالت دارد:
//   paradeCat = false  →  ۲۲ حالتِ رفتاري (انيميشن‌دار)
//   paradeCat = true   →  همه‌ي چهره‌هاي کاتالوگ (CATALOG_N)
bool paradeCat = true;

void paradeShow(int i) {
  paradeAt = millis();
  if (paradeCat) {
    paradeIdx = (i + CATALOG_N) % CATALOG_N;
    showFace(paradeIdx, 0);
  } else {
    paradeIdx = (i + PARADE_N) % PARADE_N;
    setMood(PARADE[paradeIdx], 0);
    Serial.printf("[رژه] %d/%d  %s\n", paradeIdx + 1, PARADE_N,
                  moodName(PARADE[paradeIdx]));
  }
}

void loop() {
  if (imuTest) {
    imuTestStep();
    soulTick();
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'i' || c == 'I') { imuTest = false; Serial.println("[شتاب] آزمايش خاموش"); }
    }
    return;
  }

  if (parade) {
    // ☠️ کاربر گزارش داد «ديگه به لمس و صدا واکنش نشون نمي‌ده»
    //    و حق داشت: رژه که روشن بود ، حواس خاموش مي‌ماند و
    //    راهِ روشني براي بيرون آمدن نداشت جز حرفِ `p` در
    //    مانيتور. حالا **هر لمسي** از رژه بيرونش مي‌آورد.
    if (millis() - paradeAt > PARADE_MS) paradeShow(paradeIdx + 1);
    touchRead();

    // 🛡 در رژه `senseStep` اجرا نمي‌شود ، پس micTakeClap هم
    //    صدا زده نمي‌شود و پرچمِ دست زدن روي true مي‌ماند.
    //    بدون اين ، لحظه‌اي که از رژه بيرون بيايي يک دست
    //    زدنِ کهنه شليک مي‌شود.
    micTakeClap();
    gestureLock = false;

    if (touch.pressed || touch.gesture != TG_NONE) {
      parade = false;
      Serial.println("[رژه] با لمس خاموش شد");
      setMood(M_IDLE, 0);
    }
  } else {
    senseStep();
    moodStep();
  }
  voiceStep();     // پر کردنِ بافرِ صدا — هيچ‌وقت منتظر نمي‌ماند
  soulTick();      // فراموشيِ آرام + ذخيره‌ي به‌موقع در فلش
  soulReport();

  // در Serial Monitor:  r = پاک کردن حافظه ، d = گزارشِ فوري
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r' || c == 'R') soulReset();
    // w = پين‌ياب — «سيمم را کجا زده‌ام؟»
    if (c == 'w' || c == 'W') pinFinder();
    // i = آزمايشِ زنده‌ي شتاب‌سنج
    if (c == 'i' || c == 'I') {
      if (imu.kind == IMU_NONE) {
        Serial.println("[شتاب] چيپي پيدا نشده — اول اسکنِ I2C را ببين");
      } else {
        imuTest = true;
        Serial.printf("[شتاب] آزمايش روشن — %s\n", imu.name);
        Serial.printf("        آستانه‌ي تکان=%.2f  آستانه‌ي ضربه=%.2f g\n",
                      SHAKE_TRIGGER, IMPACT_G);
        Serial.println("        بي‌حرکت روي ميز بايد |v| نزديکِ ۱٫۰۰ باشد.");
        Serial.println("        دوباره i بزن تا خاموش شود.");
      }
    }
    // v = صداي بعدي را بگو ، براي آزمايشِ بلندگو
    if (c == 'v' || c == 'V') {
      static int vi = 0;
      vi = (vi + 1) % VOICE_N;
      Serial.printf("[صدا] %d/%d  %s\n", vi + 1, VOICE_N, VOICE[vi].name);
      voiceSay(vi);
    }
    // + و − بلنديِ کدک
    if (c == '+') { static int vv = VOICE_VOL; vv = min(100, vv + 6);
                    es8311Volume(vv); Serial.printf("[صدا] بلندي %d\n", vv); }
    if (c == '-') { static int vv = VOICE_VOL; vv = max(0,  vv - 6);
                    es8311Volume(vv); Serial.printf("[صدا] بلندي %d\n", vv); }
    if (c == 'f' || c == 'F') soulForgive();
    if (c == 'p' || c == 'P') {
      parade = !parade;
      Serial.println(parade ? "[رژه] روشن — n بعدي · b قبلي · c نوع · p خاموش"
                            : "[رژه] خاموش");
      if (parade) paradeShow(0); else setMood(M_IDLE, 0);
    }
    if (c == 'c' || c == 'C') {
      paradeCat = !paradeCat;
      Serial.println(paradeCat ? "[رژه] چهره‌هاي کاتالوگ"
                               : "[رژه] ۲۲ حالتِ رفتاري");
      if (parade) paradeShow(0);
    }
    if (parade && (c == 'n' || c == 'N')) paradeShow(paradeIdx + 1);
    if (parade && (c == 'b' || c == 'B')) paradeShow(paradeIdx - 1);
    if (c == 'd' || c == 'D')
      Serial.printf("[دل] پيوند %d · رنجش %d · %s · %lu بار روشن شده\n",
                    soul.bond, soul.grudge, soulMoodName(),
                    (unsigned long)soul.boots);
  }

  bowPhase += 0.10f;
  if (bowPhase > 62.8f) bowPhase -= 62.8f;

  if (mood == M_IDLE || mood == M_HAPPY) gazeIdle();
  else if (mood != M_PET)                gazeTowards(0, 0);
  gazeStep();
  faceStep();

  float t = moodT();
  int   o = eyeOpen();

  // ابرو با نگاه حرکت مي‌کند — ولي کمتر از چشم.
  // اين‌جا مقدارِ پيش‌فرض را مي‌گذاريم ؛ حالت‌هايي که نگاهِ
  // خاصِ خودشان را دارند (کنجکاو ، فکر ، گوش‌دادن) خودشان
  // دوباره تنظيمش مي‌کنند.
  setBrowGaze((int)gazeX, (int)gazeY);

  // ---------- گذر نرم ----------
  // نيمه‌ي اول: چهره‌ي قبلي در حال بسته شدن
  // نيمه‌ي دوم: چهره‌ي جديد در حال باز شدن
  uint32_t td      = millis() - transStart;
  bool     trans   = td < TRANS_MS;
  Mood     show    = mood;
  float    shutter = 0.0f;

  if (trans) {
    float h = (float)td / TRANS_MS;
    if (h < 0.5f) {
      show    = prevMood;
      shutter = h * 2.0f;
      // زمانِ خودِ حالت قبلي — وگرنه انيميشنش از اول شروع مي‌شود
      t = (millis() - prevMoodStart) / 1000.0f;
    } else {
      show    = mood;
      shutter = (1.0f - h) * 2.0f;
    }
  }

  // چهره‌ي کاتالوگ بر همه‌چيز مقدم است
  if (catFace >= 0) {
    if (catUntil && millis() >= catUntil) { hideFace(); }
    else {
      drawRecipe(CATALOG[catFace], (int)faceX, (int)faceY,
                 (int)gazeX, (int)gazeY, o, t);
      if (CATALOG[catFace].flags & FR_BOW)
        drawBow(CX - EYE_DX - 34 + (int)faceX, CY - 66 + (int)faceY, bowPhase);
      drawShutter(shutter);
      fbPush();
      return;
    }
  }

  switch (show) {
    case M_IDLE:     drawIdle(o);      break;
    case M_HAPPY:    drawHappy(o);     break;
    case M_PET:      drawPet(t);       break;
    case M_LOVE:     drawLove(t);      break;
    case M_DIZZY:    drawDizzy(t);     break;
    case M_HURT:     drawHurt(t, o);   break;
    case M_SURPRISE: drawSurprise(t);  break;
    case M_SLEEPY:   drawSleepy(t);    break;
    case M_ASLEEP:   drawAsleep(t);    break;
    case M_CURIOUS:  drawCurious(t, o);break;
    case M_LISTEN:   drawListen(t, o); break;
    case M_ANGRY:    drawAngry(t, o);  break;
    case M_KISS:     drawKiss(t);      break;
    case M_CRY:      drawCry(t);       break;
    case M_EYEROLL:  drawEyeRoll(t);   break;
    case M_HEARTHAND:drawHeartHand(t); break;
    case M_FEED:     drawFeed(t);      break;
    case M_RAINBOW:  drawRainbow(t);   break;
    case M_THINK:    drawThink(t, o);  break;
    case M_SHY:      drawShy(t, o);    break;
    case M_EXCITED:  drawExcited(t);   break;
    case M_YAWN:     drawYawn(t);      break;
  }

  drawShutter(shutter);
  fbPush();
}
