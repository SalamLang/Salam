<?php
$acc = 0;
$lens = array(1, 10, 100, 1000, 10000, 100000, 1000000);
foreach ($lens as $L) {
    for ($i = 1; $i <= $L; $i++) {
        $acc = ($acc + $i) % 1000000007;
    }
}
echo $acc, "\n";
