#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================
//  موچی — test 06 : نمایشگر ۱٫۳ اینچ مربعی (۲۴۰×۲۴۰)
// ============================================
// تست ۱٫۶۹ (خراب از آب درآمد) در firmware\tools\st7789_169_test04.cpp
// نمایشگر گرد GC9A01 (در راه) در firmware\tools\gc9a01_test05.cpp
//
// چرا الان ۱٫۳؟ — وضوحش ۲۴۰×۲۴۰ است ، دقیقا مثل نمایشگر گرد GC9A01.
// هر کاری روی این بکنیم (مخصوصا انتقال موتور چهره) مستقیم به گرد منتقل می‌شود.
//
// ماژول (از نوشته‌ي پشتش ، ۳ مهر ۱۴۰۵):
//   YY-IPS130-V4.0 · 240x240 · ST7789 · SPI
//   ۷ پایه: GND VCC SCL SDA RES DC BLK — ⚠️ پایه‌ي CS ندارد
//   (CS داخل ماژول به زمین وصل است ، پس همیشه «انتخاب‌شده» است)
//   نور پشتش ۶ مهر با ۳ سیم تست شد و روشن شد ✅
//
// ⚠️ حالت SPI (حدس نیست — دو احتمال مستند):
//   ماژول‌های بدون CS با mode 0 معمولی جواب نمی‌دهند.
//   راهنمای رایج: اول SPI_MODE2 ، اگر تصویر نیامد SPI_MODE3.
//   فقط خط SPI_MODE_TO_TRY را عوض کن.
//
// سیم‌کشی — همان سیم‌های ۱٫۶۹ ، فقط CS ندارد:
//   VCC → ریل +  (3V3 — هرگز V5)
//   GND → ریل −
//   BLK → ریل +  (نور همیشه روشن)
//   SCL → G14    (ساعت SPI — ربطی به SCL شتاب‌سنج ندارد!)
//   SDA → G13    (داده SPI — ربطی به SDA شتاب‌سنج ندارد!)
//   RES → G25
//   DC  → G26
//   (سیمی که قبلا به CS می‌رفت — G27 — آزاد می‌ماند)
// ============================================

const uint8_t SPI_MODE_TO_TRY = SPI_MODE2;   // اگر تصویر نیامد: SPI_MODE3

const int PIN_SCK  = 14;
const int PIN_MOSI = 13;
const int PIN_RST  = 25;
const int PIN_DC   = 26;

Adafruit_ST7789 tft(-1, PIN_DC, PIN_RST);    // -1 = پایه‌ي CS ندارد

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

// قاب + چهار گوشه‌ي رنگی: اگر جابه‌جایی غلط باشد ، یک لبه بریده می‌شود
// یا نوار آشغال کنارش می‌آید.
void edgeTest() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, 240, 240, ST77XX_WHITE);
  tft.drawRect(2, 2, 236, 236, ST77XX_WHITE);
  tft.fillRect(0, 0, 20, 20, ST77XX_RED);          // بالا-چپ قرمز
  tft.fillRect(220, 0, 20, 20, ST77XX_GREEN);      // بالا-راست سبز
  tft.fillRect(0, 220, 20, 20, ST77XX_BLUE);       // پایین-چپ آبی
  tft.fillRect(220, 220, 20, 20, ST77XX_YELLOW);   // پایین-راست زرد
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(84, 100);
  tft.print("MOOCHI");
  tft.setCursor(66, 124);
  tft.print("240 x 240");
  Serial.println("  قاب و چهار گوشه");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  موچی - test 06 : نمایشگر ۱٫۳ اینچ");
  Serial.println("==============================");
  Serial.print("حالت SPI: ");
  Serial.println(SPI_MODE_TO_TRY == SPI_MODE2 ? "MODE2" :
                 SPI_MODE_TO_TRY == SPI_MODE3 ? "MODE3" : "دیگر");

  SPI.begin(PIN_SCK, -1, PIN_MOSI, -1);
  tft.init(240, 240, SPI_MODE_TO_TRY);
  tft.setSPISpeed(40000000);
  tft.setRotation(0);
  Serial.println("فرمان راه‌اندازی فرستاده شد (ESP32 نمی‌فهمد نمایشگر واقعا جواب داده یا نه)");
}

void loop() {
  colorPage(ST77XX_RED,   ST77XX_WHITE, "RED");    delay(1500);
  colorPage(ST77XX_GREEN, ST77XX_BLACK, "GREEN");  delay(1500);
  colorPage(ST77XX_BLUE,  ST77XX_WHITE, "BLUE");   delay(1500);
  colorPage(ST77XX_WHITE, ST77XX_BLACK, "WHITE");  delay(1500);
  edgeTest();                                       delay(4000);
}
