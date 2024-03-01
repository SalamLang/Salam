#include <stdio.h>

int main(void)
{
    long long len = 3000000;
    long long shift = 13;
    long long x = 1;
    long long checksum = 0;
    long long step;
    long long letter;
    long long shifted;
    for (step = 0; step < len; step++) {
        x = (x * 1103515245 + 12345) % 2147483648LL;
        letter = x % 26;
        shifted = (letter + shift) % 26;
        checksum = (checksum + shifted) % 1000000007;
    }
    printf("%lld\n", checksum);
    return 0;
}
