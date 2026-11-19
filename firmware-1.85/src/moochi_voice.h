#pragma once
// =====================================================
//   MOOCHI  —  صدا
//
//   ☠️ تصميمِ اول ، و مهم‌ترينش: موچي **حرف نمي‌زند**.
//
//      وسوسه‌ي اول اين است که فايلِ صوتي ضبط‌شده بگذاري. سه
//      دليل که نگذاشتم:
//
//        ۱ فضا — يک ثانيه صداي ۱۶ کيلوهرتزِ ۱۶ بيتي ۳۲ کيلوبايت
//          است. سي تا صدا يعني يک مگابايت ، و جدولِ پارتيشن
//          هنوز تنظيم نشده.
//        ۲ زبان — هر کلمه‌اي که بگويد موچي را به يک زبان
//          مي‌چسباند. جيک‌جيک زبان ندارد.
//        ۳ زنده بودن — فايلِ ضبط‌شده هر بار **عينا** يکي است.
//          گوش خيلي زود تکرار را مي‌گيرد و صدا مصنوعي مي‌شود.
//
//      پس صدا **ساخته** مي‌شود ، نه پخش. هر بار کمي فرق دارد ،
//      درست مثل چهره‌ها که هر بار از دسته قرعه مي‌خورند.
//
//   ☠️ تصميمِ دوم: هيچ delay اي. مثلِ بقيه‌ي پروژه.
//      حلقه‌ي اصلي دارد ۳۰ بار در ثانيه صورت مي‌کشد. اگر
//      اين‌جا منتظرِ DMA بمانيم ، انيميشن لِه مي‌شود.
// =====================================================

#include <Arduino.h>
#include <driver/i2s_std.h>
#include "es8311.h"
#include "moochi_mic.h"          // spkChan · SPK_DOUT · PA_CTRL · MIC_RATE

// ---------- تنظيم ----------
#define VOICE_RATE     MIC_RATE      // همان نرخِ ميکروفن — پورت مشترک است
#define VOICE_CHUNK    128           // نمونه در هر بار نوشتن
#define VOICE_VOL      62            // بلنديِ کدک ۰..۱۰۰
#define VOICE_PA_OFF   400           // ms سکوت تا خاموش کردنِ آمپ

// ☠️ آمپ را بعد از سکوت خاموش مي‌کنيم ، نه اينکه هميشه روشن
//    بماند. NS4150B وقتي روشن است و داده‌اي نمي‌آيد ، يک
//    «هيس»ِ ملايم مي‌دهد. روي يک رباتِ روميزي که ساکت نشسته ،
//    اين هيس خيلي بيشتر از خودِ صدا به چشم — به گوش — مي‌آيد.

// =====================================================
//   ساختِ يک آوا
//
//   هر آوا زنجيره‌اي از «هجا» است. هر هجا:
//     f0→f1  سُر خوردنِ فرکانس (هرتز)
//     ms     طول
//     shape  شکلِ موج ۰ سينوس · ۱ سينوس+هارمونيک · ۲ مربعيِ نرم
//     vib    عمقِ ويبراتو (۰ يعني بدونِ لرزش)
// =====================================================
struct Syll {
  uint16_t f0, f1;
  uint16_t ms;
  uint8_t  shape;
  uint8_t  vib;
};

struct Utter {
  const char  *name;
  const Syll  *s;
  uint8_t      n;
  uint8_t      gain;        // ۰..۱۰۰ — بلنديِ نسبي
};

// ---------- واژه‌نامه‌ي موچي ----------
//
//   الگوهاي عمومي که در هر زبانِ «بامزه» تکرار مي‌شوند:
//     بالا رفتنِ فرکانس  → پرسش ، کنجکاوي ، ذوق
//     پايين آمدن        → غم ، خستگي ، نه
//     پرش‌هاي کوتاهِ پشتِ هم → خنده
//     فرکانسِ پايينِ لرزان → غرغر ، عصبانيت

static const Syll SY_CHIRP[]    = {{ 760,1180, 70,1, 6},{1180, 980, 60,1, 4}};
static const Syll SY_HAPPY[]    = {{ 640, 980, 80,1, 5},{ 980,1320, 70,1, 7},
                                   {1320,1180,110,1,10}};
