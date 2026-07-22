<?php

// phpcs:disable PSR1.Files.SideEffects -- benchmark script intentionally declares and runs

function reverse_range(array &$arr, int $lo, int $hi): void
{
    while ($lo < $hi) {
        $tmp = $arr[$lo];
        $arr[$lo] = $arr[$hi];
        $arr[$hi] = $tmp;
        $lo = $lo + 1;
        $hi = $hi - 1;
    }
}

function main(): void
{
    $n = 4000000;
    $k = 1234567 % $n;
    $arr = array_fill(0, $n, 0);
    for ($i = 0; $i < $n; $i++) {
        $arr[$i] = $i;
    }
    reverse_range($arr, 0, $k - 1);
    reverse_range($arr, $k, $n - 1);
    reverse_range($arr, 0, $n - 1);
    $checksum = 0;
    for ($i = 0; $i < $n; $i++) {
        $checksum = ($checksum + ($arr[$i] % 1000000007) * (($i + 1) % 1000000007)) % 1000000007;
    }
    echo $arr[0] . "\n";
    echo $arr[$n - 1] . "\n";
    echo $checksum . "\n";
}

main();
