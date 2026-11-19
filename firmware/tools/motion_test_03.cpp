#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ============================================
//  PET ROBOT — test 03 : خواندن حرکت (شتاب‌سنج + ژیروسکوپ)
// ============================================
// اسکنر قبلی (test 02) در firmware\tools\i2c_scanner_v2.cpp نگه داشته شد.
// PlatformIO فقط پوشه‌ي src را می‌سازد ، پس آن فایل مزاحم نیست.
//
// چیپ ما MPU6500 است (WHO_AM_I = 0x70) ، نه MPU6050 — ۳ مهر ۱۴۰۵.
// کد برای هر دو کار می‌کند؛ فقط رجیستر 0x1D مال MPU6500 است.
//
// رجیسترها — از نقشه‌ي رجیستر TDK InvenSense برای MPU-6500:
//   0x6B PWR_MGMT_1    بیت۷ = ریست ، CLKSEL=1 = بهترین ساعت
//   0x1A CONFIG        فیلتر ژیرو (3 = حدود 41Hz)
//   0x1B GYRO_CONFIG   بیت‌های ۴:۳ → 1 = ±500 درجه/ثانیه (65.5 واحد = 1°/s)
//   0x1C ACCEL_CONFIG  بیت‌های ۴:۳ → 1 = ±4g (8192 واحد = 1g)
//   0x1D ACCEL_CONFIG2 فیلتر شتاب (3 = حدود 45Hz) — فقط MPU6500
//   0x3B..0x48         ۱۴ بایت: شتاب XYZ ، دما ، چرخش XYZ (بایت بالا اول)
//
// چرا ±4g و ±500 و نه کمترین: وقتی موچی را تکان بدهند ، با ±2g
// عدد به سقف می‌خورد و «چقدر محکم تکانش دادند» گم می‌شود.
// ============================================

// پایه‌های موقت بردبورد (همان اسکنر). روی برد نهایی 21/22.
const int PIN_SDA = 33;
const int PIN_SCL = 32;
const uint8_t MPU_ADDR = 0x68;

const float ACC_PER_G   = 8192.0f;   // ±4g
const float GYRO_PER_DPS = 65.5f;    // ±500 °/s

uint8_t chipId = 0;
float gyroBias[3] = {0, 0, 0};       // خطای ثابت ژیرو ، در حالت سکون اندازه می‌گیریم

// ---------- ابزار ----------
bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

int readReg(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)1) != 1) return -1;
  return Wire.read();
}

// ۱۴ بایت خام را می‌خواند. اگر ارتباط قطع شد false.
bool readRaw(int16_t acc[3], int16_t gyr[3], int16_t &temp) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14) != 14) return false;
  uint8_t b[14];
  for (int i = 0; i < 14; i++) b[i] = Wire.read();
  for (int i = 0; i < 3; i++) acc[i] = (int16_t)((b[i * 2] << 8) | b[i * 2 + 1]);
  temp = (int16_t)((b[6] << 8) | b[7]);
  for (int i = 0; i < 3; i++) gyr[i] = (int16_t)((b[8 + i * 2] << 8) | b[9 + i * 2]);
  return true;
}

// ---------- راه‌اندازی چیپ ----------
bool setupMpu() {
  int who = readReg(0x75);
  if (who < 0) {
    Serial.println("☠️ چیپ جواب نمی‌دهد. سیم‌ها را چک کن (USB کشیده!).");
    return false;
  }
  chipId = (uint8_t)who;
  Serial.print("شناسه‌ي چیپ: 0x");
  Serial.print(chipId, HEX);
  if (chipId == 0x70)      Serial.println("  = MPU6500");
  else if (chipId == 0x68) Serial.println("  = MPU6050");
  else if (chipId == 0x71) Serial.println("  = MPU9250");
  else                     Serial.println("  = ناآشنا (ادامه می‌دهیم ولی مطمئن نیستیم)");

  writeReg(0x6B, 0x80);          // ریست کامل
  delay(100);
  writeReg(0x6B, 0x01);          // بیدار + ساعت خودکار
  delay(50);
  writeReg(0x1A, 0x03);          // فیلتر ژیرو ~41Hz (لرزش ریز حذف)
  writeReg(0x1B, 0x08);          // ±500 °/s
  writeReg(0x1C, 0x08);          // ±4g
  if (chipId != 0x68) writeReg(0x1D, 0x03);   // فیلتر شتاب — MPU6050 این رجیستر را ندارد
  delay(50);

  // بررسی: آیا تنظیم‌ها واقعا نشستند؟ (حدس نمی‌زنیم ، می‌خوانیم)
  int g = readReg(0x1B), a = readReg(0x1C), p = readReg(0x6B);
  Serial.print("تنظیم‌ها خوانده شد:  PWR=0x"); Serial.print(p, HEX);
  Serial.print("  GYRO=0x"); Serial.print(g, HEX);
  Serial.print("  ACCEL=0x"); Serial.println(a, HEX);
  if ((g & 0x18) != 0x08 || (a & 0x18) != 0x08 || (p & 0x40)) {
    Serial.println("⚠️ تنظیم‌ها درست ننشستند — عددها قابل اعتماد نیستند.");
    return false;
  }
  return true;
}

