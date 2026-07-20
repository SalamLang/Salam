<?php

$count = 1;
for ($i = 3; $i < 100000; $i += 2) {
    $is_p = 1;
    for ($j = 3; $j * $j <= $i; $j += 2) {
        if ($i % $j == 0) {
            $is_p = 0;
            break;
        }
    }
    $count += $is_p;
}
echo $count, "\n";
