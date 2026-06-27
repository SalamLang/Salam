#include "salam_mod_cmp_iso.h"

static void _Salam_main_vfn_i32_i32(int32_t a, int32_t b);
static int32_t _Salam_main_ifn_i32_i32(int32_t a, int32_t b);

static void _Salam_main_vfn_i32_i32(int32_t a, int32_t b) {
    printf("%d\n", (int)((a + b)));
}

static int32_t _Salam_main_ifn_i32_i32(int32_t a, int32_t b) {
    return (a + b);
}

int main(int argc, char** argv) {
    salam_set_args(argc, argv);
    _Salam_main_vfn_i32_i32(1, 7);
    const int32_t x = _Salam_main_ifn_i32_i32(1, 7);
    printf("%d\n", (int)(x));
    return 0;
}
