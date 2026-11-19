#pragma once
// =====================================================
//   MOOCHI — حس حرکت + «یاد دادن» حرکت‌ها (۱۲ مهر ۱۴۰۵)
//
//   چرا عوض شد (تست ۱۲ مهر: «اصلا واکنش‌هاش درست نیست»):
//   ۱. قبلا شتاب‌سنج فقط یک بار در هر فریم خوانده می‌شد (هر ۲۵ تا ۹۰ ms).
//      ضربه‌ي تیز فقط چند میلی‌ثانیه طول می‌کشد و بیشترش دیده نمی‌شد ، و
//      «پرش» بین دو نمونه به سرعت فریم بستگی داشت (فریم کند = پرش بزرگ‌تر).
//      حالا یک «کار» جدا روی هسته‌ي ۰ ، هر ۵ ms (۲۰۰ بار در ثانیه) می‌خواند.
//   ۲. قبلا هر نمونه جدا با چند عدد ثابت مقایسه می‌شد. حالا هر حرکت از اول
//      تا آخرش یک «رویداد» است با ۵ ویژگی:
//        بیشترین چرخش (°/s) ، بیشترین شتاب اضافه (g) ، بیشترین تیزی (پرش شتاب
//        بین دو نمونه‌ي ۵ms) ، مدت (ms) ، تعداد برگشت جهت چرخش (رفت‌وبرگشت تکان).
//   ۳. عددها حدس من نیستند: در «یاد دادن» (فرمان t ، یا خودکار بار اول)
//      تو هر حرکت را ۳ بار انجام می‌دهی و ویژگی‌هایش ذخیره می‌شود. بعدا هر
//      رویداد به نزدیک‌ترین حرکتِ یادگرفته نسبت داده می‌شود.
//      تا وقتی یاد نداده‌ای ، عددهای پیش‌فرض (حدسی) به کار می‌روند.
// =====================================================
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <VL53L0X.h>        // ۱۲ مهر: فاصله‌سنج — کتابخانه‌ي Pololu (همان باس I2C ، آدرس 0x29)

enum MotionClass { MC_GENTLE, MC_SHAKE, MC_KNOCK, MC_MOVE, MC_N };
const char *MC_FA[MC_N] = {"آرام", "تکان", "ضربه", "حرکت تند"};
const char *MC_EN[MC_N] = {"gentle", "SHAKE", "KNOCK", "MOVE"};
const int NF = 5;

struct MotionEvent {
  float peakG, peakA, peakJ;   // °/s ، g ، g
  uint16_t dur;                // ms
  uint8_t rev;                 // برگشت جهت چرخش‌های تند
};

// ---------- ویژگی‌ها (لگاریتمی ، تا ضربه‌ي ۲ برابر قوی‌تر فاصله‌ي معقول داشته باشد) ----------
void motionFeatures(const MotionEvent &e, float *f) {
  f[0] = logf(1 + e.peakG / 50.0f);
  f[1] = logf(1 + e.peakA / 0.15f);
  f[2] = logf(1 + e.peakJ / 0.10f);
  f[3] = logf(1 + e.dur / 80.0f);
  f[4] = (e.rev > 8 ? 8 : e.rev) / 2.0f;
}
float motionStrength(const MotionEvent &e) {
  float f[NF]; motionFeatures(e, f);
  return f[0] + f[1] + f[2];
}

// ---------- مدل (در NVS ذخیره می‌شود) ----------
const uint32_t MOTION_MAGIC = 0x4D4F5431;   // "MOT1"
struct MotionModel {
  uint32_t magic;
  float mean[MC_N][NF];
  float sd[NF];
  float rockLo, rockHi;       // بازه‌ي چرخش میانگین در «تاب دادن»
  uint8_t rockRev;            // کمترین برگشت جهت در ۴ ثانیه
  uint8_t trained;
};
MotionModel mm;

void motionDefaults() {
  // ⚠️ حدسی — فقط تا وقتی که «یاد دادن» انجام شود
  MotionEvent d[MC_N] = {
    {  50, 0.15f, 0.12f,  500, 0 },   // آرام (برداشتن ، جابه‌جا کردن)
    { 400, 1.50f, 0.80f, 1200, 6 },   // تکان
    {  40, 0.80f, 1.00f,   60, 0 },   // ضربه
    { 120, 0.50f, 0.35f,  350, 1 },   // حرکت تند
  };
  mm.magic = MOTION_MAGIC;
  for (int c = 0; c < MC_N; c++) motionFeatures(d[c], mm.mean[c]);
  for (int i = 0; i < NF; i++) mm.sd[i] = 0.5f;
  mm.rockLo = 20; mm.rockHi = 110; mm.rockRev = 4;
  mm.trained = 0;
}

