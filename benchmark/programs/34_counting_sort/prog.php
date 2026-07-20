<?php

$N = 5000000;
$RANGE = 100000;
$x = 6;
$counts = array_fill(0, $RANGE, 0);
for ($i = 0; $i < $N; $i = $i + 1) {
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $v = $x % $RANGE;
    $counts[$v] = $counts[$v] + 1;
}

$checksum = 0;
$pos = 0;
for ($v = 0; $v < $RANGE; $v = $v + 1) {
    $c = $counts[$v];
    for ($t = 0; $t < $c; $t = $t + 1) {
        $pos = $pos + 1;
        $checksum = ($checksum + ($v % 1000000007) * ($pos % 1000000007)) % 1000000007;
    }
}

echo $checksum, "\n";
