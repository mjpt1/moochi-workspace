#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

// ============================================
//  موچی — test 05 : نمایشگر گرد ۱٫۲۸ اینچ GC9A01
// ============================================
// ⚠️ این فایل «آماده برای بعد» است و هنوز در src نیست.
//    وقتی نمایشگر رسید:
//      ۱. این فایل را جای firmware\src\main.cpp بگذار
//      ۲. در platformio.ini به lib_deps اضافه کن:
//           adafruit/Adafruit GC9A01A
//
// ماژول (از عکس‌های فروشنده، ۷ مهر ۱۴۰۵):
//   پشتش: 1.28"TFT VER1.0 · 240*240 · IC:GC9A01
//   ۷ پایه‌ي لحیم‌شده: VCC GND SCL SDA DC CS RST — پایه‌ي BL ندارد
//   (نور پشت همیشه روشن است)
//
// کتابخانه: Adafruit_GC9A01A. سازنده‌ي (CS, DC, RST) و begin(freq)
// از فایل Adafruit_GC9A01A.h خود کتابخانه چک شد.
//
// سیم‌کشی — همان پایه‌های نمایشگر ۱٫۶۹ ، فقط بدون BLK:
//   VCC → ریل +  (3V3 — هرگز V5)
//   GND → ریل −
//   SCL → G14    (ساعت SPI — ربطی به SCL شتاب‌سنج ندارد!)
//   SDA → G13    (داده SPI — ربطی به SDA شتاب‌سنج ندارد!)
//   RST → G25
//   DC  → G26
//   CS  → G27
// ============================================

const int PIN_SCK  = 14;
const int PIN_MOSI = 13;
const int PIN_RST  = 25;
const int PIN_DC   = 26;
const int PIN_CS   = 27;

Adafruit_GC9A01A tft(PIN_CS, PIN_DC, PIN_RST);   // SPI سخت‌افزاری

void colorPage(uint16_t bg, uint16_t fg, const char *name) {
  tft.fillScreen(bg);
  tft.setTextColor(fg);
  tft.setTextSize(3);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, (240 - h) / 2);
  tft.print(name);
  Serial.print("  رنگ: ");
  Serial.println(name);
}

// صفحه گرد است: یک دایره‌ي کامل لبه به لبه + علامت بالا/پایین/چپ/راست.
// اگر دایره کج یا بریده بود ، جابه‌جایی یا چرخش اشتباه است.
void edgeTest() {
  tft.fillScreen(GC9A01A_BLACK);
  tft.drawCircle(120, 120, 119, GC9A01A_WHITE);
  tft.drawCircle(120, 120, 117, GC9A01A_WHITE);
  tft.fillCircle(120, 12, 8, GC9A01A_RED);      // بالا قرمز
  tft.fillCircle(228, 120, 8, GC9A01A_GREEN);   // راست سبز
  tft.fillCircle(120, 228, 8, GC9A01A_BLUE);    // پایین آبی
  tft.fillCircle(12, 120, 8, GC9A01A_YELLOW);   // چپ زرد
  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.setCursor(84, 104);
  tft.print("MOOCHI");
  tft.setCursor(66, 128);
  tft.print("240 x 240");
  Serial.println("  دایره و چهار علامت");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  موچی - test 05 : نمایشگر گرد GC9A01");
  Serial.println("==============================");

  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
  tft.begin(40000000);        // 40MHz
  tft.setRotation(0);
  Serial.println("فرمان راه‌اندازی فرستاده شد (ESP32 نمی‌فهمد نمایشگر واقعا جواب داده یا نه)");
}

void loop() {
  colorPage(GC9A01A_RED,   GC9A01A_WHITE, "RED");    delay(1500);
  colorPage(GC9A01A_GREEN, GC9A01A_BLACK, "GREEN");  delay(1500);
  colorPage(GC9A01A_BLUE,  GC9A01A_WHITE, "BLUE");   delay(1500);
  colorPage(GC9A01A_WHITE, GC9A01A_BLACK, "WHITE");  delay(1500);
  edgeTest();                                         delay(4000);
}
