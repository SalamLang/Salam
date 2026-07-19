<?php
$NITEMS = 1000;
$TARGET = 30000;
$weight = array_fill(0, $NITEMS, 0);

for ($i = 0; $i < $NITEMS; $i++) {
    $weight[$i] = (($i * 11 + 5) % 40) + 1;
}

$reachable = array_fill(0, $TARGET + 1, 0);
$reachable[0] = 1;

for ($i = 0; $i < $NITEMS; $i++) {
    $w = $weight[$i];
    for ($s = $TARGET; $s >= $w; $s--) {
        if ($reachable[$s - $w]) {
            $reachable[$s] = 1;
        }
    }
}

$count = 0;
for ($s = 0; $s <= $TARGET; $s++) {
    if ($reachable[$s]) {
        $count = $count + 1;
    }
}

echo $count, "\n";
