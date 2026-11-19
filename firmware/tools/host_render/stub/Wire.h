#pragma once
struct TwoWire{template<class...A>void begin(A...){} void setClock(long){} void beginTransmission(int){} int endTransmission(bool=true){return 0;} int requestFrom(int,int){return 0;} int read(){return 0;} void write(int){}};static TwoWire Wire;
