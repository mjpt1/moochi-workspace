#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"

// =====================================================
//   درايور نمايشگر  ESP32-S3-Touch-LCD-1.85C
//   ST77916 روي باس QSPI
// =====================================================
//  اين درايور مستقيم با esp_lcd کار مي‌کند — همان لايه‌اي که
//  خود Waveshare استفاده مي‌کند. تفاوتش با تلاش قبلي ما:
//
//  قبلا از Arduino_GFX استفاده مي‌کرديم که پين CS را خودش
//  دستي کنترل مي‌کرد و زمان‌بندي‌اش با اين پنل جور نبود.
//  نتيجه: خروجي غيرقابل‌تکرار و خط‌خطي.
//
//  esp_lcd همه‌ي زمان‌بندي را به سخت‌افزار SPI مي‌سپارد.
// =====================================================

#define LCD_W 360
#define LCD_H 360

// ---- پين‌ها (از جدول ويکي Waveshare) ----
#define LCD_PIN_CS    21
#define LCD_PIN_SCK   40
#define LCD_PIN_D0    46
#define LCD_PIN_D1    45
#define LCD_PIN_D2    42
#define LCD_PIN_D3    41
#define LCD_PIN_BL     5

// ---- چيپ گسترش‌دهنده TCA9554 ----
// ريست نمايشگر روي EXIO2 است، نه روي GPIO مستقيم
#define TCA_ADDR       0x20
#define TCA_REG_OUT    0x01
#define TCA_REG_CONF   0x03

// ---- فرمان‌هاي QSPI ----
// در حالت QSPI هر فرمان بايد در قالب ۳۲ بيتي بسته‌بندي شود:
//   بايت اول = نوع عمليات ، بايت دوم = خود فرمان
#define QSPI_WRITE_CMD    0x02UL      // نوشتن فرمان و پارامتر
#define QSPI_WRITE_COLOR  0x32UL      // نوشتن داده‌ي رنگ

static esp_lcd_panel_io_handle_t lcd_io = nullptr;

// نوع پنل: ۱ = پيش‌فرض ، ۲ = نوع دوم
// موقع راه‌اندازي از خود پنل پرسيده مي‌شود
static int lcd_variant = 1;

// ⚠️ اين سمافور قلب درست کار کردن نمايشگر است.
//
//    esp_lcd_panel_io_tx_color داده را فقط در صف مي‌گذارد و
//    فورا برمي‌گردد — منتظر نمي‌ماند واقعا فرستاده شود.
//    ولي tx_param (که براي CASET و RASET استفاده مي‌شود) همزمان است.
//
//    بدون اين سمافور، وقتي نوار اول هنوز در حال ارسال است،
//    ما پنجره‌ي آدرس نوار دوم را مي‌فرستيم و وسط کار همه‌چيز
//    به هم مي‌ريزد. نتيجه: نوارهاي عمودي و خروجي غيرقابل‌تکرار.
static SemaphoreHandle_t lcd_done = nullptr;

static bool IRAM_ATTR lcdTransDone(esp_lcd_panel_io_handle_t io,
                                   esp_lcd_panel_io_event_data_t *e,
                                   void *ctx) {
  BaseType_t hp = pdFALSE;
  xSemaphoreGiveFromISR(lcd_done, &hp);
  return hp == pdTRUE;
}

