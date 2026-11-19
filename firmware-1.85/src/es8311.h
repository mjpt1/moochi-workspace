#pragma once
// =====================================================
//   ES8311  —  کدکِ صداي بلندگو   (برد نسخه ۲ ، آدرس 0x18)
//
//   ☠️ اين فايل **ترجمه** است ، نه اختراع.
//
//      توالي رجيسترها مو‌به‌مو از درايورِ رسميِ اسپرسيف گرفته
//      شده:  espressif/esp-bsp → components/es8311/es8311.c
//      فقط لايه‌ي I2C از `i2c_master_write_to_device` به
//      `Wire` عوض شده ، چون بقيه‌ي پروژه با Wire کار مي‌کند و
//      باسِ I2C را قبلا موچي باز کرده (SDA=11 · SCL=10).
//
//      دليلش ساده است: جدولِ ضرايبِ کلاک و ترتيبِ روشن کردنِ
//      بخشِ آنالوگ چيزي نيست که بشود حدس زد. يک رجيسترِ اشتباه
//      يعني سکوتِ کامل و هيچ پيغامِ خطايي.
//
//   پينوتِ صدا (از شماتيکِ رسميِ نسخه ۲):
//      MCLK=2 · BCK=48 · LRCK=38 · DOUT=47 (به کدک) · PA=15
// =====================================================

#include <Arduino.h>
#include <Wire.h>

#define ES8311_ADDR 0x18

// ---------- رجيسترها ----------
// نامشان خودِ آدرس را دارد ، مثل درايورِ رسمي
#define ES8311_RESET_REG00       0x00
#define ES8311_CLK_MANAGER_REG01 0x01
#define ES8311_CLK_MANAGER_REG02 0x02
#define ES8311_CLK_MANAGER_REG03 0x03
#define ES8311_CLK_MANAGER_REG04 0x04
#define ES8311_CLK_MANAGER_REG05 0x05
#define ES8311_CLK_MANAGER_REG06 0x06
#define ES8311_CLK_MANAGER_REG07 0x07
#define ES8311_CLK_MANAGER_REG08 0x08
#define ES8311_SDPIN_REG09       0x09
#define ES8311_SDPOUT_REG0A      0x0A
#define ES8311_SYSTEM_REG0D      0x0D
#define ES8311_SYSTEM_REG0E      0x0E
#define ES8311_SYSTEM_REG12      0x12
#define ES8311_SYSTEM_REG13      0x13
#define ES8311_SYSTEM_REG14      0x14
#define ES8311_ADC_REG17         0x17
#define ES8311_ADC_REG1C         0x1C
#define ES8311_DAC_REG31         0x31
#define ES8311_DAC_REG32         0x32
#define ES8311_DAC_REG37         0x37

// ---------- I2C ----------
inline bool es8311Write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

inline bool es8311Read(uint8_t reg, uint8_t &val) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)ES8311_ADDR, (uint8_t)1) != 1) return false;
  val = Wire.read();
  return true;
}

// =====================================================
//   ضرايبِ کلاک
//
//   ☠️ اين اعداد را حدس نزن. جدولِ کاملِ درايورِ رسمي ۸۰ سطر
//      دارد ؛ ما فقط به نرخ‌هايي که واقعا استفاده مي‌کنيم نياز
//      داريم. سطرها **عينا** از coeff_div[] کپي شده‌اند.
//
//   moochi با MCLK = ۲۵۶ × نرخ کار مي‌کند (همان تنظيمِ I2S) ،
//   پس براي ۱۶ کيلوهرتز ⇒ MCLK = ۴٬۰۹۶٬۰۰۰
// =====================================================
struct Es8311Coeff {
  uint32_t mclk, rate;
  uint8_t  pre_div, pre_multi, adc_div, dac_div;
  uint8_t  fs_mode, lrck_h, lrck_l, bclk_div, adc_osr, dac_osr;
};

static const Es8311Coeff ES8311_COEFF[] = {
  // mclk      rate   pdiv  pmul adiv ddiv  fs   lrh   lrl  bdiv  aosr  dosr
  {  4096000, 16000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10 },
  {  6144000, 24000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10 },
  {  8192000, 32000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10 },
  { 11289600, 44100, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10 },
  { 12288000, 48000, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10 },
};

inline const Es8311Coeff *es8311Coeff(uint32_t mclk, uint32_t rate) {
  for (auto &c : ES8311_COEFF)
    if (c.mclk == mclk && c.rate == rate) return &c;
  return nullptr;
}

