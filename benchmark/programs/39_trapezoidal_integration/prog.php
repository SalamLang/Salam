<?php

function f(float $x): float
{
    return $x * $x * $x - 2.0 * $x * $x + 1.0;
}

$a = 0.0;
$b = 10.0;
$steps = 10000000;
$h = ($b - $a) / (float)$steps;
$sum = 0.5 * (f($a) + f($b));
for ($i = 1; $i < $steps; $i++) {
    $x = $a + (float)$i * $h;
    $sum = $sum + f($x);
}
$result = $sum * $h;
$scaled = $result * 1000000.0;
echo (int)$scaled, "\n";
