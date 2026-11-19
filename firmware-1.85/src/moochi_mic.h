#pragma once
// =====================================================
//   MOOCHI  —  گوش  (نسخه ۲ برد)
//
//   ⚠️ اين فايل يک بار کامل بازنويسي شد.
//
//   نسخه‌ي اول بر اساس شماتيک **نسخه ۱** نوشته شده بود و
//   خروجي‌اش هميشه صفر بود. علت: برد ما نسخه ۲ است و بخش
//   صدايش کاملا فرق دارد.
//
//   نسخه ۱ : ميکروفن ديجيتال ICS-43434 ، مستقيم روي I2S
//   نسخه ۲ : ميکروفن **آنالوگ** + چيپ ES7210 که آن را
//            ديجيتال مي‌کند — و بايد اول از طريق I2C
//            پيکربندي شود ، وگرنه ساکت مي‌ماند.
//
//   منبع پين‌ها: شماتيک نسخه ۲ ، تاييد شده با مثال رسمي
//   `Arduino/examples/08_esp_sr/08_esp_sr.ino`
// =====================================================

#include <Arduino.h>
#include <Wire.h>
#include <driver/i2s_std.h>
#include "es7210.h"

// ---------- پين‌ها (نسخه ۲) ----------
//
// ☠️ اشتباهِ نسخه‌ي قبل:
//      GPIO15 را کلاک گرفته بودم — در نسخه ۲ آن **کليد
//      فعال‌سازي آمپلي‌فاير** است ، نه کلاک.
//      GPIO2 را WS گرفته بودم — در نسخه ۲ آن **MCLK** است.
//      فقط GPIO39 تصادفا درست بود.
#define MIC_MCK    2     // کلاک اصلي — نسخه ۱ اصلا نداشت
#define MIC_BCK   48     // کلاک بيت
#define MIC_WS    38     // انتخاب چپ/راست
#define MIC_SD    39     // داده ، از ES7210 به ESP32
#define SPK_DOUT  47     // داده ، از ESP32 به ES8311 (بلندگو)
#define PA_CTRL   15     // فعال‌سازي آمپلي‌فاير

#define ES7210_ADDR      0x40
#define MIC_RATE        16000
#define MIC_MCLK_MULT     256    // نسبت MCLK به نرخ نمونه
#define MIC_FRAMES        256

// بهره‌ي ديجيتال ADC ، بر حسب دسي‌بل. بازه‌ي مجاز: ۹۵− تا ۳۲+
#define MIC_ADC_DB         6

// حالت کاليبراسيون — اعداد خام را هر نيم‌ثانيه چاپ مي‌کند.
// ميکروفن تاييد شد (۱۹ شهريور) ، پس خاموش. اگر روزي صدا
// عجيب شد ، ۱ کن و اعداد را ببين.
#define MIC_CALIBRATE  0

// صداهايي را که بلند بودند ولي دست زدن حساب نشدند چاپ مي‌کند
// تا بشود آستانه را با عددِ واقعي تنظيم کرد ، نه با حدس.
// تنظيم شد (۱۹ شهريور) ، پس خاموش.
#define MIC_NEARMISS   0

// ---------- آستانه‌هاي تشخيص دست زدن ----------
float    CLAP_RATIO = 6.0f;      // چند برابرِ صداي زمينه
// ☠️ ۰٫۰۲۰ خيلي پايين بود — حرف زدن هم رد مي‌شد.
//    در لاگ واقعي اعداد دو دسته‌ي روشن بودند:
//        0.3220  0.3618  0.4735   ← دست زدن
//        0.0222  0.0336  0.0427   ← حرف زدن
//    آستانه درست وسط دسته‌ي دوم افتاده بود.
//
// اگر دست زدنِ آرام گرفته نمي‌شود ، اين را کم کن.
// اگر هنوز با حرف زدن واکنش مي‌دهد ، زيادش کن.
float    CLAP_FLOOR = 0.110f;    // کمتر از اين اصلا حساب نمي‌شود

