<?php

$reps = 200000;
$checksum = 0;
for ($r = 0; $r < $reps; $r++) {
    $a = array_fill(0, 4, array_fill(0, 4, 0.0));
    for ($i = 0; $i < 4; $i++) {
        for ($j = 0; $j < 4; $j++) {
            $a[$i][$j] = ($r + $i * 3 + $j * 7 + 1) % 97;
        }
    }
    $c = array_fill(0, 4, array_fill(0, 4, 0.0));
    for ($i = 0; $i < 4; $i++) {
        for ($j = 0; $j < 4; $j++) {
            $s = 0.0;
            for ($k = 0; $k < 4; $k++) {
                $s += $a[$i][$k] * $a[$j][$k];
            }
            $c[$i][$j] = $s;
        }
    }
    $tr = 0.0;
    for ($i = 0; $i < 4; $i++) {
        $tr += $c[$i][$i];
    }
    $checksum = ($checksum + (int)$tr) % 1000000007;
}
echo $checksum, "\n";
