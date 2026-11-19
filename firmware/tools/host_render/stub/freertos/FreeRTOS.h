#pragma once
#include <deque>
#include <vector>
#include <cstring>
#include <cstdint>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(x) (x)
#define pdTRUE 1
struct portMUX_TYPE{int x;};
#define portMUX_INITIALIZER_UNLOCKED {0}
#define portENTER_CRITICAL(m)
#define portEXIT_CRITICAL(m)
struct QStub{ size_t sz; std::deque<std::vector<char>> q; };
typedef QStub* QueueHandle_t;
extern uint32_t g_millis;
inline QueueHandle_t xQueueCreate(int,size_t sz){auto q=new QStub; q->sz=sz; return q;}
inline int xQueueSend(QueueHandle_t q,const void*p,int){ std::vector<char> v(q->sz); memcpy(v.data(),p,q->sz); q->q.push_back(v); return 1;}
inline int xQueueReceive(QueueHandle_t q,void*p,int to){ if(q->q.empty()){ g_millis+=to; return 0;} memcpy(p,q->q.front().data(),q->sz); q->q.pop_front(); return 1;}
inline TickType_t xTaskGetTickCount(){return 0;}
inline void vTaskDelay(int){} inline void vTaskDelayUntil(TickType_t*,int){}
inline int xTaskCreatePinnedToCore(void(*)(void*),const char*,int,void*,int,void*,int){return 1;}