// =====================================================
//  جدول راه‌اندازي — از مخزن رسمي Waveshare
//  ESP32-S3-Touch-LCD-1.85C / esp_lcd_st77916.c
//  (بخش کامنت‌نشده‌ي vendor_specific_init_default)
//
//  قالب: { فرمان ، تعداد بايت داده ، بايت‌ها... }
// =====================================================
static const uint8_t LCD_INIT[] = {
    0xF0, 1, 0x28,
    0xF2, 1, 0x28,
    0x7C, 1, 0xD1,
    0x83, 1, 0xE0,
    0x84, 1, 0x61,
    0xF2, 1, 0x82,
    0xF0, 1, 0x00,
    0xF0, 1, 0x01,
    0xF1, 1, 0x01,
    0xB0, 1, 0x49,
    0xB1, 1, 0x4A,
    0xB2, 1, 0x1F,
    0xB4, 1, 0x46,
    0xB5, 1, 0x34,
    0xB6, 1, 0xD5,
    0xB7, 1, 0x30,
    0xB8, 1, 0x04,
    0xBA, 1, 0x00,
    0xBB, 1, 0x08,
    0xBC, 1, 0x08,
    0xBD, 1, 0x00,
    0xC0, 1, 0x80,
    0xC1, 1, 0x10,
    0xC2, 1, 0x37,
    0xC3, 1, 0x80,
    0xC4, 1, 0x10,
    0xC5, 1, 0x37,
    0xC6, 1, 0xA9,
    0xC7, 1, 0x41,
    0xC8, 1, 0x01,
    0xC9, 1, 0xA9,
    0xCA, 1, 0x41,
    0xCB, 1, 0x01,
    0xD0, 1, 0x91,
    0xD1, 1, 0x68,
    0xD2, 1, 0x68,
    0xF5, 2, 0x00, 0xA5,
    0xF1, 1, 0x10,
    0xF0, 1, 0x00,
    0xF0, 1, 0x02,
    0xE0, 14, 0x70, 0x09, 0x12, 0x0C, 0x0B, 0x27, 0x38, 0x54, 0x4E, 0x19, 0x15, 0x15, 0x2C, 0x2F,
    0xE1, 14, 0x70, 0x08, 0x11, 0x0C, 0x0B, 0x27, 0x38, 0x43, 0x4C, 0x18, 0x14, 0x14, 0x2B, 0x2D,
    0xF0, 1, 0x10,
    0xF3, 1, 0x10,
    0xE0, 1, 0x08,
    0xE1, 1, 0x00,
    0xE2, 1, 0x0B,
    0xE3, 1, 0x00,
    0xE4, 1, 0xE0,
    0xE5, 1, 0x06,
    0xE6, 1, 0x21,
    0xE7, 1, 0x00,
    0xE8, 1, 0x05,
    0xE9, 1, 0x82,
    0xEA, 1, 0xDF,
    0xEB, 1, 0x89,
    0xEC, 1, 0x20,
    0xED, 1, 0x14,
    0xEE, 1, 0xFF,
    0xEF, 1, 0x00,
    0xF8, 1, 0xFF,
    0xF9, 1, 0x00,
    0xFA, 1, 0x00,
    0xFB, 1, 0x30,
    0xFC, 1, 0x00,
    0xFD, 1, 0x00,
    0xFE, 1, 0x00,
    0xFF, 1, 0x00,
    0x60, 1, 0x42,
    0x61, 1, 0xE0,
    0x62, 1, 0x40,
    0x63, 1, 0x40,
    0x64, 1, 0x02,
    0x65, 1, 0x00,
    0x66, 1, 0x40,
    0x67, 1, 0x03,
    0x68, 1, 0x00,
    0x69, 1, 0x00,
    0x6A, 1, 0x00,
    0x6B, 1, 0x00,
    0x70, 1, 0x42,
    0x71, 1, 0xE0,
    0x72, 1, 0x40,
    0x73, 1, 0x40,
    0x74, 1, 0x02,
    0x75, 1, 0x00,
    0x76, 1, 0x40,
    0x77, 1, 0x03,
    0x78, 1, 0x00,
    0x79, 1, 0x00,
    0x7A, 1, 0x00,
    0x7B, 1, 0x00,
    0x80, 1, 0x38,  0x81, 1, 0x00,  0x82, 1, 0x04,  0x83, 1, 0x02,
    0x84, 1, 0xDC,  0x85, 1, 0x00,  0x86, 1, 0x00,  0x87, 1, 0x00,
    0x88, 1, 0x38,  0x89, 1, 0x00,  0x8A, 1, 0x06,  0x8B, 1, 0x02,
    0x8C, 1, 0xDE,  0x8D, 1, 0x00,  0x8E, 1, 0x00,  0x8F, 1, 0x00,
    0x90, 1, 0x38,  0x91, 1, 0x00,  0x92, 1, 0x08,  0x93, 1, 0x02,
    0x94, 1, 0xE0,  0x95, 1, 0x00,  0x96, 1, 0x00,  0x97, 1, 0x00,
    0x98, 1, 0x38,  0x99, 1, 0x00,  0x9A, 1, 0x0A,  0x9B, 1, 0x02,
    0x9C, 1, 0xE2,  0x9D, 1, 0x00,  0x9E, 1, 0x00,  0x9F, 1, 0x00,
    0xA0, 1, 0x38,  0xA1, 1, 0x00,  0xA2, 1, 0x03,  0xA3, 1, 0x02,
    0xA4, 1, 0xDB,  0xA5, 1, 0x00,  0xA6, 1, 0x00,  0xA7, 1, 0x00,
    0xA8, 1, 0x38,  0xA9, 1, 0x00,  0xAA, 1, 0x05,  0xAB, 1, 0x02,
    0xAC, 1, 0xDD,  0xAD, 1, 0x00,  0xAE, 1, 0x00,  0xAF, 1, 0x00,
    0xB0, 1, 0x38,  0xB1, 1, 0x00,  0xB2, 1, 0x07,  0xB3, 1, 0x02,
    0xB4, 1, 0xDF,  0xB5, 1, 0x00,  0xB6, 1, 0x00,  0xB7, 1, 0x00,
    0xB8, 1, 0x38,  0xB9, 1, 0x00,  0xBA, 1, 0x09,  0xBB, 1, 0x02,
    0xBC, 1, 0xE1,  0xBD, 1, 0x00,  0xBE, 1, 0x00,  0xBF, 1, 0x00,
    0xC0, 1, 0x22,  0xC1, 1, 0xAA,  0xC2, 1, 0x65,  0xC3, 1, 0x74,
    0xC4, 1, 0x47,  0xC5, 1, 0x56,  0xC6, 1, 0x00,  0xC7, 1, 0x88,
    0xC8, 1, 0x99,  0xC9, 1, 0x33,
    0xD0, 1, 0x11,  0xD1, 1, 0xAA,  0xD2, 1, 0x65,  0xD3, 1, 0x74,
    0xD4, 1, 0x47,  0xD5, 1, 0x56,  0xD6, 1, 0x00,  0xD7, 1, 0x88,
    0xD8, 1, 0x99,  0xD9, 1, 0x33,
    0xF3, 1, 0x01,
    0xF0, 1, 0x00,
    0x00  // پايان
};

