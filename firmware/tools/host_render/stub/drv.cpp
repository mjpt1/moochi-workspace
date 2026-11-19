#include "Arduino.h"
uint32_t g_millis=0;
#include "../../../src/main.cpp"
int main(){setup(); for(int i=0;i<200;i++){g_millis+=30; loop();} printf("ok\n");}
