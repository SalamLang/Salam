<?php

function powmod(int $base, int $e, int $m): int
{
    $result = 1;
    $b = $base % $m;
    $ex = $e;
    while ($ex > 0) {
        if ($ex % 2 == 1) {
            $result = ($result * $b) % $m;
        }
        $b = ($b * $b) % $m;
        $ex = intdiv($ex, 2);
    }
    return $result;
}

$s = 0;
for ($i = 1; $i <= 30000; $i++) {
    $s = ($s + powmod($i, 65537, 1000000007)) % 1000000007;
}
echo $s, "\n";
