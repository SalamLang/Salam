<?php

function quicksort(&$a, $lo, $hi) {
    if ($lo >= $hi) {
        return;
    }
    $pivot = $a[intdiv($lo + $hi, 2)];
    $i = $lo;
    $j = $hi;
    while ($i <= $j) {
        while ($a[$i] < $pivot) {
            $i = $i + 1;
        }
        while ($a[$j] > $pivot) {
            $j = $j - 1;
        }
        if ($i <= $j) {
            $tmp = $a[$i];
            $a[$i] = $a[$j];
            $a[$j] = $tmp;
            $i = $i + 1;
            $j = $j - 1;
        }
    }
    quicksort($a, $lo, $j);
    quicksort($a, $i, $hi);
}

$n = 300000;
$arr = array_fill(0, $n, 0);
$x = 1;
for ($i = 0; $i < $n; $i++) {
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $arr[$i] = $x;
}

quicksort($arr, 0, $n - 1);

$checksum = 0;
for ($i = 0; $i < $n; $i++) {
    $checksum = ($checksum + ($arr[$i] % 1000000007) * (($i + 1) % 1000000007)) % 1000000007;
}

echo $arr[0], "\n";
echo $arr[$n - 1], "\n";
echo $checksum, "\n";