// =====================================================
//  جدول «ب» — براي نوع دوم پنل
//  از Display_ST77916.cpp ، آرايه vendor_specific_init_new
//
//  کد Waveshare موقع راه‌اندازي شناسه‌ي پنل را مي‌خواند و
//  بسته به جواب، يکي از اين دو جدول را مي‌فرستد.
// =====================================================
static const uint8_t LCD_INIT_B[] = {
    0xF0, 1, 0x28,
    0xF2, 1, 0x28,
    0x73, 1, 0xF0,        // ← اين فرمان فقط در اين جدول هست
    0x7C, 1, 0xD1,
    0x83, 1, 0xE0,
    0x84, 1, 0x61,
    0xF2, 1, 0x82,
    0xF0, 1, 0x00,
    0xF0, 1, 0x01,
    0xF1, 1, 0x01,
    0xB0, 1, 0x56,
    0xB1, 1, 0x4D,
    0xB2, 1, 0x24,
    0xB4, 1, 0x87,
    0xB5, 1, 0x44,
    0xB6, 1, 0x8B,
    0xB7, 1, 0x40,
    0xB8, 1, 0x86,
    0xBA, 1, 0x00,
    0xBB, 1, 0x08,
    0xBC, 1, 0x08,
    0xBD, 1, 0x00,
    0xC0, 1, 0x80,
    0xC1, 1, 0x10,
    0xC2, 1, 0x37,
    0xC3, 1, 0x80,
    0xC4, 1, 0x10,
    0xC5, 1, 0x37,
    0xC6, 1, 0xA9,
    0xC7, 1, 0x41,
    0xC8, 1, 0x01,
    0xC9, 1, 0xA9,
    0xCA, 1, 0x41,
    0xCB, 1, 0x01,
    0xD0, 1, 0x91,
    0xD1, 1, 0x68,
    0xD2, 1, 0x68,
    0xF5, 2, 0x00, 0xA5,
    0xDD, 1, 0x4F,
    0xDE, 1, 0x4F,
    0xF1, 1, 0x10,
    0xF0, 1, 0x00,
    0xF0, 1, 0x02,
    0xE0, 14, 0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34,
    0xE1, 14, 0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33,
    0xF0, 1, 0x10,
    0xF3, 1, 0x10,
    0xE0, 1, 0x07,  0xE1, 1, 0x00,  0xE2, 1, 0x00,  0xE3, 1, 0x00,
    0xE4, 1, 0xE0,  0xE5, 1, 0x06,  0xE6, 1, 0x21,  0xE7, 1, 0x01,
    0xE8, 1, 0x05,  0xE9, 1, 0x02,  0xEA, 1, 0xDA,  0xEB, 1, 0x00,
    0xEC, 1, 0x00,  0xED, 1, 0x0F,  0xEE, 1, 0x00,  0xEF, 1, 0x00,
    0xF8, 1, 0x00,  0xF9, 1, 0x00,  0xFA, 1, 0x00,  0xFB, 1, 0x00,
    0xFC, 1, 0x00,  0xFD, 1, 0x00,  0xFE, 1, 0x00,  0xFF, 1, 0x00,
    0x60, 1, 0x40,  0x61, 1, 0x04,  0x62, 1, 0x00,  0x63, 1, 0x42,
    0x64, 1, 0xD9,  0x65, 1, 0x00,  0x66, 1, 0x00,  0x67, 1, 0x00,
    0x68, 1, 0x00,  0x69, 1, 0x00,  0x6A, 1, 0x00,  0x6B, 1, 0x00,
    0x70, 1, 0x40,  0x71, 1, 0x03,  0x72, 1, 0x00,  0x73, 1, 0x42,
    0x74, 1, 0xD8,  0x75, 1, 0x00,  0x76, 1, 0x00,  0x77, 1, 0x00,
    0x78, 1, 0x00,  0x79, 1, 0x00,  0x7A, 1, 0x00,  0x7B, 1, 0x00,
    0x80, 1, 0x48,  0x81, 1, 0x00,  0x82, 1, 0x06,  0x83, 1, 0x02,
    0x84, 1, 0xD6,  0x85, 1, 0x04,  0x86, 1, 0x00,  0x87, 1, 0x00,
    0x88, 1, 0x48,  0x89, 1, 0x00,  0x8A, 1, 0x08,  0x8B, 1, 0x02,
    0x8C, 1, 0xD8,  0x8D, 1, 0x04,  0x8E, 1, 0x00,  0x8F, 1, 0x00,
    0x90, 1, 0x48,  0x91, 1, 0x00,  0x92, 1, 0x0A,  0x93, 1, 0x02,
    0x94, 1, 0xDA,  0x95, 1, 0x04,  0x96, 1, 0x00,  0x97, 1, 0x00,
    0x98, 1, 0x48,  0x99, 1, 0x00,  0x9A, 1, 0x0C,  0x9B, 1, 0x02,
    0x9C, 1, 0xDC,  0x9D, 1, 0x04,  0x9E, 1, 0x00,  0x9F, 1, 0x00,
    0xA0, 1, 0x48,  0xA1, 1, 0x00,  0xA2, 1, 0x05,  0xA3, 1, 0x02,
    0xA4, 1, 0xD5,  0xA5, 1, 0x04,  0xA6, 1, 0x00,  0xA7, 1, 0x00,
    0xA8, 1, 0x48,  0xA9, 1, 0x00,  0xAA, 1, 0x07,  0xAB, 1, 0x02,
    0xAC, 1, 0xD7,  0xAD, 1, 0x04,  0xAE, 1, 0x00,  0xAF, 1, 0x00,
    0xB0, 1, 0x48,  0xB1, 1, 0x00,  0xB2, 1, 0x09,  0xB3, 1, 0x02,
    0xB4, 1, 0xD9,  0xB5, 1, 0x04,  0xB6, 1, 0x00,  0xB7, 1, 0x00,
    0xB8, 1, 0x48,  0xB9, 1, 0x00,  0xBA, 1, 0x0B,  0xBB, 1, 0x02,
    0xBC, 1, 0xDB,  0xBD, 1, 0x04,  0xBE, 1, 0x00,  0xBF, 1, 0x00,
    0xC0, 1, 0x10,  0xC1, 1, 0x47,  0xC2, 1, 0x56,  0xC3, 1, 0x65,
    0xC4, 1, 0x74,  0xC5, 1, 0x88,  0xC6, 1, 0x99,  0xC7, 1, 0x01,
    0xC8, 1, 0xBB,  0xC9, 1, 0xAA,
    0xD0, 1, 0x10,  0xD1, 1, 0x47,  0xD2, 1, 0x56,  0xD3, 1, 0x65,
    0xD4, 1, 0x74,  0xD5, 1, 0x88,  0xD6, 1, 0x99,  0xD7, 1, 0x01,
    0xD8, 1, 0xBB,  0xD9, 1, 0xAA,
    0xF3, 1, 0x01,
    0xF0, 1, 0x00,
    0x00  // پايان
};

