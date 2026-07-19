<?php
$a = array_fill(0, 200, array_fill(0, 200, 0));
$b = array_fill(0, 200, array_fill(0, 200, 0));
for ($i = 0; $i < 200; $i++) {
    for ($j = 0; $j < 200; $j++) {
        $a[$i][$j] = ($i * 3 + $j * 7 + 1) % 97;
    }
}
for ($i = 0; $i < 200; $i++) {
    for ($j = 0; $j < 200; $j++) {
        $b[$i][$j] = ($i * 5 + $j * 2 + 3) % 89;
    }
}
$checksum = 0;
for ($i = 0; $i < 200; $i++) {
    for ($j = 0; $j < 200; $j++) {
        $c = 0;
        for ($k = 0; $k < 200; $k++) {
            $c = $c + $a[$i][$k] * $b[$k][$j];
        }
        $checksum = ($checksum + $c) % 1000000007;
    }
}
echo $checksum, "\n";