// =====================================================
//   راه‌اندازي
//     rate = نرخ نمونه‌برداري · MCLK فرض مي‌شود ۲۵۶ برابرِ آن
// =====================================================
inline bool es8311Init(uint32_t rate) {
  const uint32_t mclk = rate * 256;
  const Es8311Coeff *co = es8311Coeff(mclk, rate);
  if (!co) {
    Serial.printf("   ❌ ES8311: نرخ %lu با MCLK %lu در جدول نيست\n",
                  (unsigned long)rate, (unsigned long)mclk);
    return false;
  }

  // ---------- ريست و روشن کردن ----------
  if (!es8311Write(ES8311_RESET_REG00, 0x1F)) {
    Serial.println("   ❌ ES8311 روي باس نيست");
    return false;
  }
  delay(20);
  es8311Write(ES8311_RESET_REG00, 0x00);
  es8311Write(ES8311_RESET_REG00, 0x80);      // فرمانِ روشن شدن

  // ---------- کلاک ----------
  // 0x3F = همه‌ي کلاک‌ها روشن · بيتِ ۷ صفر = MCLK از پايه‌ي MCLK
  es8311Write(ES8311_CLK_MANAGER_REG01, 0x3F);

  uint8_t r;
  // رجيستر ۲ — تقسيم‌کننده و ضرب‌کننده‌ي پيش از کلاک
  if (es8311Read(ES8311_CLK_MANAGER_REG02, r)) {
    r &= 0x07;
    r |= (co->pre_div - 1) << 5;
    r |= co->pre_multi << 3;
    es8311Write(ES8311_CLK_MANAGER_REG02, r);
  }
  es8311Write(ES8311_CLK_MANAGER_REG03, (co->fs_mode << 6) | co->adc_osr);
  es8311Write(ES8311_CLK_MANAGER_REG04, co->dac_osr);
  es8311Write(ES8311_CLK_MANAGER_REG05,
              ((co->adc_div - 1) << 4) | (co->dac_div - 1));

  if (es8311Read(ES8311_CLK_MANAGER_REG06, r)) {
    r &= 0xE0;
    r |= (co->bclk_div < 19) ? (co->bclk_div - 1) : co->bclk_div;
    r &= ~(1 << 5);                           // SCLK معکوس نيست
    es8311Write(ES8311_CLK_MANAGER_REG06, r);
  }
  if (es8311Read(ES8311_CLK_MANAGER_REG07, r)) {
    r &= 0xC0;
    r |= co->lrck_h;
    es8311Write(ES8311_CLK_MANAGER_REG07, r);
  }
  es8311Write(ES8311_CLK_MANAGER_REG08, co->lrck_l);

  // ---------- فرمت ----------
  // برد مستر است و کدک برده. بيت ۶ رجيستر ۰ = ۰ يعني برده.
  if (es8311Read(ES8311_RESET_REG00, r)) {
    r &= 0xBF;
    es8311Write(ES8311_RESET_REG00, r);
  }
  es8311Write(ES8311_SDPIN_REG09,  3 << 2);   // ورودي ۱۶ بيت
  es8311Write(ES8311_SDPOUT_REG0A, 3 << 2);   // خروجي ۱۶ بيت

  // ---------- روشن کردنِ بخشِ آنالوگ ----------
  //   ☠️ اين پنج خط «پيش‌فرض نيستند». بدونشان همه‌ي کلاک‌ها
  //      درست‌اند و رجيسترها هم درست خوانده مي‌شوند ، ولي از
  //      بلندگو هيچ صدايي نمي‌آيد — چون DAC هنوز خاموش است.
  es8311Write(ES8311_SYSTEM_REG0D, 0x01);     // آنالوگ روشن
  es8311Write(ES8311_SYSTEM_REG0E, 0x02);     // PGA و مدولاتورِ ADC
  es8311Write(ES8311_SYSTEM_REG12, 0x00);     // DAC روشن
  es8311Write(ES8311_SYSTEM_REG13, 0x10);     // خروجي به درايورِ HP
  es8311Write(ES8311_ADC_REG1C,    0x6A);     // حذفِ آفستِ DC
  es8311Write(ES8311_DAC_REG37,    0x08);     // دور زدنِ اکولايزرِ DAC

  return true;
}

// ---------- بلندي ۰ تا ۱۰۰ ----------
inline void es8311Volume(int vol) {
  if (vol < 0)   vol = 0;
  if (vol > 100) vol = 100;
  es8311Write(ES8311_DAC_REG32, vol ? ((vol * 256) / 100) - 1 : 0);
}

// ---------- بي‌صدا ----------
inline void es8311Mute(bool m) {
  uint8_t r;
  if (!es8311Read(ES8311_DAC_REG31, r)) return;
  if (m) r |=  ((1 << 6) | (1 << 5));
  else   r &= ~((1 << 6) | (1 << 5));
  es8311Write(ES8311_DAC_REG31, r);
}
