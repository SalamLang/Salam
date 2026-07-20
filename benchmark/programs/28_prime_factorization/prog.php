<?php

$limit = 2000000;
$total = 0;
for ($n = 2; $n <= $limit; $n++) {
    $temp = $n;
    $d = 2;
    while ($d * $d <= $temp) {
        while ($temp % $d == 0) {
            $total += 1;
            $temp = intdiv($temp, $d);
        }
        $d += 1;
    }
    if ($temp > 1) {
        $total += 1;
    }
}
echo $total, "\n";
