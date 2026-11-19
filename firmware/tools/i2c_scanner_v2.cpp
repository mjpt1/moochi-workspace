#include <Arduino.h>
#include <Wire.h>
#include <esp_heap_caps.h>

// ============================================
//  PET ROBOT — test 02 : اسکنر I2C  (نسخه‌ي ۲)
// ============================================
// این کد روی باس I2C می‌گردد و آدرس هر ماژولی
// که جواب بدهد را چاپ می‌کند.
//
// انتظار ما:
//   0x29  →  VL53L0X  (فاصله‌سنج لیزری)  — فعلا وصل نیست
//   0x68  →  MPU6050  (ژیروسکوپ و شتاب‌سنج)
//
// ☠️ سه چیز که نسخه‌ي اول نداشت — هر سه از تجربه‌ي
//    برد Waveshare (۳ مهر ۱۴۰۵) آمده‌اند:
//
//  ۱. «جواب دادن» کافی نیست. اگر خط SDA پایین گیر کند،
//     هر آدرسی «بله» می‌گوید و اسکنر ده‌ها دستگاهِ قلابی
//     نشان می‌دهد. پس برای MPU6050 شناسه‌ي واقعیِ چیپ
//     (رجیستر WHO_AM_I) را هم می‌خوانیم.
//
//  ۲. قبل از اسکن ، وضع خود سیم‌ها را می‌سنجیم. خط گیرکرده
//     را جدا گزارش می‌کنیم تا با «چیزی وصل نیست» قاطی نشود.
//
//  ۳. حافظه‌ي آزاد برد را یک بار اندازه می‌گیریم. ESP32
//     ما PSRAM ندارد و برای نمایشگر باید بدانیم بزرگ‌ترین
//     تکه‌ي پیوسته‌ي آزاد چقدر است — عدد واقعی ، نه حدس.
// ============================================

// ☠️ پایه‌های موقت — فقط برای آزمایش روی بردبورد (۳ مهر ۱۴۰۵)
//
//    ESP32 ما ۳۸ پایه دارد و روی بردبورد فقط یک طرفش ردیف
//    سوراخ آزاد دارد: طرفِ راست (V5 · 3V3 · GND · G13..G32).
//    G21 و G22 طرفِ چپ‌اند و زیر بدنه‌ي برد گیر افتاده‌اند.
//
//    ESP32 می‌تواند I2C را روی تقریبا هر پایه‌ای راه بیندازد ،
//    پس دو پایه‌ي کنار هم از طرفِ راست را برداشتیم:
//      G33 = SDA   ·   G32 = SCL
//    چرا این دو: هیچ‌کدام پایه‌ي بوت (strapping) نیستند ، و
//    برخلاف G34/G35/SN/SP «فقط ورودی» هم نیستند — I2C باید
//    بتواند خط را پایین بکشد.
//
//    روی برد نهایی (برد سوراخ‌دار) دوباره به 21/22 برمی‌گردیم.
const int PIN_SDA = 33;
const int PIN_SCL = 32;

// ---------- ۱) آیا سیم‌ها سالم‌اند؟ ----------
// در حالت عادی هر دو خط I2C باید «بالا» باشند.
// اگر یکی «پایین» ماند ، یعنی چیزی آن را به زمین کشیده.
void checkLines() {
  pinMode(PIN_SDA, INPUT_PULLUP);
  pinMode(PIN_SCL, INPUT_PULLUP);
  delay(2);
  bool sda = digitalRead(PIN_SDA);
  bool scl = digitalRead(PIN_SCL);

  Serial.print("وضع خطوط:  SDA=");
  Serial.print(sda ? "بالا" : "پایین !!");
  Serial.print("   SCL=");
  Serial.println(scl ? "بالا" : "پایین !!");

  if (!sda || !scl) {
    Serial.println("  ☠️ یک خط به زمین چسبیده. نتیجه‌ي اسکن قابل اعتماد نیست.");
    Serial.println("     USB را بکش و سیم‌کشی را دوباره نگاه کن:");
    Serial.println("     GND ماژول نباید روی SDA یا SCL افتاده باشد.");
  }
}

