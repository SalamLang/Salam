<?php

$sum = 0.0;
$sign = 1.0;
$den = 1.0;
for ($k = 0; $k < 3000000; $k++) {
    $sum = $sum + $sign / $den;
    $sign = 0.0 - $sign;
    $den = $den + 2.0;
}
$pi = 4.0 * $sum;
$v = $pi * 1000000.0;
echo (int)$v, "\n";
