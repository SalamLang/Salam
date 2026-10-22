#include <stdio.h>
#include "mathlib.h"

int main(void)
{
    printf("%d\n", salam_add(40, 2));
    printf("%d\n", salam_add(1, 1));
    printf("%d\n", salam_calls());
    fflush(stdout);
    salam_greet();
    return 0;
}
