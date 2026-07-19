#include "salam_mod_smoke.h"

int main(int argc, char** argv) {
    salam_set_args(argc, argv);
    SALAM_OUT_LIT("ok-native-gcc\n", 14);
    return 0;
}
