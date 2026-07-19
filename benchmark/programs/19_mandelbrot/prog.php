<?php

$WIDTH = 400;
$HEIGHT = 400;
$MAX_ITER = 100;
$sum = 0;
for ($py = 0; $py < $HEIGHT; $py++) {
    for ($px = 0; $px < $WIDTH; $px++) {
        $x0 = -2.0 + 3.0 * (float)$px / (float)$WIDTH;
        $y0 = -1.5 + 3.0 * (float)$py / (float)$HEIGHT;
        $zx = 0.0;
        $zy = 0.0;
        $iter = 0;
        while ($zx * $zx + $zy * $zy <= 4.0 && $iter < $MAX_ITER) {
            $xt = $zx * $zx - $zy * $zy + $x0;
            $zy = 2.0 * $zx * $zy + $y0;
            $zx = $xt;
            $iter = $iter + 1;
        }
        $sum = $sum + $iter;
    }
}
echo $sum, "\n";
