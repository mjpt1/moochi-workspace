#pragma once
// =====================================================
//   MOOCHI  —  حافظه‌ي شخصيت
//
//   تا حالا موچي هر بار که روشن مي‌شد ، يک موجودِ تازه بود.
//   ديروز نوازشش کرده باشي يا اذيتش ، فرقي نمي‌کرد.
//
//   اين فايل آن را عوض مي‌کند. دو عدد نگه مي‌داريم:
//
//     پيوند (bond)   چقدر با تو خوب است
//     رنجش (grudge)  چقدر از تو دل‌خور است
//
//   و اين دو عدد **رفتارش را عوض مي‌کنند** ، نه اينکه فقط
//   جايي ذخيره شوند. حافظه‌اي که کاري نکند حافظه نيست.
//
//   ⚠️ فرقِ مهم با RTC memory:
//     `RTC_DATA_ATTR` فقط از خوابِ عميق و ريستِ نرم جان
//     سالم به در مي‌برد — با قطعِ برق پاک مي‌شود. موچي
//     قرار است از برق کشيده شود و فردا روشن شود ، پس
//     بايد در **فلش** بنويسيم. کتابخانه‌ي Preferences
//     (همان NVS) اين کار را مي‌کند.
// =====================================================

#include <Arduino.h>
#include <Preferences.h>

// ---------- حدهاي صفات ----------
const int16_t SOUL_MAX = 1000;

// ---------- چقدر هر رفتار اثر مي‌گذارد ----------
const int16_t GAIN_PET_SEC   =  6;   // هر ثانيه نوازش
const int16_t GAIN_LOVE      = 45;   // عاشق شدن
const int16_t GAIN_CLAP      =  3;   // دست زدن — تعامل مثبتِ کوچک
const int16_t GAIN_TALK      =  2;   // حرف زدن با او

const int16_t GAIN_POKE      =  3;   // يک سقلمه
const int16_t GAIN_ANNOY     = 22;   // سقلمه‌ي پياپي تا اذيت شدن
const int16_t GAIN_ANGRY     = 30;   // عصباني شدن
const int16_t GAIN_IMPACT    = 55;   // ضربه

// ---------- فراموشي ----------
//
// رنجش بايد **زودتر از پيوند** فراموش شود. اگر برعکس باشد
// موچي با گذر زمان بدخُلق‌تر مي‌شود ، که نه واقعي است نه
// دوست‌داشتني.
//
// هر دو در دو جا آب مي‌روند:
//   ۱ در طولِ روشن بودن ، آرام (SOUL_DECAY_MS)
//   ۲ هر بار که روشن مي‌شود ، يک پرشِ بخشش
// ☠️ توازنِ اول غلط بود و لاگ نشانش داد:
//      رنجش در دو دقيقه‌ي آزمايش به ۹۹۳ رسيد و **آن‌جا ماند**.
//      چون بالا رفتنش ۳۵ تا ۶۰ واحد در هر رويداد بود ولي
//      فراموشي‌اش ۴ واحد در دقيقه — يعني چهار ساعت طول
//      مي‌کشيد تا پاک شود.
//
//      درس: نرخِ پر شدن و نرخِ خالي شدن بايد با هم سنجيده
//      شوند ، نه جدا. من هر کدام را جداگانه «منطقي» انتخاب
//      کرده بودم و ترکيبشان بي‌معنا بود.
//
//      حالا رنجش در حدود ده دقيقه از اوج پاک مي‌شود.
const uint32_t SOUL_DECAY_MS   = 20000;  // هر بيست ثانيه
const int16_t  DECAY_BOND      =  1;     // پيوند: کُند
const int16_t  DECAY_GRUDGE    = 32;     // رنجش: خيلي سريع‌تر

const int16_t  BOOT_FORGIVE    = 250;    // خواب که برود ، زياد مي‌بخشد
const int16_t  BOOT_MISS       = 12;     // و اين‌قدر پيوند (دوري اثر دارد)