// ---------- ۲) آیا واقعا MPU6050 است؟ ----------
// رجیستر 0x75 (WHO_AM_I) شناسه‌ي چیپ را برمی‌گرداند.
// MPU6050 → 0x68 · MPU6500 → 0x70 · MPU9250 → 0x71
int readWhoAmI(byte addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x75);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(addr, (byte)1) != 1) return -1;
  return Wire.read();
}

void scanBus() {
  byte found = 0;

  Serial.println();
  Serial.println("در حال اسکن باس I2C ...");
  Serial.println("------------------------");

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte err = Wire.endTransmission();

    if (err == 0) {
      found++;
      Serial.print("  [OK] device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);

      // شناسایی ماژول‌های خودمان
      if (addr == 0x29) Serial.print("   <-- VL53L0X");
      if (addr == 0x68 || addr == 0x69) {
        Serial.print(addr == 0x68 ? "   <-- MPU6050" : "   <-- MPU6050 (AD0=HIGH)");
        int who = readWhoAmI(addr);
        if (who < 0) {
          Serial.print("  ⚠️ جواب داد ولی شناسه‌اش خوانده نشد");
        } else {
          Serial.print("  شناسه=0x");
          Serial.print(who, HEX);
          if (who == 0x68)      Serial.print("  ✅ MPU6050 واقعی و سالم");
          else if (who == 0x70) Serial.print("  ✅ MPU6500 (خانواده‌ي همان)");
          else if (who == 0x71) Serial.print("  ✅ MPU9250 (خانواده‌ي همان)");
          else                  Serial.print("  ❓ شناسه‌ي ناآشنا");
        }
      }
      Serial.println();
    }
  }

  Serial.println("------------------------");
  if (found == 0) {
    Serial.println("  هيچ دستگاهي پيدا نشد!");
    Serial.print("  چک کن: VCC->3V3 · GND->GND · SDA->G");
    Serial.print(PIN_SDA);
    Serial.print(" · SCL->G");
    Serial.println(PIN_SCL);
  } else if (found > 8) {
    // ☠️ چند ده دستگاه یعنی خط SDA گیر کرده ، نه اینکه واقعا این همه ماژول داریم
    Serial.print("  ☠️ ");
    Serial.print(found);
    Serial.println(" دستگاه؟! این غیرممکن است — خط SDA گیر کرده.");
  } else {
    Serial.print("  تعداد کل: ");
    Serial.println(found);
  }
  Serial.println();
}

// ---------- ۳) حافظه ----------
void reportMemory() {
  size_t freeAll  = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  size_t biggest  = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  Serial.println("حافظه‌ي برد (برای تصمیم درباره‌ي نمایشگر):");
  Serial.print("  آزاد کل:              ");
  Serial.print(freeAll / 1024);
  Serial.println(" KB");
  Serial.print("  بزرگ‌ترین تکه‌ي پیوسته: ");
  Serial.print(biggest / 1024);
  Serial.println(" KB");
  Serial.println("  (یک تصویر کامل ۲۴۰×۲۸۰ حدود ۱۳۱ KB لازم دارد)");
}

void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("========================");
  Serial.println("  PET - I2C scanner v2");
  Serial.println("========================");
  Serial.print("SDA = GPIO");
  Serial.println(PIN_SDA);
  Serial.print("SCL = GPIO");
  Serial.println(PIN_SCL);
  Serial.println();

  reportMemory();
  Serial.println();
  checkLines();

  Wire.begin(PIN_SDA, PIN_SCL);
}

void loop() {
  scanBus();
  delay(4000);   // هر ۴ ثانیه دوباره اسکن کن
}
