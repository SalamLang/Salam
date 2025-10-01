#include <utility/memory/memory_move/memory_move.h>

void memory_move(void* dest, const void* src, size_t size) {
    DEBUG_ME;
    if (!dest || !src) {
        log_fatal("memory_move received NULL pointers\n");
    }
    memmove(dest, src, size);
}
