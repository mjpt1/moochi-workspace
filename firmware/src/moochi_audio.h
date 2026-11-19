#pragma once
// =====================================================
//   MOOCHI — صدا از آمپلی‌فایر MAX98357 (۱۲ مهر ۱۴۰۵)
//
//   «واژه‌نامه»ي ۲۱ آوا همان moochi_voice.h نسخه‌ي firmware-1.85 است
//   (صدای ساخته‌شده ، بدون فایل). فقط خروجی عوض شد:
//     قبلا: کدک ES8311 برد Waveshare
//     حالا: MAX98357 با I2S استاندارد (کتابخانه‌ي ESP_I2S خود هسته‌ي Arduino-ESP32)
//
//   سیم‌کشی (۱۱ آبان ۱۴۰۵ — بعد از بازبینی):
//     BCLK → G27   LRC → G25   DIN → G23  (قبلا G12 بود؛ G12 پایه‌ي بوت است و ممکن بود برد بالا نیاید)
//     VIN  → ۵ ولت ریل برق   GND → زمین مشترک   GAIN و SD → وصل نشوند
//   G27 و G25 قبلا CS و RST نمایشگر بودند. نمایشگر گرد بدون آن دو کار می‌کند
//   (متن پشت ماژول + کتابخانه‌ي Adafruit_GC9A01A: CS=-1 و RST=-1 مجاز است).
//
//
//   صدا در یک «کار» جدا روی هسته‌ي ۰ ساخته و فرستاده می‌شود تا چهره کند نشود.
// =====================================================
#include <ESP_I2S.h>

const int PIN_I2S_BCLK = 27, PIN_I2S_LRC = 25, PIN_I2S_DIN = 23;
#define VOICE_RATE  16000
#define VOICE_CHUNK 128

struct Syll { uint16_t f0, f1; uint16_t ms; uint8_t shape; uint8_t vib; };
struct Utter { const char *name; const Syll *s; uint8_t n; uint8_t gain; };

static const Syll SY_CHIRP[]    = {{ 760,1180, 70,1, 6},{1180, 980, 60,1, 4}};
static const Syll SY_HAPPY[]    = {{ 640, 980, 80,1, 5},{ 980,1320, 70,1, 7},{1320,1180,110,1,10}};
static const Syll SY_CURIOUS[]  = {{ 520, 620,120,0, 3},{ 700,1240,180,1, 8}};
static const Syll SY_QUESTION[] = {{ 700, 700, 90,1, 0},{ 620,1360,200,1, 9}};
static const Syll SY_SAD[]      = {{ 720, 520,180,0, 6},{ 500, 330,260,0, 9}};
static const Syll SY_CRY[]      = {{ 880, 640,140,0,14},{ 820, 560,160,0,16},{ 700, 420,240,0,18}};
static const Syll SY_GIGGLE[]   = {{1080,1240, 55,1, 4},{ 900,1060, 50,1, 4},{1120,1300, 55,1, 4},{ 940,1100, 60,1, 5}};
static const Syll SY_EXCITED[]  = {{ 900,1420, 60,1, 4},{1100,1620, 55,1, 5},{1300,1860, 50,1, 6},{1700,1500, 90,1,10}};
static const Syll SY_LOVE[]     = {{ 620, 900,130,0, 4},{ 900,1100,120,0, 6},{1040, 960,220,0,12}};
static const Syll SY_ANGRY[]    = {{ 240, 200,150,2,16},{ 210, 165,190,2,18}};
static const Syll SY_GRUMPY[]   = {{ 330, 290,150,2, 8},{ 300, 250,180,2,10}};
static const Syll SY_SURPRISE[] = {{1500,1500, 45,1, 0},{ 900,1700,120,1, 6}};
static const Syll SY_YAWN[]     = {{ 420, 640,300,0, 5},{ 640, 360,420,0, 8}};
static const Syll SY_SLEEP[]    = {{ 400, 300,260,0, 4},{ 300, 240,340,0, 5}};
static const Syll SY_EAT[]      = {{ 560, 720, 60,2, 0},{ 480, 620, 60,2, 0},{ 620, 900,110,1, 6}};
static const Syll SY_OK[]       = {{ 880, 880, 60,1, 0},{1240,1240, 90,1, 0}};
static const Syll SY_NO[]       = {{ 640, 640, 70,2, 0},{ 440, 440,110,2, 0}};
static const Syll SY_LISTEN[]   = {{ 980, 980, 50,0, 0},{ 760, 760, 50,0, 0}};
static const Syll SY_BOOT[]     = {{ 480, 720,110,1, 0},{ 720,1080,110,1, 0},{1080,1440,140,1, 6}};
static const Syll SY_HURT[]     = {{1200, 700, 90,1,12},{ 640, 480,200,0,14}};
static const Syll SY_DIZZY[]    = {{ 700,1000,120,1,20},{1000, 640,120,1,20},{ 640, 940,120,1,20},{ 940, 560,180,1,20}};

