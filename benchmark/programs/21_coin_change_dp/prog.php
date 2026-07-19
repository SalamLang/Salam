<?php
$AMOUNT = 100000;
$MOD = 1000000007;
$coins = [1, 5, 10, 25, 50, 100];
$ways = array_fill(0, $AMOUNT + 1, 0);
$ways[0] = 1;

foreach ($coins as $c) {
    for ($a = $c; $a <= $AMOUNT; $a++) {
        $ways[$a] = ($ways[$a] + $ways[$a - $c]) % $MOD;
    }
}

echo $ways[$AMOUNT], "\n";
