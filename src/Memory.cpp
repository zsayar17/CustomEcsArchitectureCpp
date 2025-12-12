#include "Memory.hpp"

void* aligned_alloc_16(size_t size) {
#ifdef _WIN32
    return _aligned_malloc(size, 16);
#else
    void* ptr;
    if (posix_memalign(&ptr, 16, size) != 0) return NULL;
    return ptr;
#endif
}

void aligned_free_16(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}
