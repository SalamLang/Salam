<?php
$N = 12;
$cols = array_fill(0, $N, 0);
$diag1 = array_fill(0, 2 * $N - 1, 0);
$diag2 = array_fill(0, 2 * $N - 1, 0);
$solutions = 0;

function solve($row) {
    global $N, $cols, $diag1, $diag2, $solutions;
    if ($row == $N) {
        $solutions = $solutions + 1;
        return;
    }
    for ($col = 0; $col < $N; $col++) {
        $d1 = $row + $col;
        $d2 = $row - $col + $N - 1;
        if ($cols[$col] == 0 && $diag1[$d1] == 0 && $diag2[$d2] == 0) {
            $cols[$col] = 1;
            $diag1[$d1] = 1;
            $diag2[$d2] = 1;
            solve($row + 1);
            $cols[$col] = 0;
            $diag1[$d1] = 0;
            $diag2[$d2] = 0;
        }
    }
}

solve(0);
echo $solutions, "\n";
