<?php
$nitems = 500;
$capacity = 12500;
$weight = array_fill(0, $nitems, 0);
$value = array_fill(0, $nitems, 0);

for ($i = 0; $i < $nitems; $i++) {
    $weight[$i] = (($i * 7 + 3) % 50) + 1;
    $value[$i] = (($i * 13 + 5) % 100) + 1;
}

$dp = array_fill(0, $capacity + 1, 0);

for ($i = 0; $i < $nitems; $i++) {
    $wi = $weight[$i];
    $vi = $value[$i];
    for ($w = $capacity; $w >= $wi; $w--) {
        $cand = $dp[$w - $wi] + $vi;
        if ($cand > $dp[$w]) {
            $dp[$w] = $cand;
        }
    }
}

echo $dp[$capacity], "\n";
