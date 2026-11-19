#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
using std::min; using std::max;
typedef uint8_t byte;
extern uint32_t g_millis;
inline uint32_t millis(){ return g_millis; }
inline long random(long n){ return n>0? rand()%n : 0; }
inline long random(long a,long b){ return b>a? a + rand()%(b-a) : a; }
template<class T,class L,class H> inline T constrain(T x,L l,H h){ return x<l?l:(x>h?h:x); }
#ifndef PI
#define PI 3.14159265358979f
#endif
#define PROGMEM
struct SerialStub {
  template<class... A> void printf(const char* f, A... a){ }
  template<class T> void print(T){ } template<class T> void println(T){ } void println(){}
};
static SerialStub Serial;
