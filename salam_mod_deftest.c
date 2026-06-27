#include "salam_mod_deftest.h"

static void _Salam_main_greet_str_str_i32(const char* name, const char* greeting, int32_t times);

static void _Salam_main_greet_str_str_i32(const char* name, const char* greeting, int32_t times) {
    for (int64_t __rep1 = 0, __repn1 = (int64_t)(times); __rep1 < __repn1; __rep1++) {
        printf("%s %s\n", greeting, name);
    }
}

int main(int argc, char** argv) {
    salam_set_args(argc, argv);
    _Salam_main_greet_str_str_i32("Sara", "Hello", 1);
    _Salam_main_greet_str_str_i32("Ali", "Hi", 1);
    _Salam_main_greet_str_str_i32("Max", "Hey", 3);
    return 0;
}
