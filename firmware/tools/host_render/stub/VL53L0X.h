#pragma once
#include "Wire.h"
struct VL53L0X{ enum {RESULT_INTERRUPT_STATUS=0x13,RESULT_RANGE_STATUS=0x14,SYSTEM_INTERRUPT_CLEAR=0x0B};
 void setBus(TwoWire*){} void setTimeout(int){} bool init(){return true;} bool setMeasurementTimingBudget(int){return true;} void startContinuous(int=0){}
 uint8_t readReg(int){return 0;} uint16_t readReg16Bit(int){return 0;} void writeReg(int,int){} };