int motionClassify(const MotionEvent &e, float *dist) {
  float f[NF]; motionFeatures(e, f);
  int best = 0; float bd = 1e9f;
  for (int c = 0; c < MC_N; c++) {
    float s = 0;
    for (int i = 0; i < NF; i++) { float z = (f[i] - mm.mean[c][i]) / mm.sd[i]; s += z * z; }
    s = sqrtf(s);
    // ۱۲ مهر: «ناگهانی» یعنی کوتاه. در لاگ ، جابه‌جا کردنِ ۲٫۵ ثانیه‌ای «حرکت تند» شد
    // (چون مدت در «آرام»ها خیلی متفاوت بود و وزنش کم شد). رویداد بلند ضربه/حرکت تند نیست.
    if (c == MC_KNOCK && e.dur > 400)  s = 1e6f;
    if (c == MC_MOVE  && e.dur > 1500) s = 1e6f;
    if (dist) dist[c] = s > 1e5f ? 99 : s;
    if (s < bd) { bd = s; best = c; }
  }
  return best;
}

// ---------- پردازش نمونه‌ها (۲۰۰ بار در ثانیه) ----------
struct ImuSnap {
  float a[3];          // شتاب نرم‌شده (برای کج شدن و وارونه بودن)
  float g;             // چرخش لحظه‌ای °/s
  float rockE;         // میانگین نرم چرخش (حدود ۰٫۵ ثانیه)
  int   rockRevs;      // برگشت جهت چرخش در ۴ ثانیه‌ي اخیر (آستانه‌ي پایین)
  uint32_t lastActive; // آخرین زمانی که حرکت قابل توجهی بود
  uint16_t distMm;     // فاصله‌سنج: میلی‌متر ، ۰ = چیزی نیست / سنسور نیست
  uint32_t distAt;     // زمان آخرین اندازه
};

// ---------- فاصله‌سنج VL53L0X ----------
// چون شتاب‌سنج و فاصله‌سنج روی یک باس I2C هستند ، هر دو فقط در همین کارِ هسته‌ي ۰
// خوانده می‌شوند (دو هسته همزمان به Wire دست بزنند ، خراب می‌شود).
VL53L0X tof;
bool tofOk = false;
bool tofBegin() {
  Wire.beginTransmission(0x29);
  if (Wire.endTransmission() != 0) { Serial.println("فاصله‌سنج (0x29) پیدا نشد — بدون آن ادامه می‌دهیم."); return false; }
  tof.setBus(&Wire);
  tof.setTimeout(200);
  if (!tof.init()) { Serial.println("⚠️ فاصله‌سنج جواب داد ولی راه نیفتاد."); return false; }
  tof.setMeasurementTimingBudget(33000);   // ۳۳ میلی‌ثانیه برای هر اندازه
  tof.startContinuous();
  Serial.println("فاصله‌سنج VL53L0X پیدا شد (0x29).");
  tofOk = true;
  return true;
}

static portMUX_TYPE imuMux = portMUX_INITIALIZER_UNLOCKED;
static ImuSnap imuSnapShared;
static QueueHandle_t motionQ = nullptr;
static volatile bool imuPause = false, imuPaused = false;

// آستانه‌های شروع رویداد: پایین‌تر از این = لرزش دست / نویز
const float EV_G = 35, EV_A = 0.10f, EV_J = 0.12f;
const uint32_t EV_QUIET_MS = 150, EV_MAX_MS = 2500;

struct ImuProc {
  float aLP[3] = {0, 0, 1}, aPrev[3] = {0, 0, 1};
  bool  first = true;
  float rockE = 0;
  float lowDir[3] = {0, 0, 0}, hiDir[3] = {0, 0, 0};
  uint32_t revT[16] = {0}; int revI = 0;
  bool inEv = false; uint32_t evStart = 0, evLastAct = 0;
  MotionEvent ev;
  uint32_t lastActive = 0;
};
static ImuProc ip;

