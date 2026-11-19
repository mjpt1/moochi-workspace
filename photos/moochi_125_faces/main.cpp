// ============================================================
//  main.cpp — Moochi 125-Face Demo
//  برد: ESP32-S3 (کلاس سازگار با Waveshare ESP32-S3-Touch-LCD-1.85C)
//  چهره‌های ۱ تا ۱۲۵ را به‌ترتیب می‌چرخاند؛ لمس/دکمه = چهره‌ی بعدی.
//  اگر نمایشگر LCD در دسترس نباشد، رندر روی بافر انجام می‌شود و
//  شمارنده‌ی فریم روی Serial چاپ می‌شود (قابل‌کامپایل بدون LCD).
// ============================================================
#include <Arduino.h>
#include "moochi_faces_125.h"

// ---------- بافر ----------
static uint16_t *fb = nullptr;

#ifndef MOOCHI_NO_LCD
// اگر پروژه‌ی اصلی وصل است، این بخش را با moochi_lcd.h پروژه جایگزین کن
#define MOOCHI_NO_LCD 1
#endif

void setup() {
  Serial.begin(115200);
  delay(300);
  fb = (uint16_t *)malloc(MOOCHI_FB_W * MOOCHI_FB_H * sizeof(uint16_t));
  if (!fb) { Serial.println("[ERR] fb alloc failed"); while(1) delay(1000); }
  Serial.println("[MOOCHI] 125-face demo started");
}

static int   curFace = 1;
static uint32_t lastSwitch = 0;
static const uint32_t FACE_MS = 3000;   // هر چهره ۳ ثانیه

void loop() {
  uint32_t now = millis();
  if (now - lastSwitch >= FACE_MS) {
    lastSwitch = now;
    char name[80];
    moochiFaceName125(curFace, name, sizeof(name));
    Serial.printf("[FACE] %s\n", name);
    drawMoochiFace125(curFace, 0, 0, 100, fb);
    curFace++;
    if (curFace > 125) curFace = 1;
  }
  // قدم ریز مردمک — نگاه زنده
  static uint32_t lastGaze = 0;
  static int gx = 0, gy = 0, dir = 1;
  if (now - lastGaze > 700) {
    lastGaze = now;
    gx += 6 * dir;
    if (gx > 18 || gx < -18) { dir = -dir; gy = (gy == 0) ? 6 : 0; }
    drawMoochiFace125(curFace, gx, gy, 100, fb);
  }
  delay(16);
}
