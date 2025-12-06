#include <utility/memory/memory_set/memory_set.h>

void memory_set(void* ptr, size_t len, int value) {
    DEBUG_ME;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = value;
    }
}
