<?php
$s = 0;
for ($i = 1; $i <= 500000; $i++) {
    $m = $i;
    while ($m > 0) {
        $s += $m % 10;
        $m = intdiv($m, 10);
    }
}
echo $s, "\n";
