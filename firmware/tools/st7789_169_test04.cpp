#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================
//  موچی — test 04 : اولین روشن شدن نمایشگر ۱٫۶۹ اینچ
// ============================================
// تست قبلی (حرکت) در firmware\tools\motion_test_03.cpp نگه داشته شد.
//
// نمایشگر: ZJY169S0800TG01 · ST7789V2 · 240×280 · SPI چهار سیمه
// (از نوشته‌ي پشت خود ماژول خوانده شد — ۳ مهر ۱۴۰۵)
//
// کتابخانه: Adafruit ST7789. در سورس خود کتابخانه (Adafruit_ST7789.cpp)
// چک شد که init(240, 280) پنجره را وسط حافظه‌ي ۲۴۰×۳۲۰ چیپ می‌گذارد
// (۲۰ خط فاصله از بالا) — همان چیزی که پنل ۱٫۶۹ لازم دارد.
//
// سیم‌کشی (فقط سمت راست ESP32 ، همه روی بردبورد):
//   نمایشگر SCL → G14    (ساعت SPI — ربطی به SCL شتاب‌سنج ندارد!)
//   نمایشگر SDA → G13    (داده SPI — ربطی به SDA شتاب‌سنج ندارد!)
//   نمایشگر RES → G25
//   نمایشگر DC  → G26
//   نمایشگر CS  → G27
//   نمایشگر BLK → ریل +  (نور همیشه روشن)
//   نمایشگر VCC → ریل +  (3V3 — هرگز V5)
//   نمایشگر GND → ریل −
// ============================================

const int PIN_SCK  = 14;
const int PIN_MOSI = 13;
const int PIN_RST  = 25;
const int PIN_DC   = 26;
const int PIN_CS   = 27;

// چرخش تصویر: 0 = عادی ، 2 = ۱۸۰ درجه (سر و ته).
// ۱۱ مهر: با 0 تصویر برعکس بود (چون روی بردبورد پایه‌ها بالای صفحه‌اند) → 2.
// برای ۲۴۰×۲۸۰ این دو حالت جابه‌جایی یکسان دارند (کتابخانه پنجره را وسط می‌گذارد).
const uint8_t SCREEN_ROTATION = 2;

Adafruit_ST7789 tft(PIN_CS, PIN_DC, PIN_RST);   // SPI سخت‌افزاری

// یک صفحه‌ي تک‌رنگ با اسم رنگ وسطش
void colorPage(uint16_t bg, uint16_t fg, const char *name) {
  tft.fillScreen(bg);
  tft.setTextColor(fg);
  tft.setTextSize(3);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, (280 - h) / 2);
  tft.print(name);
  Serial.print("  رنگ: ");
  Serial.println(name);
}

// قاب دور صفحه + گوشه‌ها: اگر وضوح یا جابه‌جایی اشتباه باشد ،
// یکی از لبه‌ها بریده می‌شود یا نوار آشغال کنارش می‌آید.
void edgeTest() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, 240, 280, ST77XX_WHITE);          // لبه‌ي بیرونی
  tft.drawRect(2, 2, 236, 276, ST77XX_WHITE);
  tft.fillRect(0, 0, 20, 20, ST77XX_RED);              // بالا-چپ قرمز
  tft.fillRect(220, 0, 20, 20, ST77XX_GREEN);          // بالا-راست سبز
  tft.fillRect(0, 260, 20, 20, ST77XX_BLUE);           // پایین-چپ آبی
  tft.fillRect(220, 260, 20, 20, ST77XX_YELLOW);       // پایین-راست زرد
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 110);
  tft.print("MOOCHI");
  tft.setCursor(60, 140);
  tft.print("240 x 280");
  Serial.println("  قاب و چهار گوشه");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  موچی - test 04 : نمایشگر");
  Serial.println("==============================");

  // SPI روی پایه‌های خودمان. MISO لازم نیست (-1) — نمایشگر فقط می‌گیرد.
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);

  tft.init(240, 280);          // حالت SPI پیش‌فرض (mode 0) — این پنل پایه‌ي CS دارد
  tft.setSPISpeed(40000000);   // 40MHz
  tft.setRotation(SCREEN_ROTATION);
  Serial.println("نمایشگر راه افتاد (اگر صفحه عوض نشد ، سیم‌کشی را چک کن)");
}

void loop() {
  colorPage(ST77XX_RED,   ST77XX_WHITE, "RED");    delay(1500);
  colorPage(ST77XX_GREEN, ST77XX_BLACK, "GREEN");  delay(1500);
  colorPage(ST77XX_BLUE,  ST77XX_WHITE, "BLUE");   delay(1500);
  colorPage(ST77XX_WHITE, ST77XX_BLACK, "WHITE");  delay(1500);
  edgeTest();                                       delay(4000);
}