static const Syll SY_CURIOUS[]  = {{ 520, 620,120,0, 3},{ 700,1240,180,1, 8}};
static const Syll SY_QUESTION[] = {{ 700, 700, 90,1, 0},{ 620,1360,200,1, 9}};
static const Syll SY_SAD[]      = {{ 720, 520,180,0, 6},{ 500, 330,260,0, 9}};
static const Syll SY_CRY[]      = {{ 880, 640,140,0,14},{ 820, 560,160,0,16},
                                   { 700, 420,240,0,18}};
static const Syll SY_GIGGLE[]   = {{1080,1240, 55,1, 4},{ 900,1060, 50,1, 4},
                                   {1120,1300, 55,1, 4},{ 940,1100, 60,1, 5}};
static const Syll SY_EXCITED[]  = {{ 900,1420, 60,1, 4},{1100,1620, 55,1, 5},
                                   {1300,1860, 50,1, 6},{1700,1500, 90,1,10}};
static const Syll SY_LOVE[]     = {{ 620, 900,130,0, 4},{ 900,1100,120,0, 6},
                                   {1040, 960,220,0,12}};
static const Syll SY_ANGRY[]    = {{ 240, 200,150,2,16},{ 210, 165,190,2,18}};
static const Syll SY_GRUMPY[]   = {{ 330, 290,150,2, 8},{ 300, 250,180,2,10}};
static const Syll SY_SURPRISE[] = {{1500,1500, 45,1, 0},{ 900,1700,120,1, 6}};
static const Syll SY_YAWN[]     = {{ 420, 640,300,0, 5},{ 640, 360,420,0, 8}};
static const Syll SY_SLEEP[]    = {{ 400, 300,260,0, 4},{ 300, 240,340,0, 5}};
static const Syll SY_EAT[]      = {{ 560, 720, 60,2, 0},{ 480, 620, 60,2, 0},
                                   { 620, 900,110,1, 6}};
static const Syll SY_OK[]       = {{ 880, 880, 60,1, 0},{1240,1240, 90,1, 0}};
static const Syll SY_NO[]       = {{ 640, 640, 70,2, 0},{ 440, 440,110,2, 0}};
static const Syll SY_LISTEN[]   = {{ 980, 980, 50,0, 0},{ 760, 760, 50,0, 0}};
static const Syll SY_BOOT[]     = {{ 480, 720,110,1, 0},{ 720,1080,110,1, 0},
                                   {1080,1440,140,1, 6}};
static const Syll SY_HURT[]     = {{1200, 700, 90,1,12},{ 640, 480,200,0,14}};
static const Syll SY_DIZZY[]    = {{ 700,1000,120,1,20},{1000, 640,120,1,20},
                                   { 640, 940,120,1,20},{ 940, 560,180,1,20}};

enum VoiceId {
  V_NONE = -1,
  V_CHIRP, V_HAPPY, V_CURIOUS, V_QUESTION, V_SAD, V_CRY, V_GIGGLE,
  V_EXCITED, V_LOVE, V_ANGRY, V_GRUMPY, V_SURPRISE, V_YAWN, V_SLEEP,
  V_EAT, V_OK, V_NO, V_LISTEN, V_BOOT, V_HURT, V_DIZZY
};

#define U(n, a, g) { n, a, (uint8_t)(sizeof(a)/sizeof(a[0])), g }
static const Utter VOICE[] = {
  U("جيک",      SY_CHIRP,    70),
  U("شاد",      SY_HAPPY,    80),
  U("کنجکاو",   SY_CURIOUS,  70),
  U("سوال",     SY_QUESTION, 72),
  U("غمگين",    SY_SAD,      66),
  U("گريه",     SY_CRY,      72),
  U("خنده",     SY_GIGGLE,   76),
  U("ذوق",      SY_EXCITED,  86),
  U("عشق",      SY_LOVE,     74),
  U("عصباني",   SY_ANGRY,    92),
  U("غرغر",     SY_GRUMPY,   70),
  U("جا خورد",  SY_SURPRISE, 84),
  U("خميازه",   SY_YAWN,     58),
  U("خواب",     SY_SLEEP,    48),
  U("خوردن",    SY_EAT,      72),
  U("بله",      SY_OK,       72),
  U("نه",       SY_NO,       72),
  U("گوش",      SY_LISTEN,   56),
  U("روشن شد",  SY_BOOT,     78),
  U("آخ",       SY_HURT,     84),
  U("گيج",      SY_DIZZY,    74),
};
#undef U
static const int VOICE_N = sizeof(VOICE) / sizeof(VOICE[0]);