// ---------- تست دوم: تيزيِ حمله ----------
// اين تفاوتِ واقعيِ دست زدن و حرف زدن است ، نه بلندي.
//   دست زدن : در يک قاب (۱۶ ميلي‌ثانيه) از سکوت به اوج مي‌پرد
//   حرف زدن : صدا بالا مي‌آيد ، چند قاب طول مي‌کشد
// پس مي‌خواهيم اين قاب دست‌کم چند برابرِ **قابِ قبلي** باشد.
// ۶ برابر ، از روي اعداد واقعيِ لاگ انتخاب شد نه حدس:
//   دست زدن‌ها : 21x  7.2x  88x  41x  46x  15x  103x  6.4x  48x  45x
//   حرف زدن‌ها : 1.6x  2.3x  1.3x  ...  (بيشترين ۲٫۳)
// شکافْ پهن است ، پس ۶ راحت وسطش مي‌نشيند.
float    CLAP_ATTACK = 6.0f;

// ☠️ شرط «قبلش ساکت بوده» حذف شد.
//    براي فيلتر کردن صداي تلويزيون گذاشته بودمش ، ولي در
//    لاگ ديده شد که **دست زدنِ سريع را مي‌کشد**:
//       دست‌ها هر ۱۷۰ ميلي‌ثانيه ، طنينِ هرکدام ۱۵۰ ميلي‌ثانيه
//       → بينشان هيچ‌وقت سکوت نمي‌شود → هيچ‌کدام رد نمي‌شوند
//    آزمون تيزي همان کار را بهتر مي‌کند ، چون سکوت را در
//    بازه‌ي درست (يک قاب ، ۱۶ ميلي‌ثانيه) مي‌سنجد نه ۱۵۰.
//
// ۲۵۰ ميلي‌ثانيه: طنين تا اين‌جا خوابيده ، پس دوباره‌شماري
// نمي‌شود ؛ ولي دست زدنِ تند هم جا مي‌گيرد.
uint32_t CLAP_GAP   = 250;       // فاصله‌ي دو تشخيص پياپي

// ---------- تستِ سوم: دُمِ کوتاه ----------
//
// ☠️ سه آزمونِ قبلي در واقع «صداي بلندِ تيز» را مي‌گرفتند ،
//    نه دست زدن. و بستنِ در ، ضربه‌ي فنجان روي ميز ، سرفه و
//    افتادنِ چيزي همگي همان امضا را دارند: بلند ، ناگهاني ،
//    قبلش ساکت.
//
// نوشته بودم «دستِ آدم طنين ندارد» — و **غلط بود**.
// دست طنين ندارد ولي **اتاق دارد**. ميکروفن بازتابِ ديوار و
// سقف را مي‌شنود و همان دُم مي‌سازد.
//
// داده‌ي واقعيِ هشت دست زدن در اتاق:
//     50%  34%  40%  129%  69%  40%  41%  52%
// و من آستانه را روي ۲۲٪ گذاشته بودم. نتيجه: **همه‌ي**
// دست زدن‌هاي واقعي رد شدند و موچي از خواب بيدار نشد.
//
// و آن ۱۲۹٪ يک باگِ دوم را لو داد: دُم نمي‌تواند از اوج
// بلندتر باشد ، مگر اينکه اوج را اشتباه گرفته باشم. و
// گرفته بودم — **اولين** قابِ بلند را اوج مي‌گرفتم ، ولي
// صدا يکي دو قاب بعد به بيشينه مي‌رسد.
//
// درس: وقتي عددي «غيرممکن» درمي‌آيد (بيشتر از ۱۰۰٪) ، به
// جاي تنظيمِ آستانه دنبالِ غلطِ اندازه‌گيري بگرد.
#define PEAK_FRAMES   2          // دو قابِ اول ، براي يافتنِ اوجِ واقعي
#define TAIL_FRAMES   7          // تا اين قاب ، دُم را مي‌سنجيم
float    TAIL_MAX   = 0.75f;     // از داده‌ي واقعي ، نه حدس

