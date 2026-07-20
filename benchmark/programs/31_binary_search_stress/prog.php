<?php

$n = 2000000;
$arr = array();
for ($i = 0; $i < $n; $i++) {
    $arr[$i] = $i * 2;
}

$q = 5000000;
$x = 5;
$checksum = 0;
for ($qi = 0; $qi < $q; $qi++) {
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $target = $x % (2 * $n);
    $lo = 0;
    $hi = $n;
    while ($lo < $hi) {
        $mid = intdiv($lo + $hi, 2);
        if ($arr[$mid] < $target) {
            $lo = $mid + 1;
        } else {
            $hi = $mid;
        }
    }
    $checksum = ($checksum + $lo) % 1000000007;
}

echo $checksum, "\n";