// =====================================================
//   وضعيتِ پخش
// =====================================================
struct VoiceState {
  bool     ready   = false;
  bool     on      = false;      // آمپ روشن است؟
  int      cur     = V_NONE;
  uint8_t  syl     = 0;          // هجاي جاري
  uint32_t sylPos  = 0;          // نمونه‌هاي رفته از اين هجا
  uint32_t sylLen  = 0;          // طولِ هجا به نمونه
  float    phase   = 0;          // فازِ نوسان‌ساز
  float    vibPh   = 0;
  float    pitch   = 1.0f;       // تغييرِ کوکِ اين اجرا
  float    speed   = 1.0f;
  uint8_t  gain    = 70;
  uint32_t quietAt = 0;          // آخرين لحظه‌اي که صدا داشتيم
};
VoiceState voice;

// بافرِ باقي‌مانده — وقتي DMA پر است ، نمونه‌ها دور ريخته نشوند
static uint8_t pend[VOICE_CHUNK * 4];
static size_t  pendBytes = 0;

// ---------- آمپ ----------
inline void voicePA(bool on) {
  if (voice.on == on) return;
  voice.on = on;
  digitalWrite(PA_CTRL, on ? HIGH : LOW);
}

// =====================================================
//   راه‌اندازي
//   ⚠️ بايد **بعد از** micInit صدا زده شود ، چون کانالِ
//      بلندگو آن‌جا ساخته مي‌شود.
// =====================================================
inline bool voiceBegin() {
  pinMode(PA_CTRL, OUTPUT);
  digitalWrite(PA_CTRL, LOW);

  if (!spkChan) {
    Serial.println("   ❌ صدا: کانالِ I2S نيست (micInit اول اجرا شد؟)");
    return false;
  }
  if (!es8311Init(VOICE_RATE)) return false;
  es8311Volume(VOICE_VOL);
  es8311Mute(false);

  voice.ready = true;
  Serial.printf("   ✅ صدا: ES8311 @0x18 · %d هرتز · آمپ روي GPIO%d\n",
                VOICE_RATE, PA_CTRL);
  return true;
}

// =====================================================
//   گفتن
//
//   pitchVar و speedVar تنوع مي‌دهند. همان فلسفه‌ي چهره‌ها:
//   هيچ اجرايي عينِ اجراي قبلي نباشد.
// =====================================================
inline void voiceSay(int id, float pitchVar = 1.0f, float speedVar = 1.0f) {
  if (!voice.ready || id < 0 || id >= VOICE_N) return;
  voice.cur    = id;
  voice.syl    = 0;
  voice.sylPos = 0;
  voice.phase  = 0;
  voice.vibPh  = 0;
  voice.gain   = VOICE[id].gain;
  // ±۸٪ کوک و ±۶٪ سرعت ، تصادفي
  voice.pitch  = pitchVar * (0.92f + random(160) / 1000.0f);
  voice.speed  = speedVar * (0.94f + random(120) / 1000.0f);
  pendBytes    = 0;                  // آوايِ قبلي قطع شد ، ته‌مانده‌اش نماند
  voice.sylLen = (uint32_t)(VOICE[id].s[0].ms * VOICE_RATE
                            / 1000.0f / voice.speed);
  voicePA(true);
}

inline bool voiceBusy() { return voice.cur != V_NONE; }
inline void voiceStop() { voice.cur = V_NONE; }

// ---------- نوسان‌ساز ----------
//
//   ☠️ موجِ مربعيِ خالص روي بلندگوي کوچک **زننده** است — پُر از
//      هارمونيکِ بالاست و مثلِ زنگِ ساعت مي‌شود. پس «مربعيِ نرم»
//      مي‌سازم: سينوس را با tanh کمي اشباع مي‌کنم. همان جانِ
//      مربعي را دارد بدونِ تيزي‌اش.
inline float voiceOsc(float ph, uint8_t shape) {
  float s = sinf(ph);
  switch (shape) {
    case 1: return 0.78f * s + 0.22f * sinf(ph * 2.0f);   // + هارمونيکِ دوم
    case 2: return tanhf(s * 2.6f) * 0.8f;                // مربعيِ نرم
    default: return s;
  }
}