// =====================================================
//  کمک‌کارها
// =====================================================
static bool tcaWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

// فرستادن يک فرمان به پنل
static void lcdCmd(uint8_t cmd, const uint8_t *data = nullptr, size_t len = 0) {
  uint32_t packed = (QSPI_WRITE_CMD << 24) | ((uint32_t)cmd << 8);
  esp_lcd_panel_io_tx_param(lcd_io, packed, data, len);
}

// نور صفحه — در core 3.x شماره‌ي پين مي‌دهيم، نه شماره‌ي کانال
static inline void lcdBacklight(uint16_t level) {   // ۰ تا ۱۰۲۳
  ledcWrite(LCD_PIN_BL, level > 1023 ? 1023 : level);
}

// =====================================================
//  راه‌اندازي
// =====================================================
static bool lcdInit(uint32_t clock_hz = 40000000) {
  // ۱) نور را خاموش نگه دار تا تصوير آماده شود
  ledcAttach(LCD_PIN_BL, 20000, 10);
  lcdBacklight(0);

  // ۲) ريست سخت‌افزاري پنل از طريق TCA9554
  //    فقط EXIO2 — همان چيزي که کد Waveshare مي‌زند.
  //    قبلا سه پين را با هم پايين مي‌برديم که لازم نبود.
  if (!tcaWrite(TCA_REG_CONF, 0x00)) {
    Serial.println("[lcd] TCA9554 جواب نداد");
    return false;
  }
  tcaWrite(TCA_REG_OUT, 0xFF);  delay(20);
  tcaWrite(TCA_REG_OUT, 0xFD);  delay(10);     // فقط بيت ۱ (EXIO2) پايين
  tcaWrite(TCA_REG_OUT, 0xFF);  delay(50);

  // ۳) باس SPI در حالت چهارخطه
  spi_bus_config_t buscfg = {};
  buscfg.data0_io_num = LCD_PIN_D0;
  buscfg.data1_io_num = LCD_PIN_D1;
  buscfg.sclk_io_num  = LCD_PIN_SCK;
  buscfg.data2_io_num = LCD_PIN_D2;
  buscfg.data3_io_num = LCD_PIN_D3;
  // کل صفحه در يک تراکنش: ۳۶۰ × ۳۶۰ × ۲ = ۲۵۹۲۰۰ بايت
  buscfg.max_transfer_sz = LCD_W * LCD_H * 2 + 64;
  buscfg.flags = SPICOMMON_BUSFLAG_MASTER;        // بدون QUAD — مثل کد Waveshare

  if (spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
    Serial.println("[lcd] spi_bus_initialize شکست خورد");
    return false;
  }

  // ۴) لايه‌ي ارتباط با پنل
  esp_lcd_panel_io_spi_config_t io_cfg = {};
  io_cfg.cs_gpio_num = LCD_PIN_CS;
  io_cfg.dc_gpio_num = -1;              // QSPI پين DC ندارد
  io_cfg.spi_mode = 0;
  // ⚠️ اول با سرعت پايين شروع مي‌کنيم.
  //    خواندن شناسه‌ي پنل در سرعت بالا جواب نمي‌دهد —
  //    کد Waveshare هم همين کار را مي‌کند: ۵ مگاهرتز براي خواندن،
  //    بعد دوباره با سرعت کامل مي‌سازد.
  io_cfg.pclk_hz = 5 * 1000 * 1000;
  io_cfg.trans_queue_depth = 10;
  io_cfg.lcd_cmd_bits = 32;             // همان چيزي که Waveshare مي‌گويد
  io_cfg.lcd_param_bits = 8;
  io_cfg.flags.quad_mode = true;
  io_cfg.on_color_trans_done = lcdTransDone;   // خبر بده کي تمام شد

  lcd_done = xSemaphoreCreateBinary();
  if (!lcd_done) {
    Serial.println("[lcd] سمافور ساخته نشد");
    return false;
  }

  if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,
                               &io_cfg, &lcd_io) != ESP_OK) {
    Serial.println("[lcd] esp_lcd_new_panel_io_spi شکست خورد");
    return false;
  }

  // ۵) 🔑 شناسايي نوع پنل
  //
  //    اين مهم‌ترين کشف کل ماجراست. دو نوع پنل ST77916 وجود دارد
  //    که جدول راه‌اندازي متفاوتي مي‌خواهند. کد Waveshare رجيستر
  //    0x04 را مي‌خواند و بر اساس جوابش تصميم مي‌گيرد.
  //
  //    ما سه روز جدول نوع اول را به پنل مي‌فرستاديم.
  {
    uint8_t id[4] = {0, 0, 0, 0};
    // خواندن با کد عمليات 0x0B (نه 0x02 که براي نوشتن است)
    uint32_t readCmd = (0x0BUL << 24) | (0x04UL << 8);
    esp_err_t r = esp_lcd_panel_io_rx_param(lcd_io, readCmd, id, 4);

    if (r == ESP_OK) {
      Serial.printf("[lcd] شناسه پنل (0x04): %02X %02X %02X %02X\n",
                    id[0], id[1], id[2], id[3]);
      if (id[0] == 0x00 && id[1] == 0x02 && id[2] == 0x7F && id[3] == 0x7F) {
        lcd_variant = 2;
      } else if (id[0] == 0x00 && id[1] == 0x7F && id[2] == 0x7F && id[3] == 0x7F) {
        lcd_variant = 1;
      } else {
        Serial.println("[lcd] ⚠️ شناسه ناشناخته — جدول پيش‌فرض را امتحان مي‌کنم");
      }
    } else {
      Serial.printf("[lcd] ⚠️ خواندن شناسه شکست خورد (خطا %d)\n", r);
    }
  }

  // ۵.۵) حالا که شناسه را خوانديم، ارتباط را با سرعت کامل بساز
  esp_lcd_panel_io_del(lcd_io);
  io_cfg.pclk_hz = clock_hz;
  if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,
                               &io_cfg, &lcd_io) != ESP_OK) {
    Serial.println("[lcd] ساخت مجدد با سرعت کامل شکست خورد");
    return false;
  }
  Serial.printf("[lcd] سرعت باس: %d MHz\n", clock_hz / 1000000);

  // ۶) ريست نرم‌افزاري
  lcdCmd(0x01);
  delay(120);

  // ۶) عمق رنگ و ترتيب رنگ
  uint8_t colmod = 0x55;    // RGB565 شانزده بيتي
  uint8_t madctl = 0x00;    // RGB . اگر قرمز و آبي قاطي شد → 0x08
  lcdCmd(0x3A, &colmod, 1);
  lcdCmd(0x36, &madctl, 1);

  // ۷) جدول سازنده — کدام؟ از خود پنل مي‌پرسيم.
  const uint8_t *p = (lcd_variant == 2) ? LCD_INIT_B : LCD_INIT;
  Serial.printf("[lcd] جدول %s استفاده مي‌شود\n",
                (lcd_variant == 2) ? "ب (نوع دوم)" : "الف (پيش‌فرض)");
  while (*p) {
    uint8_t cmd = *p++;
    uint8_t len = *p++;
    lcdCmd(cmd, p, len);
    p += len;
  }

  // ۸) روشن کردن
  //
  //    وارونگي رنگ: اين پنل حتما INVON مي‌خواهد.
  //    با INVOFF همه‌ي رنگ‌ها دقيقا مکمل مي‌شوند —
  //    مشکي سفيد ، قرمز فيروزه‌اي ، سبز بنفش ، آبي زرد.
  lcdCmd(0x21);            // INVON — با وارونگي
  lcdCmd(0x11);            // SLPOUT
  delay(120);
  lcdCmd(0x29);            // DISPON
  delay(20);

  Serial.println("[lcd] راه‌اندازي تمام شد");
  return true;
}

