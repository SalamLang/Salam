<?php

// phpcs:disable PSR1.Files.SideEffects -- benchmark script intentionally declares and runs

function gcd(int $a, int $b): int
{
    $x = $a;
    $y = $b;
    while ($y != 0) {
        $t = $x % $y;
        $x = $y;
        $y = $t;
    }
    return $x;
}

$s = 0;
for ($i = 1; $i <= 300000; $i++) {
    $s += gcd($i, 987654321);
}
echo $s, "\n";