// نشانگرِ عيب‌يابي — عددهاي واقعي را چاپ مي‌کند تا آستانه را
// با داده تنظيم کنيم نه با حدس. وقتي تنظيم شد ، ۰ کن.
#define CLAP_DEBUG 1

// =====================================================
//   ☠️ تشخيصِ کلمه‌ي «موچي» — حذف شد
//
//   امتحانش کرديم و کار نکرد. سه طراحيِ پشت سر هم ، هر سه
//   غلط ، و هر سه را لاگ لو داد نه فکر کردن:
//
//     ۱ فرض کردم بين «مو» و «چي» مکث هست
//       → همه‌ي کلمه‌ها «هجا=1» درآمدند ، ۳۵۲ و ۴۱۶ms.
//         در گفتار طبيعي «موچي» يک صداي پيوسته است.
//
//     ۲ فرض کردم نيمه‌ي دوم خش‌دارتر است
//       → عددها برعکس درآمدند: موچي ۰٫۴ ، مامان ۰٫۹
//
//     ۳ نوشتم «وسط را دور بينداز چون گذار است»
//       → و وسط دقيقا جايي بود که «چ» مي‌افتد. چيزي را
//         که دنبالش بودم با دست خودم انداختم دور.
//
//   وسوسه اين بود که مقايسه را برعکس کنم و بگويم درست شد.
//   ولي آن مي‌شد وصله زدن به چهار نمونه ، نه فهميدن.
//
//   راهِ درستش wake word واقعي است که بسته است: ESP-SR
//   براي کلمه‌ي سفارشي ۲۰٬۰۰۰ نمونه از ۵۰۰+ نفر (شامل
//   ۱۰۰ کودک) مي‌خواهد ، دو تا سه هفته آموزش در خود
//   اسپرسيف ، و هزينه.  (ESP_Wake_Words_Customization)
//
//   پس موچي به **حضورِ صدا** واکنش نشان مي‌دهد نه به معني.
//   حرف بزني ، گوش مي‌دهد. دست بزني ، جا مي‌خورد.
// =====================================================

// ---------- حضورِ صدا ----------
float    VOICE_ON     = 0.012f;  // بالاتر از اين يعني صدا هست
float    VOICE_OFF    = 0.007f;  // پايين‌تر از اين يعني ساکت

// مکثِ کوتاه‌تر از اين ، حرف زدن را قطع‌شده حساب نمي‌کند.
// نفس کشيدن و فاصله‌ي بين کلمه‌ها معمولا زير اين است.
uint32_t TALK_BREAK   = 600;

struct Mic {
  bool present = false;
  volatile float level = 0;
  volatile float baseNoise = 0;
  volatile float levelL = 0;
  volatile float levelR = 0;
  volatile bool  clap = false;
  volatile uint32_t lastClap = 0;
  volatile uint32_t claps = 0;

  volatile bool  talking = false;     // کسي دارد حرف مي‌زند
  volatile uint32_t talkStart = 0;
};
Mic mic;

static i2s_chan_handle_t     micChan   = nullptr;
static es7210_dev_handle_t   es7210    = nullptr;

// ☠️ چرا کانالِ بلندگو **اين‌جا** ساخته مي‌شود و نه در فايلِ صدا:
//
//    ميکروفن و بلندگو روي **يک** کنترلرِ I2S هستند و BCLK و
//    LRCK را به اشتراک مي‌گذارند. در ESP-IDF هر دو کانال بايد
//    در **يک** فراخوانيِ i2s_new_channel ساخته شوند ، وگرنه
//    کانالِ دوم پورتِ ديگري مي‌گيرد و پايه‌ها تداخل مي‌کنند.
//
//    پس مالکِ پورت همين فايل است. فايلِ صدا فقط دسته را
//    قرض مي‌گيرد.
i2s_chan_handle_t            spkChan   = nullptr;

