<?php

$LEN1 = 2000;
$LEN2 = 2000;

$seq1 = array_fill(0, $LEN1, 0);
$seq2 = array_fill(0, $LEN2, 0);
$prev = array_fill(0, $LEN2 + 1, 0);
$cur = array_fill(0, $LEN2 + 1, 0);

for ($i = 0; $i < $LEN1; $i = $i + 1) {
    $seq1[$i] = ($i * 31 + 7) % 26;
}

for ($j = 0; $j < $LEN2; $j = $j + 1) {
    $seq2[$j] = ($j * 17 + 11) % 26;
}

for ($j = 0; $j <= $LEN2; $j = $j + 1) {
    $prev[$j] = $j;
}

for ($i = 1; $i <= $LEN1; $i = $i + 1) {
    $cur[0] = $i;
    for ($j = 1; $j <= $LEN2; $j = $j + 1) {
        if ($seq1[$i - 1] == $seq2[$j - 1]) {
            $cost = 0;
        } else {
            $cost = 1;
        }
        $del_cost = $prev[$j] + 1;
        $ins_cost = $cur[$j - 1] + 1;
        $sub_cost = $prev[$j - 1] + $cost;
        $m = $del_cost;
        if ($ins_cost < $m) {
            $m = $ins_cost;
        }
        if ($sub_cost < $m) {
            $m = $sub_cost;
        }
        $cur[$j] = $m;
    }
    for ($j = 0; $j <= $LEN2; $j = $j + 1) {
        $prev[$j] = $cur[$j];
    }
}

echo $prev[$LEN2], "\n";
