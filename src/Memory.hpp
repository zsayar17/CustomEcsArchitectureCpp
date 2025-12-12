#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <cstdlib>
#include <malloc.h>

void* aligned_alloc_16(size_t size);

void aligned_free_16(void* ptr);

#endif
