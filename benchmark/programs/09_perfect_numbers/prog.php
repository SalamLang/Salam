<?php

$count = 0;
$total = 0;
for ($n = 2; $n <= 30000; $n++) {
    $s = 1;
    for ($j = 2; $j * $j <= $n; $j++) {
        if ($n % $j == 0) {
            $s += $j;
            $q = intdiv($n, $j);
            if ($q != $j) {
                $s += $q;
            }
        }
    }
    if ($s == $n) {
        $count++;
        $total += $n;
    }
}
echo $count, "\n", $total, "\n";
