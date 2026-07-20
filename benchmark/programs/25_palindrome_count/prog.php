<?php

$count = 0;
for ($n = 1; $n <= 2000000; $n++) {
    $temp = $n;
    $rev = 0;
    while ($temp > 0) {
        $rev = $rev * 10 + $temp % 10;
        $temp = intdiv($temp, 10);
    }
    if ($rev == $n) {
        $count = $count + 1;
    }
}
echo $count, "\n";
