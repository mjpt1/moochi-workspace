#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <functional>
using std::min; using std::max;
typedef uint8_t byte; typedef bool boolean;
extern uint32_t g_millis;
inline uint32_t millis(){ return g_millis; }
inline long random(long n){ return n>0? rand()%n : 0; }
inline long random(long a,long b){ return b>a? a + rand()%(b-a) : a; }
template<class T,class L,class H> inline T constrain(T x,L l,H h){ return x<l?l:(x>h?h:x); }
#ifndef PI
#define PI 3.14159265358979f
#endif
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define HEX 16
#define SPI_MODE0 0
#define SPI_MODE3 3
inline uint32_t esp_random(){return 1234;} inline void randomSeed(uint32_t s){srand(s);}
inline void delay(uint32_t ms){g_millis+=ms;}
#define MALLOC_CAP_8BIT 4
struct SerialStub { void begin(long){} int available(){return 0;} int read(){return -1;}
  template<class... A> void printf(const char* f, A... a){ }
  template<class T> void print(T){ } template<class T,class U> void print(T,U){ }
  template<class T> void println(T){ } template<class T,class U> void println(T,U){ } void println(){} };
static SerialStub Serial;
