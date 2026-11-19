#pragma once
// =====================================================
//   MOOCHI  —  حواس
//   لمس (CST816)  و  حرکت (شتاب‌سنج)
//
//   ⚠️ آنچه قطعي است و آنچه نيست
//   ---------------------------------------------------
//   قطعي  : CST816 روي آدرس 0x15 باس اصلي ديده شده
//   قطعي  : TCA9554 روي 0x20 ديده شده
//   نامعلوم: QMI8658 (ژيروي داخلي برد) روي باس ديده نشد.
//            بايد 0x6A يا 0x6B باشد و نيست.
//
//   براي همين کد هيچ فرضي نمي‌کند: هر سه احتمال را امتحان
//   مي‌کند و اگر هيچ‌کدام نبود، بقيه‌ي ربات کار مي‌کند و
//   فقط حرکت‌محورها خاموش مي‌شوند. در Serial دقيقا مي‌گويد
//   چه پيدا شد و چه نشد.
// =====================================================

#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>

// ---------- پين‌ها ----------
#define I2C_SDA   11      // باس اصلي برد
#define I2C_SCL   10

// ☠️☠️ هرگز GPIO19 و GPIO20 را استفاده نکن ☠️☠️
//
//   روي ESP32-S3:   GPIO19 = USB_D−   ·   GPIO20 = USB_D+
//
//   اين دو پين روي هدر برد بيرون آمده‌اند و در يادداشت‌هاي
//   اوليه به اشتباه «باس I2C دوم» نوشته شده بودند. ولي ما
//   از همان USB داخلي براي Serial و آپلود استفاده مي‌کنيم.
//   اگر Wire1.begin(20,19) صدا زده شود، فرمور در همان لحظه
//   کابل USB خودش را قطع مي‌کند: پورت COM ناپديد مي‌شود و
//   آپلود بعدي مي‌گويد «پورت وجود ندارد».
//   يک بار اين اتفاق افتاد. تنها راه برگشت BOOT+RST است.
//
//   پس باس دوم روي دو پين آزاد ديگرِ همان هدر مي‌نشيند:
#define I2C2_SDA  13
#define I2C2_SCL  12

// باس دوم روي GPIO13/12 — هر دو در کلِ پروژه آزادند ، چک شد.
// روشن است چون MPU6050 نصب شده. اگر چيزي رويش نباشد هم ضرري
// ندارد: Wire1.begin روي دو پينِ آزاد فقط دو پين را مي‌گيرد.
//
// ☠️ يادآوري: هرگز ۱۹ و ۲۰ — آن‌ها USB هستند.
#define USE_SECOND_BUS  1

// ---------- آدرس‌ها ----------
#define ADDR_TOUCH    0x15   // CST816
#define ADDR_QMI_A    0x6A   // QMI8658 ، حالت اول
#define ADDR_QMI_B    0x6B   // QMI8658 ، حالت دوم
// ☠️ MPU6050 **دو** آدرس دارد و اين را جا انداخته بودم:
//      پايه‌ي AD0 پايين (يا شناور روي GY-521) ⇒ 0x68
//      پايه‌ي AD0 بالا                        ⇒ 0x69
//    روي بعضي ماژول‌هاي GY-521 مقاومتِ pull-down نصب نشده و
//    AD0 شناور مي‌ماند ؛ آن‌وقت آدرس مي‌تواند 0x69 دربيايد.
//    هزينه‌ي چک کردنِ آدرسِ دوم صفر است ، پس هر دو را مي‌زنيم.
#define ADDR_MPU      0x68
#define ADDR_MPU_B    0x69