// یک نمونه را پردازش کن (جدا از خواندن ، تا روی کامپیوتر هم قابل تست باشد)
void imuTick(const float *a, const float *gv, uint32_t now) {
  float amag = sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
  float g = sqrtf(gv[0]*gv[0] + gv[1]*gv[1] + gv[2]*gv[2]);
  if (ip.first) { for (int i = 0; i < 3; i++) ip.aLP[i] = ip.aPrev[i] = a[i]; ip.first = false; }
  float dj[3] = {a[0]-ip.aPrev[0], a[1]-ip.aPrev[1], a[2]-ip.aPrev[2]};
  float jerk = sqrtf(dj[0]*dj[0] + dj[1]*dj[1] + dj[2]*dj[2]);
  for (int i = 0; i < 3; i++) { ip.aPrev[i] = a[i]; ip.aLP[i] += (a[i] - ip.aLP[i]) * 0.12f; }
  ip.rockE += (g - ip.rockE) * 0.01f;

  // برگشت جهت چرخش (آستانه‌ي پایین ، برای تاب دادن)
  if (g > 15) {
    float d = gv[0]*ip.lowDir[0] + gv[1]*ip.lowDir[1] + gv[2]*ip.lowDir[2];
    if (d < 0) { ip.revT[ip.revI] = now; ip.revI = (ip.revI + 1) % 16; }
    for (int i = 0; i < 3; i++) ip.lowDir[i] = gv[i];
  }

  bool act = g > EV_G || fabsf(amag - 1) > EV_A || jerk > EV_J;
  if (act) ip.lastActive = now;
  if (act && !ip.inEv) {
    ip.inEv = true; ip.evStart = now; ip.evLastAct = now;
    ip.ev = {0, 0, 0, 0, 0};
    for (int i = 0; i < 3; i++) ip.hiDir[i] = 0;
  }
  if (ip.inEv) {
    if (act) ip.evLastAct = now;
    if (g > ip.ev.peakG) ip.ev.peakG = g;
    if (fabsf(amag - 1) > ip.ev.peakA) ip.ev.peakA = fabsf(amag - 1);
    if (jerk > ip.ev.peakJ) ip.ev.peakJ = jerk;
    if (g > 80) {   // برگشت جهتِ چرخش تند = رفت‌وبرگشت تکان
      float d = gv[0]*ip.hiDir[0] + gv[1]*ip.hiDir[1] + gv[2]*ip.hiDir[2];
      if (d < 0 && ip.ev.rev < 255) ip.ev.rev++;
      for (int i = 0; i < 3; i++) ip.hiDir[i] = gv[i];
    }
    if (now - ip.evLastAct >= EV_QUIET_MS || now - ip.evStart >= EV_MAX_MS) {
      uint32_t d = ip.evLastAct - ip.evStart + 5;
      ip.ev.dur = d > 65000 ? 65000 : d;
      if (motionQ) xQueueSend(motionQ, &ip.ev, 0);
      ip.inEv = false;
    }
  }

  int nr = 0;
  for (int i = 0; i < 16; i++) if (ip.revT[i] && now - ip.revT[i] < 4000) nr++;
  portENTER_CRITICAL(&imuMux);
  for (int i = 0; i < 3; i++) imuSnapShared.a[i] = ip.aLP[i];
  imuSnapShared.g = g;
  imuSnapShared.rockE = ip.rockE;
  imuSnapShared.rockRevs = nr;
  imuSnapShared.lastActive = ip.lastActive;
  portEXIT_CRITICAL(&imuMux);
}

void getImuSnap(ImuSnap &s) {
  portENTER_CRITICAL(&imuMux);
  s = imuSnapShared;
  portEXIT_CRITICAL(&imuMux);
}

// ---------- کارِ خواندن (هسته‌ي ۰) ----------
bool readImu6(float *a, float *gv) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14) != 14) return false;
  uint8_t b[14];
  for (int i = 0; i < 14; i++) b[i] = Wire.read();
  auto s16 = [&](int i) { return (int16_t)((b[i] << 8) | b[i + 1]); };
  a[0] = s16(0) / 8192.0f; a[1] = s16(2) / 8192.0f; a[2] = s16(4) / 8192.0f;
  gv[0] = s16(8) / 65.5f; gv[1] = s16(10) / 65.5f; gv[2] = s16(12) / 65.5f;
  return true;
}