// =====================================================
//  راه‌اندازي ES7210
//
//  تنظيم‌ها از مثال رسمي 08_esp_sr برداشته شده‌اند:
//    فرمت I2S · نسبت MCLK ۲۵۶ · ۱۶ کيلوهرتز · ۱۶ بيت
//    باياس ميکروفن ۲٫۸۷ ولت · بهره ۳۶ دسي‌بل · بلندي ۴۰
// =====================================================
inline bool es7210Init() {
  es7210_i2c_config_t i2cCfg = {};
  i2cCfg.i2c_addr = ES7210_ADDR;

  if (es7210_new_codec(&i2cCfg, &es7210) != ESP_OK) {
    Serial.println("   ❌ ES7210 ساخته نشد");
    return false;
  }

  es7210_codec_config_t cfg = {};
  cfg.i2s_format        = ES7210_I2S_FMT_I2S;
  cfg.mclk_ratio        = MIC_MCLK_MULT;
  cfg.sample_rate_hz    = MIC_RATE;
  cfg.bit_width         = ES7210_I2S_BITS_16B;
  cfg.mic_bias          = ES7210_MIC_BIAS_2V87;
  cfg.mic_gain          = ES7210_MIC_GAIN_36DB;
  cfg.flags.tdm_enable  = false;

  if (es7210_config_codec(es7210, &cfg) != ESP_OK) {
    Serial.println("   ❌ ES7210 پيکربندي نشد");
    return false;
  }
  // ☠️ مثال رسمي ويوشير اينجا عدد ۴۰ مي‌دهد — و **غلط است**.
  //    در es7210.cpp خط ۳۵۸ شرط اعتبار اين است:
  //        volume_db >= -95 && volume_db <= 32
  //    پس ۴۰ رد مي‌شود و ADC روي ۰ دسي‌بلِ پيش‌فرض مي‌ماند.
  //    مثال خودش هم ESP_ERROR_CHECK دورش گذاشته ، يعني بايد
  //    کرش کند. باز هم همان درس: مثال رسمي را هم بايد چک کرد.
  //
  //    ۶ دسي‌بل يعني دو برابر. با اين ، دست زدن حدود ۰٫۲ مي‌شود
  //    که هنوز خيلي زير سقف (۱٫۰) است و جا براي صداي بلندتر دارد.
  if (es7210_config_volume(es7210, MIC_ADC_DB) != ESP_OK) {
    Serial.println("   ⚠️ تنظيم بلندي ES7210 نشد");
  }
  return true;
}

// =====================================================
//  راه‌اندازي I2S
// =====================================================
inline bool micInit() {
  if (!es7210Init()) return false;      // اول چيپ ، بعد I2S

  i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  chanCfg.dma_desc_num  = 4;
  chanCfg.dma_frame_num = MIC_FRAMES;
  // هر دو کانال با هم — دوطرفه‌ي کامل روي يک پورت
  if (i2s_new_channel(&chanCfg, &spkChan, &micChan) != ESP_OK) return false;

  i2s_std_config_t stdCfg = {
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(MIC_RATE),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                    I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
      .mclk = (gpio_num_t)MIC_MCK,      // ← نسخه ۱ اين را نداشت
      .bclk = (gpio_num_t)MIC_BCK,
      .ws   = (gpio_num_t)MIC_WS,
      .dout = (gpio_num_t)SPK_DOUT,     // ← به ES8311
      .din  = (gpio_num_t)MIC_SD,       // ← از ES7210
      .invert_flags = { false, false, false },
    },
  };
  stdCfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

  //   در حالتِ دوطرفه هر دو کانال **همان** تنظيم را مي‌گيرند.
  //   کلاک و اسلات بايد يکي باشند ، وگرنه IDF خطا مي‌دهد.
  if (i2s_channel_init_std_mode(micChan, &stdCfg) != ESP_OK) return false;
  if (i2s_channel_init_std_mode(spkChan, &stdCfg) != ESP_OK) {
    Serial.println("   ⚠️ کانالِ بلندگو راه نيفتاد — صدا خاموش");
    spkChan = nullptr;
  }
  if (i2s_channel_enable(micChan) != ESP_OK) return false;
  if (spkChan && i2s_channel_enable(spkChan) != ESP_OK) spkChan = nullptr;

  mic.present = true;
  return true;
}

