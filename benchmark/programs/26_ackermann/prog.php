<?php

// phpcs:disable PSR1.Files.SideEffects -- benchmark script intentionally declares and runs

function ackermann(int $m, int $n): int
{
    if ($m == 0) {
        return $n + 1;
    }
    if ($n == 0) {
        return ackermann($m - 1, 1);
    }
    return ackermann($m - 1, ackermann($m, $n - 1));
}

echo ackermann(3, 6), "\n";
