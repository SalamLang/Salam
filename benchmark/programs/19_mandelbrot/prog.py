"""Mandelbrot escape-iteration-count benchmark."""

WIDTH = 400
HEIGHT = 400
MAX_ITER = 100
sum_ = 0
for py in range(HEIGHT):
    for px in range(WIDTH):
        x0 = -2.0 + 3.0 * float(px) / float(WIDTH)
        y0 = -1.5 + 3.0 * float(py) / float(HEIGHT)
        zx = 0.0
        zy = 0.0
        iter_ = 0
        while zx * zx + zy * zy <= 4.0 and iter_ < MAX_ITER:
            xt = zx * zx - zy * zy + x0
            zy = 2.0 * zx * zy + y0
            zx = xt
            iter_ = iter_ + 1
        sum_ = sum_ + iter_
print(sum_)
