<?php
$N = 1000000;
$MOD = 1000000007;
$arr = array_fill(0, $N, 0);
for ($i = 0; $i < $N; $i++) {
    $arr[$i] = ($i * $i + 3 * $i + 7) % $MOD;
}
$s = 0;
for ($p = 0; $p < 5; $p++) {
    for ($i = 0; $i < $N; $i++) {
        $s = ($s + $arr[$i] + $p) % $MOD;
    }
}
echo $s, "\n";
