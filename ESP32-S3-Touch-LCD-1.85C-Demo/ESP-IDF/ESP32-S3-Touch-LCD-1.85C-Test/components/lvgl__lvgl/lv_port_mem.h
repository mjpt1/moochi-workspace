#pragma once
#include <stdlib.h>
#include "esp_heap_caps.h"

/**
 * @brief LVGL 自定义 malloc，优先用 PSRAM
 */
static inline void *my_lvgl_malloc(size_t size)
{
    // 优先分配 PSRAM
    void *p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    // 如果 PSRAM 分配失败，退回内部 RAM
    if (!p) p = heap_caps_malloc(size, MALLOC_CAP_8BIT);
    return p;
}

/**
 * @brief LVGL 自定义 free
 */
static inline void my_lvgl_free(void *ptr)
{
    free(ptr);
}

#define LV_MEM_CUSTOM_ALLOC my_lvgl_malloc
#define LV_MEM_CUSTOM_FREE  my_lvgl_free