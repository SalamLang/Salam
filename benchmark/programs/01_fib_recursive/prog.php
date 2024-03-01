<?php

// phpcs:disable PSR1.Files.SideEffects -- benchmark script intentionally declares and runs

function fib(int $n): int
{
    if ($n < 2) {
        return $n;
    }
    return fib($n - 1) + fib($n - 2);
}

echo fib(30), "\n";