// =====================================================
//   گامِ صدا — از حلقه‌ي اصلي صدا زده مي‌شود
//
//   با مهلتِ صفر مي‌نويسد. اگر DMA پر باشد ، برمي‌گردد و
//   دفعه‌ي بعد امتحان مي‌کند. هيچ‌وقت منتظر نمي‌ماند.
// =====================================================
inline void voiceStep() {
  if (!voice.ready) return;
  uint32_t now = millis();

  if (voice.cur == V_NONE) {
    if (voice.on && now - voice.quietAt > VOICE_PA_OFF) voicePA(false);
    return;
  }

  const Utter &u = VOICE[voice.cur];
  static int16_t buf[VOICE_CHUNK * 2];        // استريو

  // اول باقي‌مانده‌ي دفعه‌ي قبل
  if (pendBytes) {
    size_t w = 0;
    i2s_channel_write(spkChan, pend, pendBytes, &w, 0);
    if (w < pendBytes) { memmove(pend, pend + w, pendBytes - w);
                         pendBytes -= w;  voice.quietAt = now;  return; }
    pendBytes = 0;
  }

  for (int i = 0; i < VOICE_CHUNK; i++) {
    if (voice.cur == V_NONE) { buf[i*2] = buf[i*2+1] = 0; continue; }

    const Syll &sy = u.s[voice.syl];
    float t = (voice.sylLen > 1)
              ? (float)voice.sylPos / (float)(voice.sylLen - 1) : 1.0f;

    // فرکانس — سُر خوردنِ خطي ، به علاوه‌ي ويبراتو
    float f = (sy.f0 + (sy.f1 - sy.f0) * t) * voice.pitch;
    if (sy.vib) {
      voice.vibPh += 2.0f * PI * 5.5f / VOICE_RATE;
      f *= 1.0f + (sy.vib / 1000.0f) * sinf(voice.vibPh);
    }

    // پوشِ دامنه — حمله‌ي کوتاه ، افتِ نرم
    //   ☠️ بدونِ پوش ، سر و تهِ هر هجا يک «کليک» مي‌دهد. پرشِ
    //      ناگهاني از صفر به دامنه يعني يک ضربه‌ي پله‌اي ، و
    //      بلندگو آن را به صورتِ تَق پخش مي‌کند.
    float env = 1.0f;
    const float ATK = 0.06f, REL = 0.28f;
    if (t < ATK)          env = t / ATK;
    else if (t > 1 - REL) env = (1.0f - t) / REL;

    voice.phase += 2.0f * PI * f / VOICE_RATE;
    if (voice.phase > 2.0f * PI) voice.phase -= 2.0f * PI;

    float a = voiceOsc(voice.phase, sy.shape) * env
              * (voice.gain / 100.0f) * 0.55f;
    int16_t v = (int16_t)(a * 32000.0f);
    buf[i * 2] = v;  buf[i * 2 + 1] = v;

    // پيشروي
    if (++voice.sylPos >= voice.sylLen) {
      voice.sylPos = 0;
      voice.phase  = 0;                       // هجاي تازه ، فازِ تازه
      if (++voice.syl >= u.n) {
        voice.cur = V_NONE;
        voice.quietAt = now;
      } else {
        voice.sylLen = (uint32_t)(u.s[voice.syl].ms * VOICE_RATE
                                  / 1000.0f / voice.speed);
        if (voice.sylLen < 2) voice.sylLen = 2;
      }
    }
  }

  //   ☠️ باگي که پيش از فلش گرفتمش:
  //      با مهلتِ صفر ، اگر DMA پر باشد i2s_channel_write هيچ
  //      يا بخشي را مي‌نويسد. نسخه‌ي اول نتيجه را **دور
  //      مي‌ريخت** ولي وضعيتِ نوسان‌ساز را جلو برده بود — يعني
  //      آن نمونه‌ها براي هميشه گم مي‌شدند و صدا بريده‌بريده
  //      مي‌شد. و چون خطايي برنمي‌گرداند ، بي‌صدا خراب مي‌شد.
  //
  //      حالا باقي‌مانده نگه داشته مي‌شود و دفعه‌ي بعد اول
  //      **همان** فرستاده مي‌شود ، بعد نمونه‌ي تازه ساخته.
  size_t wrote = 0;
  i2s_channel_write(spkChan, buf, sizeof(buf), &wrote, 0);
  if (wrote < sizeof(buf)) {
    pendBytes = sizeof(buf) - wrote;
    memmove(pend, (uint8_t *)buf + wrote, pendBytes);
  }
  voice.quietAt = now;
}
