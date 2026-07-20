<?php

$n = 5000000;
$total_runs = 0;
$sumsq = 0;
$curval = -1;
$curlen = 0;
$i = 0;
while ($i < $n) {
    $v = intdiv($i, 50) % 7;
    if ($v == $curval) {
        $curlen = $curlen + 1;
    } else {
        if ($curlen > 0) {
            $total_runs = $total_runs + 1;
            $sumsq = ($sumsq + $curlen * $curlen) % 1000000007;
        }
        $curval = $v;
        $curlen = 1;
    }
    $i = $i + 1;
}
$total_runs = $total_runs + 1;
$sumsq = ($sumsq + $curlen * $curlen) % 1000000007;
echo $total_runs, "\n";
echo $sumsq, "\n";