// ---------- هر چند وقت در فلش بنويسيم ----------
//
// ⚠️ فلش عمر محدود دارد (حدود ۱۰۰ هزار بار نوشتن در هر
//    خانه). اگر هر ثانيه بنويسيم ، در يک ماه مي‌سوزد.
//
// پس دو شرط مي‌گذاريم و **هر دو** بايد برقرار باشند:
//    ۱ دست‌کم ۹۰ ثانيه از نوشتنِ قبلي گذشته باشد
//    ۲ عددها دست‌کم ۱۵ واحد عوض شده باشند
//
// يعني وقتي موچي بي‌کار روي ميز است ، اصلا نمي‌نويسد.
const uint32_t SAVE_EVERY_MS   = 90000;
const int16_t  SAVE_MIN_CHANGE = 15;

struct Soul {
  int16_t  bond    = 0;
  int16_t  grudge  = 0;
  uint32_t boots   = 0;
  uint32_t petSec  = 0;     // مجموعِ ثانيه‌هاي نوازش ، براي آمار
  uint32_t pokes   = 0;
};
Soul soul;

static Preferences soulNvs;
static int16_t  soulSavedBond = 0, soulSavedGrudge = 0;
static uint32_t soulLastSave = 0, soulLastDecay = 0;
static bool     soulReady = false;

inline int16_t soulClamp(int32_t v) {
  if (v < 0) return 0;
  if (v > SOUL_MAX) return SOUL_MAX;
  return (int16_t)v;
}

// =====================================================
//  نامِ خُلق — براي لاگ و براي فهميدنِ اينکه چه شده
// =====================================================
inline const char *soulMoodName() {
  if (soul.grudge > 600)                    return "دل‌خور و زودرنج";
  if (soul.grudge > 300 && soul.bond < 300) return "بي‌اعتماد";
  if (soul.bond   > 700)                    return "دل‌بسته";
  if (soul.bond   > 350)                    return "رفيق";
  if (soul.boots  < 3)                      return "تازه‌وارد";
  return "معمولي";
}

// =====================================================
//  خواندن در آغاز
// =====================================================
inline void soulLoad() {
  if (!soulNvs.begin("moochi", false)) {
    Serial.println("   ⚠️ حافظه باز نشد — موچي هر بار از نو شروع مي‌کند");
    return;
  }
  soulReady = true;

  soul.bond   = soulNvs.getShort("bond",   0);
  soul.grudge = soulNvs.getShort("grudge", 0);
  soul.boots  = soulNvs.getULong("boots",  0);
  soul.petSec = soulNvs.getULong("petSec", 0);
  soul.pokes  = soulNvs.getULong("pokes",  0);

  int16_t wasG = soul.grudge;

  // ---------- بخششِ آغازِ راه ----------
  // «شب گذشت و صبح که شد ، کمتر دل‌خور بود.»
  // ما ساعتِ واقعي نداريم که بدانيم چند ساعت خاموش بوده ،
  // پس هر روشن شدن را يک واحدِ زمان حساب مي‌کنيم.
  //
  // 📌 بعدا: برد يک ساعتِ PCF85063 روي آدرس 0x51 دارد. اگر
  //    ثابت شود که با قطع برق هم ساعت را نگه مي‌دارد ،
  //    مي‌شود فراموشي را بر حسبِ ساعتِ واقعي حساب کرد.
  //    هنوز آزمايشش نکرده‌ايم ، پس ادعايش را نمي‌کنم.
  soul.grudge = soulClamp(soul.grudge - BOOT_FORGIVE);
  soul.bond   = soulClamp(soul.bond   - BOOT_MISS);

  soul.boots++;
  soulSavedBond = soul.bond;  soulSavedGrudge = soul.grudge;
  soulLastSave  = millis();
  soulLastDecay = millis();

  // ذخيره‌ي فوري ، چون شمارنده‌ي روشن شدن عوض شد
  soulNvs.putULong("boots", soul.boots);
  soulNvs.putShort("bond",   soul.bond);
  soulNvs.putShort("grudge", soul.grudge);

  Serial.printf("   🧠 حافظه: %s   (پيوند %d · رنجش %d)\n",
                soulMoodName(), soul.bond, soul.grudge);
  Serial.printf("      بار %lu ام است که روشن مي‌شود · %lu ثانيه نوازش · %lu سقلمه\n",
                (unsigned long)soul.boots, (unsigned long)soul.petSec,
                (unsigned long)soul.pokes);
  if (wasG != soul.grudge)
    Serial.printf("      (کمي بخشيد: رنجش %d → %d)\n", wasG, soul.grudge);
}

