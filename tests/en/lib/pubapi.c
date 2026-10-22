#include <stdio.h>
#include "pubapi.h"

int main(void)
{
    printf("%d\n", pubapi_Add(40, 2));
    printf("%s\n", pubapi_Greeting());
    return 0;
}
