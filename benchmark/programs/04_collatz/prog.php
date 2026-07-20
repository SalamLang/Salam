<?php

$best_steps = 0;
$best_n = 1;
for ($n = 1; $n <= 30000; $n++) {
    $m = $n;
    $c = 0;
    while ($m != 1) {
        if ($m % 2 == 0) {
            $m = intdiv($m, 2);
        } else {
            $m = 3 * $m + 1;
        }
        $c++;
    }
    if ($c > $best_steps) {
        $best_steps = $c;
        $best_n = $n;
    }
}
echo $best_steps, "\n", $best_n, "\n";
