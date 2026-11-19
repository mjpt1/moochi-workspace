#pragma once
struct Preferences{bool begin(const char*,bool=false){return true;} void end(){} size_t getBytes(const char*,void*,size_t){return 0;} size_t putBytes(const char*,const void*,size_t){return 0;} uint8_t getUChar(const char*,uint8_t d){return d;} size_t putUChar(const char*,uint8_t){return 1;}};
