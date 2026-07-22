<?php

// phpcs:disable PSR1.Files.SideEffects -- benchmark script intentionally declares and runs

function heapify(array &$arr, int $n, int $i): void
{
    $largest = $i;
    $l = 2 * $i + 1;
    $r = 2 * $i + 2;
    if ($l < $n && $arr[$l] > $arr[$largest]) {
        $largest = $l;
    }
    if ($r < $n && $arr[$r] > $arr[$largest]) {
        $largest = $r;
    }
    if ($largest != $i) {
        $tmp = $arr[$i];
        $arr[$i] = $arr[$largest];
        $arr[$largest] = $tmp;
        heapify($arr, $n, $largest);
    }
}

$n = 300000;
$x = 3;
$arr = array_fill(0, $n, 0);
for ($i = 0; $i < $n; $i++) {
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $arr[$i] = $x;
}
for ($i = intdiv($n, 2) - 1; $i >= 0; $i--) {
    heapify($arr, $n, $i);
}
for ($end = $n - 1; $end >= 1; $end--) {
    $tmp = $arr[0];
    $arr[0] = $arr[$end];
    $arr[$end] = $tmp;
    heapify($arr, $end, 0);
}
$checksum = 0;
for ($i = 0; $i < $n; $i++) {
    $checksum = ($checksum + ($arr[$i] % 1000000007) * (($i + 1) % 1000000007)) % 1000000007;
}
echo $arr[0] . "\n";
echo $arr[$n - 1] . "\n";
echo $checksum . "\n";
