#include <stdio.h>

int main(void)
{
    int WIDTH = 400;
    int HEIGHT = 400;
    long long MAX_ITER = 100;
    long long sum = 0;
    int py;
    for (py = 0; py < HEIGHT; py++) {
        int px;
        for (px = 0; px < WIDTH; px++) {
            double x0 = -2.0 + 3.0 * (double)px / (double)WIDTH;
            double y0 = -1.5 + 3.0 * (double)py / (double)HEIGHT;
            double zx = 0.0;
            double zy = 0.0;
            long long iter = 0;
            while (zx * zx + zy * zy <= 4.0 && iter < MAX_ITER) {
                double xt = zx * zx - zy * zy + x0;
                zy = 2.0 * zx * zy + y0;
                zx = xt;
                iter = iter + 1;
            }
            sum = sum + iter;
        }
    }
    printf("%lld\n", sum);
    return 0;
}