enum VoiceId {
  V_NONE = -1,
  V_CHIRP, V_HAPPY, V_CURIOUS, V_QUESTION, V_SAD, V_CRY, V_GIGGLE,
  V_EXCITED, V_LOVE, V_ANGRY, V_GRUMPY, V_SURPRISE, V_YAWN, V_SLEEP,
  V_EAT, V_OK, V_NO, V_LISTEN, V_BOOT, V_HURT, V_DIZZY
};
#define U(n, a, g) { n, a, (uint8_t)(sizeof(a) / sizeof(a[0])), g }
static const Utter VOICE[] = {
  U("جیک", SY_CHIRP, 70), U("شاد", SY_HAPPY, 80), U("کنجکاو", SY_CURIOUS, 70), U("سوال", SY_QUESTION, 72),
  U("غمگین", SY_SAD, 66), U("گریه", SY_CRY, 72), U("خنده", SY_GIGGLE, 76), U("ذوق", SY_EXCITED, 86),
  U("عشق", SY_LOVE, 74), U("عصبانی", SY_ANGRY, 92), U("غرغر", SY_GRUMPY, 70), U("جا خورد", SY_SURPRISE, 84),
  U("خمیازه", SY_YAWN, 58), U("خواب", SY_SLEEP, 48), U("خوردن", SY_EAT, 72), U("بله", SY_OK, 72),
  U("نه", SY_NO, 72), U("گوش", SY_LISTEN, 56), U("روشن شد", SY_BOOT, 78), U("آخ", SY_HURT, 84),
  U("گیج", SY_DIZZY, 74),
};
#undef U
static const int VOICE_N = sizeof(VOICE) / sizeof(VOICE[0]);

// بلندی کل (۰..۱). بیشتر از ۰٫۵ ممکن است برق USB را بیندازد و برد ری‌استارت شود.
const float VOL_LEVELS[4] = {0.0f, 0.15f, 0.30f, 0.50f};
volatile int volIdx = 2;
volatile int voiceReq = V_NONE;           // درخواست از حلقه‌ي اصلی (کار صدا برمی‌دارد)
bool audioOk = false;

static I2SClass i2sOut;

inline void voiceSay(int id) { if (id >= 0 && id < VOICE_N) voiceReq = id; }

inline float voiceOsc(float ph, uint8_t shape) {
  float s = sinf(ph);
  switch (shape) {
    case 1: return 0.78f * s + 0.22f * sinf(ph * 2.0f);
    case 2: return tanhf(s * 2.6f) * 0.8f;              // مربعی نرم
    default: return s;
  }
}

void audioTask(void *) {
  static int16_t buf[VOICE_CHUNK * 2];                  // استریو (L و R یکی)
  int cur = V_NONE; uint8_t syl = 0;
  uint32_t sylPos = 0, sylLen = 0;
  float phase = 0, vibPh = 0, pitch = 1, speed = 1; uint8_t gain = 70;
  for (;;) {
    int req = voiceReq;
    if (req != V_NONE) {
      voiceReq = V_NONE;
      cur = req; syl = 0; sylPos = 0; phase = 0; vibPh = 0; gain = VOICE[cur].gain;
      pitch = 0.92f + random(160) / 1000.0f;                 // ±۸٪ کوک ، هر بار کمی فرق
      speed = 0.94f + random(120) / 1000.0f;
      sylLen = (uint32_t)(VOICE[cur].s[0].ms * VOICE_RATE / 1000.0f / speed);
    }
    float vol = VOL_LEVELS[volIdx & 3];
    for (int i = 0; i < VOICE_CHUNK; i++) {
      int16_t v = 0;
      if (cur != V_NONE) {
        const Utter &u = VOICE[cur];
        const Syll &sy = u.s[syl];
        float t = sylLen > 1 ? (float)sylPos / (float)(sylLen - 1) : 1.0f;
        float f = (sy.f0 + (sy.f1 - sy.f0) * t) * pitch;
        if (sy.vib) { vibPh += 2.0f * PI * 5.5f / VOICE_RATE; f *= 1.0f + (sy.vib / 1000.0f) * sinf(vibPh); }
        float env = 1.0f;                                    // حمله و افت نرم (بدون «تق»)
        const float ATK = 0.06f, REL = 0.28f;
        if (t < ATK) env = t / ATK; else if (t > 1 - REL) env = (1.0f - t) / REL;
        phase += 2.0f * PI * f / VOICE_RATE;
        if (phase > 2.0f * PI) phase -= 2.0f * PI;
        float a = voiceOsc(phase, sy.shape) * env * (gain / 100.0f) * vol;
        v = (int16_t)(a * 32000.0f);
        if (++sylPos >= sylLen) {
          sylPos = 0; phase = 0;
          if (++syl >= u.n) cur = V_NONE;
          else { sylLen = (uint32_t)(u.s[syl].ms * VOICE_RATE / 1000.0f / speed); if (sylLen < 2) sylLen = 2; }
        }
      }
      buf[i * 2] = v; buf[i * 2 + 1] = v;
    }
    i2sOut.write((const uint8_t *)buf, sizeof(buf));   // منتظر می‌ماند تا جا باشد (روی هسته‌ي ۰)
  }
}

bool audioBegin() {
  i2sOut.setPins(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DIN);
  if (!i2sOut.begin(I2S_MODE_STD, VOICE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("⚠️ صدا: I2S راه نیفتاد");
    return false;
  }
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 1, nullptr, 0);
  audioOk = true;
  Serial.printf("صدا: I2S روشن (BCLK=G%d LRC=G%d DIN=G%d) ، بلندی %d از ۳\n",
                PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DIN, (int)volIdx);
  return true;
}
