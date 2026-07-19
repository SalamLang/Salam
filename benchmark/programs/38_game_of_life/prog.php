<?php
$W = 150;
$H = 150;
$GENERATIONS = 150;

$grid = array_fill(0, $W * $H, 0);
$next_grid = array_fill(0, $W * $H, 0);

for ($r = 0; $r < $H; $r++) {
    for ($c = 0; $c < $W; $c++) {
        $grid[$r * $W + $c] = ($r * 31 + $c * 17 + 7) % 5 == 0 ? 1 : 0;
    }
}

for ($gen = 0; $gen < $GENERATIONS; $gen++) {
    for ($r = 0; $r < $H; $r++) {
        for ($c = 0; $c < $W; $c++) {
            $count = 0;
            for ($dr = -1; $dr <= 1; $dr++) {
                for ($dc = -1; $dc <= 1; $dc++) {
                    if ($dr == 0 && $dc == 0) {
                        continue;
                    }
                    $nr = $r + $dr;
                    $nc = $c + $dc;
                    if ($nr >= 0 && $nr < $H && $nc >= 0 && $nc < $W) {
                        $count = $count + $grid[$nr * $W + $nc];
                    }
                }
            }
            $alive = $grid[$r * $W + $c];
            if ($alive == 1) {
                $next_grid[$r * $W + $c] = ($count == 2 || $count == 3) ? 1 : 0;
            } else {
                $next_grid[$r * $W + $c] = ($count == 3) ? 1 : 0;
            }
        }
    }
    $tmp = $grid;
    $grid = $next_grid;
    $next_grid = $tmp;
}

$live_count = 0;
for ($i = 0; $i < $W * $H; $i++) {
    $live_count = $live_count + $grid[$i];
}
echo $live_count, "\n";