// ---------- صفر کردن ژیرو ----------
// ژیرو حتی ساکن هم چند درجه در ثانیه خطا دارد. ماژول را ۲ ثانیه
// روی میز ساکن نگه دار؛ میانگین را خطای ثابت حساب می‌کنیم.
void calibrateGyro() {
  while (true) {
    Serial.println();
    Serial.println("⏳ ماژول را ۲ ثانیه تکان نده ... (صفر کردن ژیرو)");
    delay(500);
    const int N = 300;
    float sum[3] = {0, 0, 0};
    int16_t mn[3] = {32767, 32767, 32767}, mx[3] = {-32768, -32768, -32768};
    int ok = 0;
    for (int i = 0; i < N; i++) {
      int16_t acc[3], gyr[3], t;
      if (readRaw(acc, gyr, t)) {
        ok++;
        for (int k = 0; k < 3; k++) {
          sum[k] += gyr[k];
          if (gyr[k] < mn[k]) mn[k] = gyr[k];
          if (gyr[k] > mx[k]) mx[k] = gyr[k];
        }
      }
      delay(5);
    }
    if (ok < N / 2) {
      Serial.println("☠️ بیشتر خواندن‌ها شکست خورد. سیم‌کشی را چک کن.");
      delay(3000);
      continue;
    }
    // اگر وسط کار تکان خورده باشد ، فاصله‌ي کمترین و بیشترین زیاد می‌شود
    int spread = 0;
    for (int k = 0; k < 3; k++) spread = max(spread, (int)(mx[k] - mn[k]));
    if (spread > 400) {   // حدود ۶ درجه/ثانیه
      Serial.println("  تکان خورد! دوباره ...");
      continue;
    }
    for (int k = 0; k < 3; k++) gyroBias[k] = sum[k] / ok;
    Serial.print("✅ صفر شد. خطای ثابت (°/s):  x=");
    Serial.print(gyroBias[0] / GYRO_PER_DPS, 2);
    Serial.print("  y="); Serial.print(gyroBias[1] / GYRO_PER_DPS, 2);
    Serial.print("  z="); Serial.println(gyroBias[2] / GYRO_PER_DPS, 2);
    return;
  }
}

// ---------- چاپ ----------
void printSigned(float v, int digits) {
  if (v >= 0) Serial.print("+");
  Serial.print(v, digits);
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  PET - test 03 : حرکت");
  Serial.println("==============================");

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

  while (!setupMpu()) {
    Serial.println("۳ ثانیه دیگر دوباره امتحان می‌کنم ...");
    delay(3000);
  }
  calibrateGyro();
  Serial.println();
  Serial.println("راهنما: ماژول را صاف روی میز بگذار (نوشته‌ها رو به بالا).");
  Serial.println("  شتاب z باید نزدیک +1.00 باشد (جاذبه). چرخش‌ها نزدیک صفر.");
  Serial.println("  بعد کجش کن ، بلندش کن ، تکانش بده و ببین عددها چه می‌کنند.");
  Serial.println();
}

void loop() {
  static int fails = 0;
  int16_t acc[3], gyr[3], t;
  if (!readRaw(acc, gyr, t)) {
    fails++;
    Serial.println("☠️ خواندن شکست خورد (سیم شل؟)");
    if (fails >= 5) {             // چند بار پشت سر هم → دوباره راه بینداز
      fails = 0;
      setupMpu();
    }
    delay(500);
    return;
  }
  fails = 0;

  float ax = acc[0] / ACC_PER_G, ay = acc[1] / ACC_PER_G, az = acc[2] / ACC_PER_G;
  float gx = (gyr[0] - gyroBias[0]) / GYRO_PER_DPS;
  float gy = (gyr[1] - gyroBias[1]) / GYRO_PER_DPS;
  float gz = (gyr[2] - gyroBias[2]) / GYRO_PER_DPS;

  float aMag = sqrtf(ax * ax + ay * ay + az * az);        // ساکن ≈ 1.00
  float gMag = sqrtf(gx * gx + gy * gy + gz * gz);
  float pitch = atan2f(ax, sqrtf(ay * ay + az * az)) * 57.2958f;   // کج جلو/عقب
  float roll  = atan2f(ay, az) * 57.2958f;                          // کج چپ/راست

  // دما فقط برای اطمینان از سلامت چیپ (فرمول‌ها فرق دارند)
  float tempC = (chipId == 0x68) ? (t / 340.0f + 36.53f) : (t / 333.87f + 21.0f);

  // ---- تشخیص ساده (آستانه‌ها اولیه‌اند؛ بعد از دیدن عددهای واقعی تنظیم می‌شوند) ----
  const char *state;
  if (aMag < 0.35f)                          state = "سقوط آزاد!";
  else if (gMag > 250 || fabsf(aMag - 1) > 0.8f) state = "تکان شدید";
  else if (gMag > 20 || fabsf(aMag - 1) > 0.12f) state = "حرکت";
  else if (az < -0.7f)                        state = "وارونه";
  else if (fabsf(ax) > 0.7f || fabsf(ay) > 0.7f) state = "به پهلو";
  else                                        state = "آرام";

  Serial.print("شتاب(g) x="); printSigned(ax, 2);
  Serial.print(" y="); printSigned(ay, 2);
  Serial.print(" z="); printSigned(az, 2);
  Serial.print(" کل="); Serial.print(aMag, 2);
  Serial.print(" | چرخش(°/s) x="); printSigned(gx, 0);
  Serial.print(" y="); printSigned(gy, 0);
  Serial.print(" z="); printSigned(gz, 0);
  Serial.print(" | کج: جلو="); printSigned(pitch, 0);
  Serial.print("° پهلو="); printSigned(roll, 0);
  Serial.print("° | دما="); Serial.print(tempC, 1);
  Serial.print(" | ");
  Serial.println(state);

  delay(250);
}