void imuTask(void *) {
  TickType_t last = xTaskGetTickCount();
  int tick = 0;
  for (;;) {
    if (imuPause) { imuPaused = true; vTaskDelay(pdMS_TO_TICKS(5)); last = xTaskGetTickCount(); continue; }
    imuPaused = false;
    float a[3], gv[3];
    if (readImu6(a, gv)) imuTick(a, gv, millis());
    // فاصله‌سنج: هر ۲۰ms یک نگاه (بدون صبر کردن) — اگر اندازه‌ي تازه آماده بود بخوان
    if (tofOk && (++tick % 4) == 0) {
      if (tof.readReg(VL53L0X::RESULT_INTERRUPT_STATUS) & 0x07) {
        uint16_t r = tof.readReg16Bit(VL53L0X::RESULT_RANGE_STATUS + 10);
        tof.writeReg(VL53L0X::SYSTEM_INTERRUPT_CLEAR, 0x01);
        if (r >= 8000 || r < 20) r = 0;                    // ۸۱۹۰ یعنی «چیزی دیده نشد»
        portENTER_CRITICAL(&imuMux);
        imuSnapShared.distMm = r; imuSnapShared.distAt = millis();
        portEXIT_CRITICAL(&imuMux);
      }
    }
    vTaskDelayUntil(&last, pdMS_TO_TICKS(5));
  }
}

// تا وقتی کار متوقف است می‌شود از Wire مستقیم استفاده کرد (کالیبره‌ي جهت)
void imuHold(bool on) {
  imuPause = on;
  if (on) { uint32_t t = millis(); while (!imuPaused && millis() - t < 100) delay(2); }
}

bool loadMotionModel() {
  Preferences pr;
  if (!pr.begin("moochi", true)) return false;
  MotionModel t;
  size_t n = pr.getBytes("motion", &t, sizeof(t));
  pr.end();
  if (n == sizeof(t) && t.magic == MOTION_MAGIC) { mm = t; return true; }
  return false;
}
void saveMotionModel() {
  Preferences pr;
  pr.begin("moochi", false);
  pr.putBytes("motion", &mm, sizeof(mm));
  pr.end();
}

void motionBegin() {
  motionDefaults();
  if (loadMotionModel()) Serial.println("حرکت‌های یادگرفته خوانده شد.");
  else Serial.println("هنوز حرکتی یاد داده نشده (عددهای پیش‌فرض).");
  motionQ = xQueueCreate(12, sizeof(MotionEvent));
  xTaskCreatePinnedToCore(imuTask, "imu", 4096, nullptr, 2, nullptr, 0);
}

void printEvent(const char *tag, const MotionEvent &e, const float *dist) {
  Serial.printf("[%s] چرخش=%.0f شتاب=%.2f تیزی=%.2f مدت=%ums برگشت=%u", tag,
                e.peakG, e.peakA, e.peakJ, (unsigned)e.dur, (unsigned)e.rev);
  if (dist) {
    Serial.print("   فاصله:");
    for (int c = 0; c < MC_N; c++) Serial.printf(" %s=%.1f", MC_FA[c], dist[c]);
  }
  Serial.println();
}

// =====================================================
//   یاد دادن حرکت‌ها
// =====================================================
void bigText(const char *t, uint16_t col) {
  tft.fillRect(0, 100, 240, 90, ST77XX_BLACK);
  tft.setTextColor(col);
  tft.setTextSize(4);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(t, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, 130);
  tft.print(t);
}
void drainMotion() { MotionEvent e; while (xQueueReceive(motionQ, &e, 0) == pdTRUE) {} }
// صبر کن تا ms میلی‌ثانیه حرکتی نباشد
void waitQuiet(uint32_t ms) {
  uint32_t t0 = millis();
  while (true) {
    ImuSnap s; getImuSnap(s);
    uint32_t now = millis();
    if (now - s.lastActive > ms && now - t0 > ms) break;
    if (now - t0 > 15000) break;
    delay(20);
  }
  drainMotion();
}