// =====================================================
//  نوشتن — فقط وقتي واقعا لازم است
// =====================================================
inline void soulSave(bool force = false) {
  if (!soulReady) return;

  int16_t d = abs(soul.bond - soulSavedBond) + abs(soul.grudge - soulSavedGrudge);
  if (!force) {
    if (d < SAVE_MIN_CHANGE) return;
    if (millis() - soulLastSave < SAVE_EVERY_MS) return;
  }
  if (d == 0 && !force) return;

  soulNvs.putShort("bond",   soul.bond);
  soulNvs.putShort("grudge", soul.grudge);
  soulNvs.putULong("petSec", soul.petSec);
  soulNvs.putULong("pokes",  soul.pokes);

  soulSavedBond   = soul.bond;
  soulSavedGrudge = soul.grudge;
  soulLastSave    = millis();
  Serial.printf("   🧠 حافظه ذخيره شد: پيوند %d · رنجش %d  (%s)\n",
                soul.bond, soul.grudge, soulMoodName());
}

// =====================================================
//  رويدادها
// =====================================================
// ☠️ نسخه‌ي اول **هيچ‌وقت کار نکرد** و هيچ خطايي هم نداد:
//
//        uint32_t sec = ms / 1000;
//        if (!sec) return;
//
//    و صدا زده مي‌شد با soulPet(900). تقسيمِ صحيح:
//        900 / 1000 = 0   →  همان خطِ اول برمي‌گردد
//
//    يعني نوازش **هرگز** پيوند نساخت. در لاگِ بار يازدهم
//    ديده شد: «۰ ثانيه نوازش» با پيوند ۸۱۸ — که همه‌اش از
//    دست زدن و عاشق شدن آمده بود.
//
//    درس: وقتي واحدِ ورودي ميلي‌ثانيه است ، داخل تابع هم
//    ميلي‌ثانيه بمان. تبديلِ زودهنگام به ثانيه ، خرده‌ها را
//    دور مي‌ريزد — و اگر هر بار کمتر از يک ثانيه بفرستي ،
//    همه‌چيز را دور مي‌ريزد.
inline void soulPet(uint32_t ms) {
  if (!ms) return;
  static uint32_t petAcc = 0;          // خرده‌ي ميلي‌ثانيه‌ها
  petAcc += ms;

  // امتياز را بر حسب ميلي‌ثانيه حساب کن ، نه ثانيه‌ي گِرد
  soul.bond   = soulClamp(soul.bond   + (int32_t)(ms * GAIN_PET_SEC / 1000));
  soul.grudge = soulClamp(soul.grudge - (int32_t)(ms * GAIN_PET_SEC / 2000));

  // شمارنده‌ي آمار فقط وقتي بالا مي‌رود که يک ثانيه جمع شود
  while (petAcc >= 1000) { petAcc -= 1000; soul.petSec++; }
}
inline void soulLove()  { soul.bond   = soulClamp(soul.bond + GAIN_LOVE);
                          soul.grudge = soulClamp(soul.grudge - GAIN_LOVE / 2); }
inline void soulClap()  { soul.bond   = soulClamp(soul.bond + GAIN_CLAP); }
inline void soulTalk()  { soul.bond   = soulClamp(soul.bond + GAIN_TALK); }

inline void soulPoke()  { soul.pokes++;
                          soul.grudge = soulClamp(soul.grudge + GAIN_POKE); }
inline void soulAnnoy() { soul.grudge = soulClamp(soul.grudge + GAIN_ANNOY); }
inline void soulAngry() { soul.grudge = soulClamp(soul.grudge + GAIN_ANGRY);
                          soul.bond   = soulClamp(soul.bond - GAIN_ANGRY / 3); }
inline void soulImpact(){ soul.grudge = soulClamp(soul.grudge + GAIN_IMPACT);
                          soul.bond   = soulClamp(soul.bond - GAIN_IMPACT / 3); }

// =====================================================
//  تيک — فراموشيِ آرام و ذخيره‌ي به‌موقع
//  هر فريم صدا زده مي‌شود ، ولي خودش مي‌فهمد کِي کار کند.
// =====================================================
inline void soulTick() {
  uint32_t now = millis();
  if (now - soulLastDecay >= SOUL_DECAY_MS) {
    soulLastDecay = now;
    soul.bond   = soulClamp(soul.bond   - DECAY_BOND);
    soul.grudge = soulClamp(soul.grudge - DECAY_GRUDGE);
  }
  soulSave(false);
}

