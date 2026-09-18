<?php

$n = 400;
$a = array_fill(0, $n, array_fill(0, $n, 0));
$b = array_fill(0, $n, array_fill(0, $n, 0));
for ($i = 0; $i < $n; $i++) {
    for ($j = 0; $j < $n; $j++) {
        $a[$i][$j] = ($i * 3 + $j * 7 + 1) % 97;
    }
}
for ($i = 0; $i < $n; $i++) {
    for ($j = 0; $j < $n; $j++) {
        $b[$i][$j] = ($i * 5 + $j * 2 + 3) % 89;
    }
}
$checksum = 0;
for ($i = 0; $i < $n; $i++) {
    for ($j = 0; $j < $n; $j++) {
        $c = 0;
        for ($k = 0; $k < $n; $k++) {
            $c = $c + $a[$i][$k] * $b[$k][$j];
        }
        $checksum = ($checksum + $c) % 1000000007;
    }
}
echo $checksum, "\n";
