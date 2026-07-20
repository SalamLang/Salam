<?php

$n = 6000;
$seq = array_fill(0, $n, 0);
$dp = array_fill(0, $n, 0);

for ($i = 0; $i < $n; $i++) {
    $seq[$i] = ($i * 37 + 11) % 10007;
}

for ($i = 0; $i < $n; $i++) {
    $dp[$i] = 1;
}

for ($i = 0; $i < $n; $i++) {
    for ($j = 0; $j < $i; $j++) {
        if ($seq[$j] < $seq[$i]) {
            if ($dp[$j] + 1 > $dp[$i]) {
                $dp[$i] = $dp[$j] + 1;
            }
        }
    }
}

$answer = $dp[0];
for ($i = 0; $i < $n; $i++) {
    if ($dp[$i] > $answer) {
        $answer = $dp[$i];
    }
}

echo $answer, "\n";