// =====================================================
//  اثرِ شخصيت روي رفتار
//
//  اين مهم‌ترين بخش است. حافظه‌اي که فقط عدد نگه دارد و
//  کاري نکند ، حافظه نيست — دفترچه است.
// =====================================================

// نسبتِ ۰ تا ۱
inline float soulBondF()   { return soul.bond   / (float)SOUL_MAX; }
inline float soulGrudgeF() { return soul.grudge / (float)SOUL_MAX; }

// چند ثانيه نوازشِ پيوسته تا عاشق شدن؟
//   غريبه : ۶ ثانيه          دل‌بسته : ۳ ثانيه
inline uint32_t soulPetToLove(uint32_t base) {
  return base - (uint32_t)(base * 0.5f * soulBondF());
}

// چند سقلمه تا اذيت شدن؟
//   آرام : ۳ تا              زودرنج : ۲ تا
inline int soulPokesToAnnoy(int base) {
  return (soul.grudge > 600) ? (base > 2 ? base - 1 : 2) : base;
}

// چقدر بي‌توجهي تا خواب‌آلودگي؟
//   غريبه : ۶۰ ثانيه         دل‌بسته : ۹۰ ثانيه
//   (وقتي با تو خوب است ، بيشتر بيدار مي‌ماند)
inline uint32_t soulSleepyAfter(uint32_t base) {
  return base + (uint32_t)(base * 0.5f * soulBondF());
}

// فاصله‌ي خوشحالي‌هاي بي‌دليل
//   دل‌بسته زودتر و بيشتر خودبه‌خود خوشحال مي‌شود
inline uint32_t soulHappyGap(uint32_t base) {
  return base - (uint32_t)(base * 0.45f * soulBondF());
}

// وقتي ناراحتي‌اش تمام شد ، آيا عصباني مي‌شود؟
//   بستگي دارد چقدر از قبل دل‌خور باشد
inline bool soulHoldsGrudge() {
  return (int)random(1000) < soul.grudge;
}

// =====================================================
//  گزارشِ دوره‌اي
//
//  چرا لازم است: اثرِ حافظه **کُند** است. اگر عددها را
//  نبيني ، نمي‌فهمي کار مي‌کند يا نه — و همين امروز ديديم
//  که چند باگ فقط با ديدنِ عددِ واقعي پيدا شدند.
//
//  وقتي مطمئن شديم کار مي‌کند ، ۰ کن.
// =====================================================
#define SOUL_REPORT 1

inline void soulReport() {
#if SOUL_REPORT
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last < 20000) return;
  last = now;
  Serial.printf("[دل] پيوند %4d  رنجش %4d   %s   "
                "(عشق در %lus · اذيت با %d سقلمه · خواب در %lus)\n",
                soul.bond, soul.grudge, soulMoodName(),
                (unsigned long)(soulPetToLove(6000) / 1000),
                soulPokesToAnnoy(3),
                (unsigned long)(soulSleepyAfter(60000) / 1000));
#endif
}

// =====================================================
//  پاک کردن — براي وقتي که مي‌خواهي از صفر شروع کني
//  در Serial Monitor حرف `r` را بفرست.
// =====================================================
// فقط رنجش را پاک مي‌کند ، پيوند دست‌نخورده مي‌ماند.
// براي وقتي که آزمايش کرده‌اي و نمي‌خواهي موچي تاوانش را
// بدهد.
inline void soulForgive() {
  if (!soulReady) return;
  soul.grudge = 0;
  soulNvs.putShort("grudge", 0);
  soulSavedGrudge = 0;
  Serial.println("   🧠 بخشيد — رنجش صفر شد (پيوند دست‌نخورده)");
}

inline void soulReset() {
  if (!soulReady) return;
  soul.bond = 0;  soul.grudge = 0;
  soul.petSec = 0;  soul.pokes = 0;
  soulNvs.putShort("bond", 0);
  soulNvs.putShort("grudge", 0);
  soulNvs.putULong("petSec", 0);
  soulNvs.putULong("pokes", 0);
  soulSavedBond = 0;  soulSavedGrudge = 0;
  Serial.println("   🧠 حافظه پاک شد — موچي دوباره غريبه است");
}
