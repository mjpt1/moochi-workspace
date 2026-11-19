#pragma once
enum {I2S_MODE_STD}; enum {I2S_DATA_BIT_WIDTH_16BIT}; enum {I2S_SLOT_MODE_STEREO};
struct I2SClass{ void setPins(int,int,int){} bool begin(int,int,int,int){return true;} size_t write(const uint8_t*,size_t n){return n;} };