// =====================================================
//  کارگرِ گوش — روي هسته‌ي دوم
//
//  چرا تسک جدا؟ حلقه‌ي اصلي دارد ۳۰ بار در ثانيه صورت
//  مي‌کشد. اگر همان‌جا منتظر صدا بمانيم انيميشن لِه مي‌شود.
// =====================================================
inline void micTask(void *) {
  static int16_t buf[MIC_FRAMES * 2];
  uint32_t lastPrint = 0;

  while (true) {
    size_t got = 0;
    if (i2s_channel_read(micChan, buf, sizeof(buf), &got, 100) != ESP_OK || got == 0) {
      vTaskDelay(pdMS_TO_TICKS(5));
      continue;
    }

    int n = got / sizeof(int16_t) / 2;
    // 🛡 خواندنِ ناقص (کمتر از يک قابِ کامل) n را صفر مي‌کند
    //    و پايين‌تر sumL/n تقسيم بر صفر مي‌شود. I2S معمولا
    //    قاب‌کامل مي‌دهد ، ولي «معمولا» تضمين نيست.
    if (n <= 0) { vTaskDelay(pdMS_TO_TICKS(2)); continue; }
    double sumL = 0, sumR = 0;
    int16_t rawMinL = INT16_MAX, rawMaxL = INT16_MIN;
    int16_t rawMinR = INT16_MAX, rawMaxR = INT16_MIN;

    for (int i = 0; i < n; i++) {
      int16_t rl = buf[i * 2], rr = buf[i * 2 + 1];
      if (rl < rawMinL) rawMinL = rl;   if (rl > rawMaxL) rawMaxL = rl;
      if (rr < rawMinR) rawMinR = rr;   if (rr > rawMaxR) rawMaxR = rr;
      float l = (float)rl / 32768.0f;
      float r = (float)rr / 32768.0f;
      sumL += (double)l * l;
      sumR += (double)r * r;
    }

    // RMS = ريشه‌ي ميانگين مربع‌ها ، يعني «بلندي».
    // چرا مربع؟ صدا هم مثبت مي‌شود هم منفي ؛ جمع ساده
    // همديگر را خنثي مي‌کند و هميشه صفر درمي‌آيد.
    float rmsL = sqrtf(sumL / n);
    float rmsR = sqrtf(sumR / n);
    float rms  = max(rmsL, rmsR);

    mic.levelL = rmsL;
    mic.levelR = rmsR;
    mic.level  = rms;

    // ---------- ردياب کفِ نويز ----------
    //
    // ☠️ نسخه‌ي اول اين بود:
    //       baseNoise = baseNoise * 0.995 + rms * 0.005
    //    يعني **خودِ صداي دست زدن هم وارد ميانگين مي‌شد**.
    //    در لاگ واقعي ديده شد:
    //       دست اول → زمينه ۰٫۰۰۲۵
    //       دست دوم → زمينه ۰٫۰۰۲۹
    //       دست سوم → زمينه ۰٫۰۰۳۴
    //    هر دست زدن کف را بالا مي‌برد ، پس دست بعدي بايد بلندتر
    //    مي‌بود. در اتاق شلوغ کم‌کم کر مي‌شد.
    //
    // ☠️ نسخه‌ي دومش هم غلط بود — و بدتر ، چون **بي‌صدا** غلط بود.
    //    نوشته بودم:
    //       else if (rms < baseNoise * 2.0f) baseNoise += ...
    //    يعني بالا رفتن فقط وقتي مجاز بود که صدا کمتر از دو برابرِ
    //    کف باشد. ولي baseNoise از صفر شروع مي‌شود و فوري به
    //    ۰٫۰۰۰۴ گير مي‌کند ، و نويز واقعي اتاق ۰٫۰۰۳۴ است.
    //       0.0034 < 0.0008 ?  هرگز.
    //    پس شرط هيچ‌وقت باز نمي‌شد و رديابْ از لحظه‌ي روشن شدن
    //    قفل بود. در لاگ ۲۷ ثانيه‌اي ديده شد: زمينه ثابت ۰٫۰۰۰۴.
    //
    // درسِ اين باگ: شرطي که به خروجيِ خودش وابسته است مي‌تواند
    // قفل شود. هميشه بپرس «اگر از بدترين حالت شروع کند ، راهي
    // براي بيرون آمدن دارد؟»
    //
    // حالا: هر دو جهت هميشه باز است. جلوي بالا رفتنِ ناگهاني را
    // به جاي شرط ، با **سقفِ گام** مي‌گيريم — هر قاب حداکثر
    // يک درصد. يک دست زدن ۶ قاب طول مي‌کشد ، پس حداکثر ۶٪ کف
    // را بالا مي‌برد که بي‌اثر است ؛ ولي کولري که روشن بماند
    // در چند ثانيه پذيرفته مي‌شود.
    float d = rms - mic.baseNoise;
    if (d < 0) {
      mic.baseNoise += d * 0.05f;                       // پايين: سريع
    } else {
      float up  = d * 0.02f;
      float cap = mic.baseNoise * 0.01f + 0.000002f;    // سقفِ گام
      mic.baseNoise += (up < cap) ? up : cap;
    }

    if (mic.baseNoise < 0.0004f) mic.baseNoise = 0.0004f;

    uint32_t now = millis();

    // ---------- تشخيص دست زدن ----------
    // سه آزمون ، و هر سه بايد قبول شوند:
    //   ۱ بلندتر از چند برابرِ کفِ نويز        (نسبي)
    //   ۲ بلندتر از يک حدِ مطلق                (تا حرف زدن رد شود)
    //   ۳ تيز ــ چند برابرِ قابِ بلافاصله قبلي  (تا ضربه از صدا جدا شود)

    // قابِ قبلي را نگه مي‌داريم تا تيزيِ حمله را بسنجيم
    static float prevRms = 0;
    bool sharp = (rms > prevRms * CLAP_ATTACK);
    float shownPrev = prevRms;
    prevRms = rms;

    bool loud = (rms > mic.baseNoise * CLAP_RATIO) && (rms > CLAP_FLOOR);

    // ---------- نامزدِ دست زدن ، و انتظار براي ديدنِ دُم ----------
    //
    // ☠️ اين‌جا تصميم **فوري** گرفته مي‌شد و همين مشکل بود:
    //    در لحظه‌ي اوج هنوز نمي‌داني صدا طنين دارد يا نه.
    //    پس اوج را ثبت مي‌کنيم ، چهار قاب صبر مي‌کنيم ، و
    //    بعد قضاوت.
    //
    //    قيمتش ۶۴ ميلي‌ثانيه تاخير است که حس نمي‌شود.
    static bool  pend     = false;
    static float pendPeak = 0;
    static float pendTail = 0;
    static int   pendAge  = 0;
    static float pendPrev = 0;

    if (!pend && now > 2000 && loud && sharp
        && (now - mic.lastClap) > CLAP_GAP) {
      pend = true;  pendPeak = rms;  pendTail = 0;
      pendAge = 0;  pendPrev = shownPrev;
    }
    else if (pend) {
      pendAge++;
      // دو قابِ اول هنوز بالا رفتن است — اوج را به‌روز کن
      if (pendAge <= PEAK_FRAMES) {
        if (rms > pendPeak) pendPeak = rms;
      } else if (rms > pendTail) {
        pendTail = rms;                          // بلندترين قابِ دُم
      }
      if (pendAge >= TAIL_FRAMES) {
        pend = false;
        float ratio = (pendPeak > 0) ? pendTail / pendPeak : 1.0f;
        bool  crisp = (ratio < TAIL_MAX);
        if (crisp) {
          mic.lastClap = now;
          mic.claps = mic.claps + 1;
          mic.clap = true;
        }
#if CLAP_DEBUG
        Serial.printf("[گوش] %s  اوج=%.3f دُم=%.3f (%2.0f%%)  قاب‌قبل=%.4f\n",
                      crisp ? "👏 تيز" : "·  طنين‌دار ، رد",
                      pendPeak, pendTail, ratio * 100.0f, pendPrev);
#endif
      }
    }
#if MIC_NEARMISS
    // صداهايي که بلند بودند ولي رد شدند — براي تنظيم آستانه
    // فقط صداهاي واقعا بلند ــ وگرنه لاگ پر مي‌شود و
    // چيزي که دنبالش هستيم گم مي‌شود
    else if (now > 2000 && rms > CLAP_FLOOR && (now - mic.lastClap) > CLAP_GAP) {
      Serial.printf("[گوش] ·  رد شد   بلندي=%.4f  قاب‌قبل=%.4f  (%s%s)\n",
                    rms, shownPrev,
                    loud  ? "" : "آرام ",
                    sharp ? "" : "نرم");
    }
#endif

    // ---------- حضورِ صدا ----------
    // پسماند: وقتي صدا شروع شد آستانه‌ي پايين‌تر مي‌گيريم ،
    // وگرنه هر افتِ کوچکِ وسطِ حرف ، صدا را تکه‌تکه مي‌کند.
    static bool inVoice = false;
    bool voiced = inVoice ? (rms > VOICE_OFF) : (rms > VOICE_ON);
    inVoice = voiced;

    // ---------- کسي دارد حرف مي‌زند؟ ----------
    //
    // ☠️ نسخه‌ي اول اشتباه بود: در پايانِ هر کلمه
    //    `mic.talking = false` مي‌گذاشتم ، پس شمارنده بعد از
    //    **هر کلمه** صفر مي‌شد. و «کلمه» يعني هر صدايي که
    //    ۱۳۰ ميلي‌ثانيه سکوت دنبالش بيايد — تقريبا هر کلمه‌ي
    //    معمولي. پس شرطِ «۱٫۵ ثانيه پيوسته» عملا يعني يک
    //    کلمه‌ي ۱٫۵ ثانيه‌اي بدون نفس کشيدن. هيچ‌وقت نمي‌شد.
    //    کاربر گزارش داد: «اصلا گوش نمي‌ده» — و درست بود.
    //
    // حرف زدن يعني «صدا با مکث‌هاي کوتاه ادامه دارد» ، نه
    // «يک صداي ممتد». پس مکثِ کمتر از TALK_BREAK را قطع
    // حساب نمي‌کنيم و از روي کلمه‌ها رد مي‌شويم.
    static uint32_t talkVoiceAt = 0;
    if (voiced) {
      if (!mic.talking || (now - talkVoiceAt) > TALK_BREAK) {
        mic.talking = true;  mic.talkStart = now;
      }
      talkVoiceAt = now;
    } else if (mic.talking && (now - talkVoiceAt) > TALK_BREAK) {
      mic.talking = false;
    }

#if MIC_CALIBRATE
    if (now - lastPrint > 500) {
      lastPrint = now;
      Serial.printf("[گوش] چپ: %d..%d   راست: %d..%d   بلندي=%.4f  زمينه=%.4f\n",
                    (int)rawMinL, (int)rawMaxL, (int)rawMinR, (int)rawMaxR,
                    rms, mic.baseNoise);
    }
#endif
  }
}

inline void micStart() {
  if (!mic.present) return;
  xTaskCreatePinnedToCore(micTask, "mic", 4096, nullptr, 1, nullptr, 0);
}

inline bool micTakeClap() {
  if (!mic.clap) return false;
  mic.clap = false;
  return true;
}

// کسي دست‌کم اين‌قدر ميلي‌ثانيه است که يک‌ريز حرف مي‌زند؟
inline bool micTalkingFor(uint32_t ms) {
  return mic.talking && (millis() - mic.talkStart) >= ms;
}