// =====================================================
//  فرستادن يک ناحيه از تصوير به پنل
//  buf بايد RGB565 باشد و (x2-x1+1)*(y2-y1+1) پيکسل داشته باشد
// =====================================================
// ⚠️ درس گران‌قيمت:
//    هر بسته‌اي که مي‌فرستيم بايد کوچک‌تر از max_transfer_sz باشد.
//    يک صفحه‌ي کامل ۲۵۹,۲۰۰ بايت است — خيلي بزرگ‌تر از حد مجاز.
//    اگر يکجا بفرستي، بخشي از داده گم مي‌شود و صفحه خط‌خطي مي‌شود.
//    پس تصوير را در نوارهاي افقي مي‌فرستيم.
// حالا max_transfer_sz به اندازه‌ي کل صفحه است، پس تقسيم نمي‌کنيم.
// اگر روزي لازم شد تقسيم کنيم، اين عدد را کم کن.
#define LCD_BAND_ROWS 360     // يعني يک تراکنش براي کل صفحه

static void lcdBlit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                    const uint16_t *buf) {
  uint16_t w = x2 - x1 + 1;
  uint32_t colorCmd = (QSPI_WRITE_COLOR << 24) | (0x2CUL << 8);

  for (uint16_t y = y1; y <= y2; y += LCD_BAND_ROWS) {
    uint16_t yEnd = y + LCD_BAND_ROWS - 1;
    if (yEnd > y2) yEnd = y2;
    uint16_t rows = yEnd - y + 1;

    uint8_t caset[4] = { (uint8_t)(x1 >> 8), (uint8_t)x1,
                         (uint8_t)(x2 >> 8), (uint8_t)x2 };
    uint8_t raset[4] = { (uint8_t)(y >> 8), (uint8_t)y,
                         (uint8_t)(yEnd >> 8), (uint8_t)yEnd };
    lcdCmd(0x2A, caset, 4);
    lcdCmd(0x2B, raset, 4);

    const uint16_t *src = buf + (size_t)(y - y1) * w;
    esp_lcd_panel_io_tx_color(lcd_io, colorCmd, src, (size_t)w * rows * 2);

    // ⚠️ حياتي: صبر کن تا اين نوار واقعا فرستاده شود.
    //    بدون اين، CASET نوار بعدي وسط ارسال اين نوار مي‌رسد
    //    و تصوير به هم مي‌ريزد.
    xSemaphoreTake(lcd_done, pdMS_TO_TICKS(1000));
  }
}
