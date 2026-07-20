<?php

const N = 300000;

function mergesort_arr(array &$arr, array &$aux, int $lo, int $hi): void
{
    if ($lo >= $hi) {
        return;
    }
    $mid = intdiv($lo + $hi, 2);
    mergesort_arr($arr, $aux, $lo, $mid);
    mergesort_arr($arr, $aux, $mid + 1, $hi);
    $i = $lo;
    $j = $mid + 1;
    $k = $lo;
    while ($i <= $mid && $j <= $hi) {
        if ($arr[$i] <= $arr[$j]) {
            $aux[$k] = $arr[$i];
            $i = $i + 1;
        } else {
            $aux[$k] = $arr[$j];
            $j = $j + 1;
        }
        $k = $k + 1;
    }
    while ($i <= $mid) {
        $aux[$k] = $arr[$i];
        $i = $i + 1;
        $k = $k + 1;
    }
    while ($j <= $hi) {
        $aux[$k] = $arr[$j];
        $j = $j + 1;
        $k = $k + 1;
    }
    for ($m = $lo; $m <= $hi; $m++) {
        $arr[$m] = $aux[$m];
    }
}

function main(): void
{
    $x = 2;
    $arr = array_fill(0, N, 0);
    $aux = array_fill(0, N, 0);

    for ($i = 0; $i < N; $i++) {
        $x = ($x * 1103515245 + 12345) % 2147483648;
        $arr[$i] = $x;
    }

    mergesort_arr($arr, $aux, 0, N - 1);

    $checksum = 0;
    for ($i = 0; $i < N; $i++) {
        $checksum = ($checksum + ($arr[$i] % 1000000007) * (($i + 1) % 1000000007)) % 1000000007;
    }

    echo $arr[0] . "\n";
    echo $arr[N - 1] . "\n";
    echo $checksum . "\n";
}

main();
