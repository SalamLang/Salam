<?php

$N = 2000000;
$BLOCK = 1414;

function update(array &$arr, array &$blocksum, int $i, int $delta, int $BLOCK): void
{
    $arr[$i] = $arr[$i] + $delta;
    $b = intdiv($i, $BLOCK);
    $blocksum[$b] = $blocksum[$b] + $delta;
}

function prefix_sum(array &$arr, array &$blocksum, int $i, int $BLOCK): int
{
    $b = intdiv($i, $BLOCK);
    $total = 0;
    $k = 0;
    while ($k < $b) {
        $total = $total + $blocksum[$k];
        $k = $k + 1;
    }
    $start = $b * $BLOCK;
    while ($start <= $i) {
        $total = $total + $arr[$start];
        $start = $start + 1;
    }
    return $total;
}

$nblocks = intdiv($N + $BLOCK - 1, $BLOCK);
$arr = array_fill(0, $N, 0);
$blocksum = array_fill(0, $nblocks, 0);

$Q = 20000;
$x = 8;
$checksum = 0;

for ($t = 0; $t < $Q; $t = $t + 1) {
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $idx = $x % $N;
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $delta = ($x % 1000) + 1;
    update($arr, $blocksum, $idx, $delta, $BLOCK);
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $qi = $x % $N;
    $s = prefix_sum($arr, $blocksum, $qi, $BLOCK);
    $checksum = ($checksum + $s) % 1000000007;
}

echo $checksum, "\n";