// یک نمونه از یک حرکت بگیر: رویدادی که از «آرام» قوی‌تر باشد.
// بعد از اولین رویداد قوی ، تا ۶۰۰ms قوی‌ترینش نگه داشته می‌شود (برداشتن قبل از تکان حساب نشود).
bool captureOne(float minStrength, MotionEvent &out, uint16_t col) {
  uint32_t t0 = millis(), firstAt = 0;
  float bestS = -1;
  while (millis() - t0 < 20000) {
    MotionEvent e;
    if (xQueueReceive(motionQ, &e, pdMS_TO_TICKS(20)) == pdTRUE) {
      float s = motionStrength(e);
      printEvent(s >= minStrength ? "نمونه" : "رد شد (ضعیف)", e, nullptr);
      if (s >= minStrength && s > bestS) { bestS = s; out = e; if (!firstAt) firstAt = millis(); }
    }
    if (firstAt && millis() - firstAt > 600) return true;
    progressBar((millis() - t0) / 20000.0f, col);
  }
  return bestS >= 0;
}

void runMotionTeach() {
  uint16_t cY = tft.color565(255, 210, 60), cG = tft.color565(80, 230, 120),
           cP = tft.color565(255, 60, 200);
  Serial.println();
  Serial.println("===== یاد دادن حرکت‌ها =====");
  drainMotion();

  static MotionEvent samp[MC_N][8];
  int ns[MC_N] = {0, 0, 0, 0};

  // ---- ۱) جابه‌جا کردن آرام: این‌ها نباید واکنشی داشته باشند ----
  Serial.println("۱) ۱۰ ثانیه ربات را عادی بردار ، جابه‌جا کن ، دستت بگیر (بدون تکان و ضربه)");
  promptScreen("CARRY ME", "gently, 10 sec", 3, cY);
  uint32_t t0 = millis();
  float gentleMaxS = 0;
  while (millis() - t0 < 10000) {
    MotionEvent e;
    if (xQueueReceive(motionQ, &e, pdMS_TO_TICKS(20)) == pdTRUE) {
      printEvent("آرام", e, nullptr);
      if (ns[MC_GENTLE] < 8) samp[MC_GENTLE][ns[MC_GENTLE]++] = e;
      float s = motionStrength(e); if (s > gentleMaxS) gentleMaxS = s;
    }
    progressBar((millis() - t0) / 10000.0f, cY);
  }
  float minS = gentleMaxS + 0.3f;
  if (minS < 2.4f) minS = 2.4f;
  Serial.printf("   رویداد آرام: %d ، کمترین قدرت قابل قبول برای بقیه: %.2f\n", ns[MC_GENTLE], minS);

  // ---- ۲) سه حرکت ، هر کدام ۳ بار ----
  const MotionClass order[3] = {MC_SHAKE, MC_KNOCK, MC_MOVE};
  const char *title[3] = {"SHAKE ME", "TAP ME", "MOVE FAST"};
  const char *sub[3]   = {"hard, like dizzy", "one sharp knock", "quick jerk/lift"};
  const char *fa[3]    = {"محکم تکانش بده (مثل وقتی می‌خواهی گیج شود)",
                          "یک ضربه‌ي تیز با انگشت روی بدنه بزن",
                          "یک‌دفعه و سریع جابه‌جایش کن (بالا بکش یا هل بده) — بدون تکان دادن"};
  for (int k = 0; k < 3; k++) {
    MotionClass c = order[k];
    for (int r = 0; r < 3; r++) {
      waitQuiet(700);
      char s2[24]; snprintf(s2, sizeof(s2), "%s  %d/3", sub[k], r + 1);
      Serial.printf("%d) %s — بار %d از ۳\n", k + 2, fa[k], r + 1);
      promptScreen(title[k], s2, 3, cY);
      MotionEvent e;
      if (captureOne(minS, e, cY)) {
        samp[c][ns[c]++] = e;
        bigText("GOT IT", cG);
      } else {
        Serial.println("   ⚠️ چیزی گرفته نشد (۲۰ ثانیه گذشت) — رد شد");
        bigText("SKIP", cP);
      }
      delay(500);
    }
  }

  // ---- ۳) تاب دادن آرام (لالایی) ----
  waitQuiet(700);
  Serial.println("۵) ۶ ثانیه آرام تابش بده ، مثل بچه که می‌خواهی بخوابد");
  promptScreen("ROCK ME", "slowly, 6 sec", 3, cY);
  t0 = millis();
  float sumE = 0; int nE = 0, maxRev = 0;
  while (millis() - t0 < 6000) {
    ImuSnap s; getImuSnap(s);
    if (millis() - t0 > 1500) { sumE += s.rockE; nE++; if (s.rockRevs > maxRev) maxRev = s.rockRevs; }
    progressBar((millis() - t0) / 6000.0f, cY);
    delay(20);
  }
  drainMotion();
  float avgE = nE ? sumE / nE : 0;
  Serial.printf("   تاب: چرخش میانگین %.0f °/s ، برگشت جهت در ۴ ثانیه تا %d\n", avgE, maxRev);

  // ---- ساختن مدل ----
  motionDefaults();
  int cnt = 0;
  float sq[NF] = {0}, fsum[MC_N][NF] = {{0}};
  for (int c = 0; c < MC_N; c++)
    for (int j = 0; j < ns[c]; j++) { float f[NF]; motionFeatures(samp[c][j], f); for (int i = 0; i < NF; i++) fsum[c][i] += f[i]; }
  for (int c = 0; c < MC_N; c++)
    if (ns[c]) for (int i = 0; i < NF; i++) mm.mean[c][i] = fsum[c][i] / ns[c];
  // پراکندگی داخل هر گروه (همه‌ي گروه‌ها با هم) ؛ کمترین ۰٫۳ تا یک نمونه‌ي خاص زیادی سخت‌گیر نشود
  for (int c = 0; c < MC_N; c++)
    for (int j = 0; j < ns[c]; j++) {
      float f[NF]; motionFeatures(samp[c][j], f);
      for (int i = 0; i < NF; i++) { float d = f[i] - mm.mean[c][i]; sq[i] += d * d; }
      cnt++;
    }
  for (int i = 0; i < NF; i++) {
    float sd = cnt > MC_N ? sqrtf(sq[i] / (cnt - MC_N)) : 0.5f;
    mm.sd[i] = sd < 0.3f ? 0.3f : sd;
  }
  if (avgE > 5 && maxRev >= 2) {
    mm.rockLo = avgE * 0.5f; mm.rockHi = avgE * 2.0f;
    int rr = maxRev / 2; mm.rockRev = rr < 2 ? 2 : rr;
  } else {
    Serial.println("   ⚠️ تاب درست ثبت نشد — عدد پیش‌فرض ماند");
  }
  mm.trained = 1;
  saveMotionModel();

  Serial.println("   میانگین‌ها (لگاریتمی: چرخش ، شتاب ، تیزی ، مدت ، برگشت):");
  for (int c = 0; c < MC_N; c++)
    Serial.printf("   %-8s n=%d  %.2f %.2f %.2f %.2f %.2f\n", MC_FA[c], ns[c],
                  mm.mean[c][0], mm.mean[c][1], mm.mean[c][2], mm.mean[c][3], mm.mean[c][4]);
  Serial.printf("   پراکندگی: %.2f %.2f %.2f %.2f %.2f\n", mm.sd[0], mm.sd[1], mm.sd[2], mm.sd[3], mm.sd[4]);
  // چک: هر نمونه با مدل خودش درست تشخیص داده می‌شود؟
  int ok = 0, tot = 0;
  for (int c = 0; c < MC_N; c++) for (int j = 0; j < ns[c]; j++) { tot++; if (motionClassify(samp[c][j], nullptr) == c) ok++; }
  Serial.printf("   نمونه‌های درست تشخیص‌داده: %d از %d\n", ok, tot);
  Serial.println("   ذخیره شد ✅");

  // ---- ۴) امتحان: هر حرکت را روی صفحه اسمش را می‌نویسد ----
  Serial.println("۶) امتحان (۲۰ ثانیه): هر حرکتی بکن ، اسمش روی صفحه می‌آید");
  promptScreen("TRY ME", "20 sec test", 3, cG);
  t0 = millis();
  while (millis() - t0 < 20000) {
    MotionEvent e;
    if (xQueueReceive(motionQ, &e, pdMS_TO_TICKS(20)) == pdTRUE) {
      float dist[MC_N];
      int c = motionClassify(e, dist);
      printEvent(MC_FA[c], e, dist);
      bigText(MC_EN[c], c == MC_GENTLE ? cY : cP);
    }
    progressBar((millis() - t0) / 20000.0f, cG);
  }
  promptScreen("DONE", "saved", 3, cG);
  delay(800);
  Serial.println("===== تمام =====");
}