// =====================================================
//  کمک‌کارهاي I2C
// =====================================================
inline bool i2cPing(TwoWire &bus, uint8_t addr) {
  bus.beginTransmission(addr);
  return bus.endTransmission() == 0;
}
inline bool i2cWrite8(TwoWire &bus, uint8_t addr, uint8_t reg, uint8_t val) {
  bus.beginTransmission(addr);
  bus.write(reg);
  bus.write(val);
  return bus.endTransmission() == 0;
}
inline bool i2cRead(TwoWire &bus, uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t n) {
  bus.beginTransmission(addr);
  bus.write(reg);
  if (bus.endTransmission(false) != 0) return false;
  if ((uint8_t)bus.requestFrom((int)addr, (int)n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = bus.read();
  return true;
}

// =====================================================
//  لمس — CST816
//
//  نگاشت رجيسترها (استاندارد CST816S):
//    0x01 حرکت (gesture) · 0x02 تعداد انگشت
//    0x03/0x04 مختصات X  · 0x05/0x06 مختصات Y
//
//  عمدا از پين وقفه (INT) استفاده نمي‌کنيم چون شماره‌اش
//  روي اين برد تاييد نشده. به جايش هر فريم پرسش مي‌کنيم
//  که هزينه‌اش ناچيز است.
// =====================================================
// ---------- حرکت‌هاي خودِ چيپ ----------
//
// 🎁 اينها را از روز اول داشتيم و نمي‌خوانديمش. بايتِ اولِ
//    همان شش بايتي که هر فريم مي‌خوانيم (b[0]) کدِ حرکت است ،
//    و چيپ خودش تشخيصشان مي‌دهد — ما فقط دور ريخته بوديم.
//
// منبع: Arduino/examples/01_lvgl_example/Touch_CST816.h
enum TouchGesture : uint8_t {
  TG_NONE   = 0x00,
  TG_UP     = 0x01,
  TG_DOWN   = 0x02,
  TG_LEFT   = 0x03,
  TG_RIGHT  = 0x04,
  TG_TAP    = 0x05,   // يک بار زدن
  TG_DOUBLE = 0x0B,   // دو بار زدن
  TG_HOLD   = 0x0C    // نگه داشتن
};

inline const char *gestureName(uint8_t g) {
  switch (g) {
    case TG_UP:     return "کشيدن به بالا";
    case TG_DOWN:   return "کشيدن به پايين";
    case TG_LEFT:   return "کشيدن به چپ";
    case TG_RIGHT:  return "کشيدن به راست";
    case TG_TAP:    return "يک ضربه";
    case TG_DOUBLE: return "دو ضربه";
    case TG_HOLD:   return "نگه داشتن";
  }
  return "-";
}

struct Touch {
  bool present = false;    // آيا چيپ روي باس هست
  bool pressed = false;
  int  x = 0, y = 0;
  uint8_t chipId = 0;
  uint8_t gesture = TG_NONE;   // حرکتِ اين فريم ، يک بار مصرف
};
Touch touch;

// ⚠️ مهم: CST816 خواب خودکار دارد
//   بعد از چند ثانيه بي‌کاري به حالت کم‌مصرف مي‌رود و ديگر
//   به پرسش I2C جواب درست نمي‌دهد. چون ما به جاي پين وقفه
//   داريم مدام مي‌پرسيم، بايد اين خواب خاموش شود:
//        رجيستر 0xFE (DisAutoSleep) = 0x01
//   بدون اين خط، لمس چند ثانيه‌ي اول کار مي‌کند و بعد مي‌ميرد.
inline void touchKeepAwake() {
  i2cWrite8(Wire, ADDR_TOUCH, 0xFE, 0x01);
}

// ⚠️ ريست سخت‌افزاري چيپ لمس
//
//   طبق ويکي Waveshare پايه‌ي ريست چيپ لمس روي EXIO1 از
//   گسترش‌دهنده‌ي TCA9554 است (آدرس 0x20 ، رجيستر خروجي 0x01).
//   نگاشت: EXIO1 = بيت ۰ · EXIO2 = بيت ۱ (ريست نمايشگر)
//
//   قبلا اين پايه را فقط بالا نگه مي‌داشتيم و هيچ‌وقت پالس
//   نمي‌زديم. نتيجه: چيپ گاهي بالا مي‌آمد و گاهي نه — همان
//   «يک بار کار مي‌کند ، يک بار نه»ي که ديديم.
//
//   ☠️ بيت ۱ را دست نزن — ريست نمايشگر است و صفحه سياه مي‌شود.
#define TCA_ADDR      0x20
#define TCA_REG_OUT   0x01
#define TCA_REG_CONF  0x03
#define EXIO_TP_RST   0x01      // بيت ۰

inline void touchHardReset() {
  uint8_t out = 0xFF;
  i2cRead(Wire, TCA_ADDR, TCA_REG_OUT, &out, 1);   // وضعيت فعلي را نگه دار
  i2cWrite8(Wire, TCA_ADDR, TCA_REG_CONF, 0x00);   // همه پايه‌ها خروجي
  i2cWrite8(Wire, TCA_ADDR, TCA_REG_OUT, out & ~EXIO_TP_RST);   // پايين
  delay(20);
  i2cWrite8(Wire, TCA_ADDR, TCA_REG_OUT, out |  EXIO_TP_RST);   // بالا
  delay(60);                                        // فرصت بالا آمدن
}

inline bool touchInit() {
  touchHardReset();

  // سه بار تلاش — اگر بار اول جواب نداد ، دوباره ريست کن
  for (int attempt = 0; attempt < 3; attempt++) {
    if (i2cPing(Wire, ADDR_TOUCH)) {
      touch.present = true;
      i2cRead(Wire, ADDR_TOUCH, 0xA7, &touch.chipId, 1);   // شناسه‌ي چيپ
      touchKeepAwake();
      if (attempt) Serial.printf("   (لمس در تلاش %d آمد)\n", attempt + 1);
      return true;
    }
    Serial.printf("   … لمس جواب نداد ، تلاش %d\n", attempt + 1);
    touchHardReset();
  }
  touch.present = false;
  return false;
}

// =====================================================
//  ريست مرحله‌اي — بدون delay
//
//  ⚠️ چرا مرحله‌اي؟
//    ريست سخت‌افزاري به دو مکث نياز دارد (۲۰ و ۶۰ ميلي‌ثانيه).
//    اگر با delay بنويسيمش ، هر بار که اجرا شود حلقه‌ي اصلي
//    ۸۰ ميلي‌ثانيه قفل مي‌شود و انيميشن مي‌پرد. پس به جاي
//    انتظار ، زمان را نگه مي‌داريم و در فريم‌هاي بعدي ادامه
//    مي‌دهيم.
//
//    همان قاعده‌ي کلِ پروژه: هيچ delay در حلقه‌ي اصلي.
// =====================================================
uint8_t  tpResetStage = 0;      // ۰ بي‌کار · ۱ پايين نگه داشته · ۲ بالا آمده
uint32_t tpResetAt    = 0;

inline void touchResetBegin() {
  uint8_t out = 0xFF;
  i2cRead(Wire, TCA_ADDR, TCA_REG_OUT, &out, 1);
  i2cWrite8(Wire, TCA_ADDR, TCA_REG_CONF, 0x00);
  i2cWrite8(Wire, TCA_ADDR, TCA_REG_OUT, out & ~EXIO_TP_RST);
  tpResetStage = 1;
  tpResetAt    = millis() + 20;
}

inline void touchResetStep() {
  if (tpResetStage == 0)     return;
  if (millis() < tpResetAt)  return;

  if (tpResetStage == 1) {
    uint8_t out = 0xFF;
    i2cRead(Wire, TCA_ADDR, TCA_REG_OUT, &out, 1);
    i2cWrite8(Wire, TCA_ADDR, TCA_REG_OUT, out | EXIO_TP_RST);
    tpResetStage = 2;
    tpResetAt    = millis() + 60;      // فرصت بالا آمدن
  } else {
    tpResetStage = 0;
    touch.present = i2cPing(Wire, ADDR_TOUCH);
    touchKeepAwake();
  }
}

inline void touchRead() {
  // اگر ريستي در جريان است ، تمامش کن و اين فريم چيزي نخوان
  if (tpResetStage) { touchResetStep(); touch.pressed = false; return; }

  if (!touch.present) {
    // چيپ گم شده — هر دو ثانيه دوباره صدايش کن
    static uint32_t lastTry = 0;
    if (millis() - lastTry > 2000) { lastTry = millis(); touchResetBegin(); }
    touch.pressed = false;
    return;
  }

  // هر دو ثانيه دوباره بيدارش نگه دار
  static uint32_t lastAwake = 0;
  if (millis() - lastAwake > 2000) { lastAwake = millis(); touchKeepAwake(); }

  // ❌ تازه‌سازي دوره‌اي برداشته شد — فرضيه غلط بود.
  //
  //   فکر مي‌کردم چيپ بعد از بي‌لمسي طولاني مي‌ميرد ، پس هر
  //   ۱۵ ثانيه ريستش مي‌کردم. ولي لاگ خلافش را نشان داد:
  //   بعد از ۱۸۱ ثانيه خواب ، چيپ همچنان ack=1 rd=1 بود و
  //   لمس را درست گزارش کرد.
  //
  //   بدتر اينکه آن ريست هر ۱۵ ثانيه يک پنجره‌ي کورِ حدود صد
  //   ميلي‌ثانيه‌اي مي‌ساخت که خودش مي‌توانست لمس را بخورد.
  //   يعني «راه‌حل» داشت باگ توليد مي‌کرد.
  //
  //   ريست فقط وقتي اجرا مي‌شود که چيپ واقعا جواب ندهد —
  //   همان مسير failCount پايين‌تر.

  // اگر چيپ چند بار پشت سر هم جواب نداد ، از نو راه‌اندازي‌اش کن.
  // ممکن است ريست شده باشد يا به حالتي رفته باشد که انتظارش را نداريم.
  static uint8_t failCount = 0;

  // ---------- نشانگر خام ----------
  // هر ۳ ثانيه: آيا چيپ روي باس جواب مي‌دهد ، و شش بايتي که
  // مي‌خوانيم چيست. در لحظه‌ي خرابي ، همين شش بايت تکليف را
  // روشن مي‌کند:
  //   ack=0                → چيپ اصلا جواب نمي‌دهد
  //   ack=1 و بايت‌ها ثابت → جواب مي‌دهد ولي حس نمي‌کند
  //   بايت‌ها عوض مي‌شوند  → حس مي‌کند و ايراد جاي ديگري است
// خاموش شد — لمس مدتي است بي‌ايراد کار مي‌کند و اين هر سه
// ثانيه لاگ را پر مي‌کرد. اگر دوباره قطع و وصل شد ، ۱ کن.
#define DEBUG_TOUCH_RAW 0
#if DEBUG_TOUCH_RAW
  {
    static uint32_t lastRaw = 0;
    if (millis() - lastRaw > 3000) {
      lastRaw = millis();
      uint8_t p[6] = {0, 0, 0, 0, 0, 0};
      bool ack = i2cPing(Wire, ADDR_TOUCH);
      bool rd  = i2cRead(Wire, ADDR_TOUCH, 0x01, p, 6);
      Serial.printf("[لمس‌خام] ack=%d rd=%d  %02X %02X %02X %02X %02X %02X\n",
                    ack, rd, p[0], p[1], p[2], p[3], p[4], p[5]);
    }
  }
#endif

  uint8_t b[6];
  if (!i2cRead(Wire, ADDR_TOUCH, 0x01, b, 6)) {
    touch.pressed = false;
    if (++failCount > 20) {
      failCount = 0;
      Serial.println("[لمس] جواب نمي‌دهد — ريست سخت‌افزاري");
      touchResetBegin();
    }
    return;
  }
  failCount = 0;

  // ---------- حرکت ----------
  // چيپ کدِ حرکت را تا وقتي حرکتِ تازه‌اي نيامده نگه مي‌دارد ،
  // پس بايد فقط **لبه** را بگيريم وگرنه هر فريم دوباره
  // شليک مي‌شود.
  {
    static uint8_t lastG = TG_NONE;
    uint8_t g = b[0];
    touch.gesture = (g != TG_NONE && g != lastG) ? g : TG_NONE;
    lastG = g;
  }

  uint8_t fingers = b[1];
  touch.pressed = (fingers > 0 && fingers < 3);
  if (touch.pressed) {
    touch.x = ((b[2] & 0x0F) << 8) | b[3];
    touch.y = ((b[4] & 0x0F) << 8) | b[5];
  }
}

// =====================================================
//  شتاب‌سنج
//
//  هر دو چيپ در بازه‌ي ±4g روي 8192 واحد بر g تنظيم
//  مي‌شوند، پس بقيه‌ي کد نيازي به دانستن نوع چيپ ندارد.
// =====================================================
enum ImuKind { IMU_NONE, IMU_QMI, IMU_MPU };

struct Imu {
  ImuKind  kind = IMU_NONE;
  TwoWire *bus  = &Wire;
  uint8_t  addr = 0;
  float ax = 0, ay = 0, az = 1.0f;   // بر حسب g
  float mag = 1.0f;                  // اندازه‌ي بردار
  const char *name = "—";            // براي گزارش و آزمايش
};
Imu imu;

// ---- QMI8658 ----
inline bool qmiSetup(TwoWire &bus, uint8_t addr) {
  uint8_t who = 0;
  if (!i2cRead(bus, addr, 0x00, &who, 1)) return false;
  if (who != 0x05) return false;                 // WHO_AM_I
  i2cWrite8(bus, addr, 0x02, 0x60);              // CTRL1: افزايش خودکار آدرس
  i2cWrite8(bus, addr, 0x03, 0x14);              // CTRL2: ±4g ، 250Hz
  i2cWrite8(bus, addr, 0x08, 0x01);              // CTRL7: شتاب‌سنج روشن
  delay(20);
  return true;
}
inline bool qmiRead(float &ax, float &ay, float &az) {
  uint8_t b[6];
  if (!i2cRead(*imu.bus, imu.addr, 0x35, b, 6)) return false;   // AccX_L ...
  int16_t rx = (int16_t)((b[1] << 8) | b[0]);                   // little-endian
  int16_t ry = (int16_t)((b[3] << 8) | b[2]);
  int16_t rz = (int16_t)((b[5] << 8) | b[4]);
  ax = rx / 8192.0f;  ay = ry / 8192.0f;  az = rz / 8192.0f;
  return true;
}

// ---- MPU6050 ----
inline bool mpuSetup(TwoWire &bus, uint8_t addr) {
  uint8_t who = 0;
  if (!i2cRead(bus, addr, 0x75, &who, 1)) return false;
  if (who != 0x68 && who != 0x70 && who != 0x71) return false;  // 6050 / 6500 / 9250
  i2cWrite8(bus, addr, 0x6B, 0x00);              // بيدار شو
  delay(10);
  i2cWrite8(bus, addr, 0x1C, 0x08);              // ±4g
  delay(10);
  return true;
}
inline bool mpuRead(float &ax, float &ay, float &az) {
  uint8_t b[6];
  if (!i2cRead(*imu.bus, imu.addr, 0x3B, b, 6)) return false;
  int16_t rx = (int16_t)((b[0] << 8) | b[1]);                   // big-endian
  int16_t ry = (int16_t)((b[2] << 8) | b[3]);
  int16_t rz = (int16_t)((b[4] << 8) | b[5]);
  ax = rx / 8192.0f;  ay = ry / 8192.0f;  az = rz / 8192.0f;
  return true;
}

// همه‌ي احتمال‌ها را به ترتيب امتحان مي‌کند
inline bool imuInit() {
  struct { TwoWire *bus; uint8_t addr; ImuKind kind; const char *name; } cand[] = {
    { &Wire,  ADDR_QMI_A, IMU_QMI, "QMI8658 @0x6A (باس اصلي)" },
    { &Wire,  ADDR_QMI_B, IMU_QMI, "QMI8658 @0x6B (باس اصلي)" },
    { &Wire,  ADDR_MPU,   IMU_MPU, "MPU6050 @0x68 (باس اصلي)" },
    { &Wire,  ADDR_MPU_B, IMU_MPU, "MPU6050 @0x69 (باس اصلي)" },
#if USE_SECOND_BUS
    { &Wire1, ADDR_QMI_A, IMU_QMI, "QMI8658 @0x6A (باس دوم)"  },
    { &Wire1, ADDR_QMI_B, IMU_QMI, "QMI8658 @0x6B (باس دوم)"  },
    { &Wire1, ADDR_MPU,   IMU_MPU, "MPU6050 @0x68 (باس دوم)"  },
    { &Wire1, ADDR_MPU_B, IMU_MPU, "MPU6050 @0x69 (باس دوم)"  },
#endif
  };
  for (auto &c : cand) {
    if (!i2cPing(*c.bus, c.addr)) continue;
    bool ok = (c.kind == IMU_QMI) ? qmiSetup(*c.bus, c.addr)
                                  : mpuSetup(*c.bus, c.addr);
    if (ok) {
      imu.bus = c.bus; imu.addr = c.addr; imu.kind = c.kind;
      imu.name = c.name;
      Serial.printf("   ✅ شتاب‌سنج: %s\n", c.name);
      Serial.println("      i در مانيتور = آزمايشِ زنده");
      return true;
    }
  }
  imu.kind = IMU_NONE;
  return false;
}

inline void imuRead() {
  if (imu.kind == IMU_NONE) return;
  float ax, ay, az;
  bool ok = (imu.kind == IMU_QMI) ? qmiRead(ax, ay, az) : mpuRead(ax, ay, az);
  if (!ok) return;
  imu.ax = ax; imu.ay = ay; imu.az = az;
  imu.mag = sqrtf(ax * ax + ay * ay + az * az);
}

// =====================================================
//   پين‌ياب   —   «سيمم را کجا زده‌ام؟»
//
//   ☠️ مسئله‌اي که سندِ پينوت حل نمي‌کند:
//      کانکتورِ J7 هيچ شماره‌اي روي خودش ندارد و فاصله‌اش
//      ۱٫۲۷ ميلي‌متر است. کاربر سيم را زده ، ولي نمي‌داند به
//      کدام سوراخ. سند مي‌گويد «پين ۶ و ۸» — که بدونِ شمردن
//      از يک سرِ معلوم بي‌فايده است.
//
//   راهِ حل: نگذار آدم حدس بزند ، از **خودِ برد** بپرس.
//
//   دو آزمايشِ پشتِ سرِ هم:
//
//     ۱ کدام پايه‌ها بالاکش دارند؟
//       ماژولِ MPU6050 روي خطِ SDA و SCL دو مقاومتِ ۱۰ کيلوييِ
//       بالاکش به VCC دارد. اگر پايه‌ي ESP را با **پايين‌کشِ
//       داخلي** (حدود ۴۵ کيلو) بخوانيم ، پايه‌ي آزاد پايين
//       مي‌خواند ولي پايه‌اي که به SDA/SCLِ ماژول وصل است
//       **بالا** مي‌خواند — چون ۱۰ کيلو بر ۴۵ کيلو غلبه مي‌کند.
//       اين دو پايه را بدونِ هيچ حدسي لو مي‌دهد.
//       (فقط اگر ماژول برق داشته باشد — که خودش هم يک خبر است.)
//
//     ۲ هر جفتِ ممکن را به عنوان باس امتحان کن.
//       دوازده پايه يعني ۱۳۲ جفتِ مرتب. هر کدام دو پينگ.
//       چند صد ميلي‌ثانيه طول مي‌کشد و جواب قطعي است.
//
//   ☠️ ۱۹ و ۲۰ عمدا در فهرست نيستند — USB هستند.
//   ☠️ نسخه‌ي اولِ اين ابزار **۳۲ جوابِ قلابي** داد. دو چيز را
//      جا انداخته بودم و هر دو در سندِ کانکتورِ خودمان بود:
//
//        GPIO8 = ولتاژِ باتري. روي برد يک مقسمِ مقاومتي دارد که
//                خط را به زمين مي‌کشد. خطِ SDA که دائم پايين
//                باشد يعني **هر آدرسي ACK مي‌دهد** — چون ACK
//                در I2C همان پايين بودنِ SDA در پالسِ نهم است.
//        GPIO0 و GPIO4 = بالاکشِ خودِ برد (بوت و وقفه‌ي لمس).
//                در آزمايشِ پايين‌کش «بالا» مي‌خوانند و آدم فکر
//                مي‌کند ماژول آن‌جاست.
//
//      و نشانه‌اي که همان‌جا لوش داد: 0x68 و 0x69 **هم‌زمان**
//      جواب دادند. يک MPU6050 فقط يک آدرس دارد. وقتي جوابي
//      از نظرِ فيزيکي ناممکن است ، به ابزار شک کن نه به دنيا.
//
//   پس دو محافظ اضافه شد:
//     ۱ پايه‌هايي که برد خودش گرفته ، اصلا امتحان نمي‌شوند
//     ۲ ACK کافي نيست — رجيسترِ WHO_AM_I خوانده مي‌شود
static const uint8_t PINFIND[]  = { 1, 3, 6, 7, 9, 12, 13, 43, 44 };
static const int     PINFIND_N  = sizeof(PINFIND) / sizeof(PINFIND[0]);

// پايه‌هايي که روي خودِ برد به چيزي وصل‌اند — نتيجه‌شان بي‌معني است
static const uint8_t PINBUSY[]  = { 0, 4, 8 };
static const char   *PINBUSY_W[] = { "بوت", "وقفه‌ي لمس", "ولتاژِ باتري" };

// آيا واقعا يک MPU آن‌جاست؟  WHO_AM_I در رجيستر 0x75
inline bool mpuWhoAmI(TwoWire &bus, uint8_t addr) {
  bus.beginTransmission(addr);
  bus.write(0x75);
  if (bus.endTransmission(false) != 0) return false;
  if (bus.requestFrom(addr, (uint8_t)1) != 1) return false;
  uint8_t who = bus.read();
  return (who == 0x68 || who == 0x70 || who == 0x71 || who == 0x73 || who == 0x98);
}

inline void pinFinder() {
  Serial.println("\n╔══ پين‌ياب ══════════════════════════════════");
  Serial.println("║ ۱) کدام پايه‌ها بالاکشِ بيروني دارند؟");

  int hi = 0;
  for (int i = 0; i < PINFIND_N; i++) {
    pinMode(PINFIND[i], INPUT_PULLDOWN);
    delayMicroseconds(300);
    if (digitalRead(PINFIND[i])) {
      Serial.printf("║    GPIO%-2d  بالا  ←── سيمي به اين وصل است\n", PINFIND[i]);
      hi++;
    }
    pinMode(PINFIND[i], INPUT);
  }
  if (hi == 0)
    Serial.println("║    هيچ‌کدام — يعني ماژول برق ندارد يا سيم جا نرفته");
  Serial.println("║    پايه‌هايي که امتحان نشدند ، چون برد خودش گرفته:");
  for (int i = 0; i < (int)(sizeof(PINBUSY) / sizeof(PINBUSY[0])); i++)
    Serial.printf("║      GPIO%-2d  %s\n", PINBUSY[i], PINBUSY_W[i]);

  Serial.println("║");
  Serial.println("║ ۲) امتحانِ جفت‌ها — با خواندنِ WHO_AM_I ، نه فقط ACK …");

  //   ☠️ روي باسي که چيزي رويش نيست ، هر requestFrom يک ارورِ
  //      قرمزِ دو خطي چاپ مي‌کند. ۷۲ جفت × ۲ آدرس = ۱۴۴ ارور ،
  //      و نتيجه‌ي واقعي زيرشان گم مي‌شود. ابزارِ عيب‌يابي که
  //      خروجي‌اش خوانده نشود ، عيب‌يابي نيست.
  esp_log_level_t old = ESP_LOG_ERROR;
  esp_log_level_set("i2c.master", ESP_LOG_NONE);

  int found = 0;
  Wire1.end();
  for (int a = 0; a < PINFIND_N; a++) {
    for (int b = 0; b < PINFIND_N; b++) {
      if (a == b) continue;
      uint8_t sda = PINFIND[a], scl = PINFIND[b];
      Wire1.begin(sda, scl, 100000);
      delay(2);

      // ☠️ محافظِ «خطِ گير کرده»: اگر بيش از دو آدرس جواب دهند ،
      //    SDA پايين گير کرده و همه‌ي ACKها قلابي‌اند.
      int acks = 0;
      for (uint8_t ad = 0x08; ad < 0x78; ad += 7) {
        Wire1.beginTransmission(ad);
        if (Wire1.endTransmission() == 0) acks++;
      }
      if (acks > 2) { Wire1.end(); continue; }

      for (uint8_t ad = 0x68; ad <= 0x69; ad++) {
        if (mpuWhoAmI(Wire1, ad)) {
          Serial.printf("║  ✅ MPU واقعي!  0x%02X  با  SDA=GPIO%d · SCL=GPIO%d\n",
                        ad, sda, scl);
          found++;
        }
      }
      Wire1.end();
    }
  }

  Serial.println("║");
  if (found) {
    Serial.println("║ اين دو عدد را در moochi_sense.h بگذار:");
    Serial.println("║    #define I2C2_SDA / I2C2_SCL");
  } else {
    //   ☠️ محدوديتي که بايد صريح گفته شود ، وگرنه اين ابزار
    //      خودش گمراه‌کننده مي‌شود:
    //
    //      پين‌ياب **GPIO10 و GPIO11 را تست نمي‌کند** ، چون باسِ
    //      اصلي است و نمي‌شود وسطِ کار از زيرِ پنج چيپِ ديگر
    //      کشيدش بيرون. ولي کانکتورِ کناريِ J6 **دقيقا همان باس
    //      است**. يعني اگر سيم روي J6 باشد ، اين ابزار هيچ‌وقت
    //      پيدايش نمي‌کند — و «پيدا نشد»ش بي‌معني است.
    //
    //      آزمايشِ معتبر براي J6 همان اسکنِ لحظه‌ي روشن شدن است.
    Serial.println("║ ❌ روي پايه‌هاي آزاد چيزي نيست.");
    Serial.println("║");
    Serial.println("║ ⚠️ ولي اين ابزار GPIO10/11 را تست نمي‌کند (باسِ اصلي).");
    Serial.println("║    کانکتورِ کناريِ J6 هم **همان باس** است. اگر سيم آن‌جاست ،");
    Serial.println("║    به اسکنِ بالا نگاه کن ، نه به اين.");
    Serial.println("║");
    Serial.println("║ کانکتورهاي چهارپايه‌ي کناري (از شماتيکِ نسخه ۲):");
    Serial.println("║    J6 :  ۱=SCL  ۲=SDA  ۳=3V3  ۴=GND   ← I2C");
    Serial.println("║    J8 :  ۱=RXD  ۲=TXD  ۳=3V3  ۴=GND   ← سريال ، بي‌فايده");
    Serial.println("║    پايه‌ي ۳ و ۴ هر دو يکي است — پس چراغِ ماژول");
    Serial.println("║    روي J8 هم روشن مي‌شود ولي داده نمي‌آيد.");
  }
  esp_log_level_set("i2c.master", old);
  Wire1.begin(I2C2_SDA, I2C2_SCL, 400000);
  Serial.println("╚═════════════════════════════════════════════\n");
}

// چاپ همه‌ي آدرس‌هاي زنده — براي عيب‌يابي
inline void i2cScanReport() {
  Serial.println("   باس اصلي (SDA=11 · SCL=10):");
  for (uint8_t a = 1; a < 127; a++)
    if (i2cPing(Wire, a)) Serial.printf("      0x%02X\n", a);
#if USE_SECOND_BUS
  Serial.printf("   باس دوم (SDA=%d · SCL=%d):\n", I2C2_SDA, I2C2_SCL);
  bool any = false;
  for (uint8_t a = 1; a < 127; a++)
    if (i2cPing(Wire1, a)) { Serial.printf("      0x%02X\n", a); any = true; }
  if (!any) {
    Serial.println("      — خالي —");
    // ☠️ «خالي» دو معنيِ کاملا متفاوت دارد و بايد از هم جدا شوند:
    //
    //      خطوط بالا (HIGH)  → باس سالم است ، فقط کسي جواب نمي‌دهد
    //                          ⇒ تغذيه‌ي ماژول ، يا پينِ اشتباه
    //      خطي پايين (LOW)   → باس **گير کرده**
    //                          ⇒ اتصالِ کوتاه ، يا ماژولِ بي‌برق که
    //                             خطِ داده را به زمين مي‌کشد
    //
    //    بدونِ اين تفکيک ، آدم ساعت‌ها دنبالِ اشتباهِ عوضي مي‌گردد.
    //   ☠️ نسخه‌ي اولِ همين آزمايش را با INPUT_PULLUP نوشتم و
    //      **بي‌فايده** بود: با بالاکشِ داخلي پايه در هر حالتي
    //      بالا مي‌خواند ، حتي وقتي هيچ سيمي وصل نيست. يعني
    //      جوابش هميشه «بالا» بود و چيزي نمي‌گفت.
    //
    //      آزمايشِ بامعني عکسِ آن است: **پايين‌کشِ داخلي**.
    //        پايه‌ي آزاد          → پايين مي‌خواند
    //        پايه‌ي وصل به ماژولِ
    //        برق‌دار (بالاکشِ ۱۰k) → بالا مي‌خواند
    //      چون ۱۰ کيلو بر پايين‌کشِ داخليِ ~۴۵ کيلو غلبه مي‌کند.
    pinMode(I2C2_SDA, INPUT_PULLDOWN);
    pinMode(I2C2_SCL, INPUT_PULLDOWN);
    delayMicroseconds(200);
    int sda = digitalRead(I2C2_SDA), scl = digitalRead(I2C2_SCL);
    Serial.printf("      بالاکشِ بيروني: SDA=%s  SCL=%s\n",
                  sda ? "هست" : "نيست", scl ? "هست" : "نيست");
    if (sda && scl)
      Serial.println("      ⇒ ماژول برق دارد و به همين دو پايه وصل است ،");
    else if (sda || scl)
      Serial.println("      ⇒ فقط يک خط وصل است — آن يکي سيم جا نرفته");
    else
      Serial.println("      ⇒ هيچ ماژولِ برق‌داري روي اين دو پايه نيست.");
    Serial.println("      ⇒ کليدِ w را بزن تا برد خودش بگويد سيم کجاست.");
    pinMode(I2C2_SDA, INPUT);
    pinMode(I2C2_SCL, INPUT);
    Wire1.begin(I2C2_SDA, I2C2_SCL, 400000);   // باس را برگردان
  }
#else
  Serial.println("   باس دوم: خاموش");
#endif
}
