<?php
$length = 3000000;
$shift = 13;
$x = 1;
$checksum = 0;
$step = 0;
while ($step < $length) {
    $x = ($x * 1103515245 + 12345) % 2147483648;
    $letter = $x % 26;
    $shifted = ($letter + $shift) % 26;
    $checksum = ($checksum + $shifted) % 1000000007;
    $step = $step + 1;
}
echo $checksum, "\n";
