#pragma once
struct TftBase{ template<class...A>void setRotation(A...){} template<class...A>void setSPISpeed(A...){}
 template<class...A>void fillScreen(A...){} template<class...A>void fillRect(A...){} template<class...A>void drawRect(A...){}
 template<class...A>void fillCircle(A...){} template<class...A>void fillTriangle(A...){} template<class...A>void getTextBounds(A...){}
 template<class...A>void print(A...){} template<class...A>void setCursor(A...){} template<class...A>void setTextColor(A...){} template<class...A>void setTextSize(A...){}
 void startWrite(){} void endWrite(){} int ax=0,ay=0,aw=1,ah=0; uint16_t *scr=nullptr; long rows=0;
 void setAddrWindow(int x,int y,int w,int h){ax=x;ay=y;aw=w;ah=h;}
 void writePixels(uint16_t*b,uint32_t n,bool=true,bool=false){ if(!scr)return; for(uint32_t i=0;i<n;i++){int X=ax+i%aw,Y=ay+i/aw; if(Y<280) scr[Y*240+X]=b[i];} rows+=n/aw;}
 uint16_t color565(int,int,int){return 0;} };
