#pragma once
#include <cstdlib>
inline void* heap_caps_malloc(size_t n,int){return malloc(n);} inline size_t heap_caps_get_largest_free_block(int){return 107000;}
