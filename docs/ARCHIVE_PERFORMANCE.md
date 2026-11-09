# Archive speed and memory

Measured speed and peak memory of `std/archive` and `std/compress` against the reference tools installed on the test machine. Numbers are from one machine on a heavily shared host, so read the ranges, not the single values.

## Method

- Inputs, generated with fixed seed 42 (`benchmark/archive`): `text.bin`, 24.0 MB of pseudo words with some CSV style lines, and `binary.bin`, 24.0 MB of structured 16 byte records, random blocks and runs of repeated bytes.
- Reference side: gzip -6, bzip2 -9, xz -6 (single thread), zstd -3 and -19, 7z 7-Zip (zip at -mx=5, 7z LZMA2 at -mx=5), rar 7.20 (`-ma5 -m3`) to make RAR 5 files, unrar 7.20 to extract them.
- Salam side: `benchmark/archive/driver.salam` built with `salam build --release --cc="gcc -O3"` (the optimized C path, gcc 15.2), calling the `archive` file functions (`GzipDecompressFile`, `ZipExtractAll`, `SevenZipExtractAll`, `RarExtractAll` and so on). Every run writes its output to disk, as the reference tools do. Decoded output was compared byte for byte with the original input.
- Each case ran 3 times: three reference runs, then three Salam runs. Table cells are the median of the 3 runs of user plus system CPU seconds, with the min to max range in brackets. Peak memory is the largest resident set size reported by `wait4`, in MB.
- Wall time is in the raw CSV but is not used: the host load average was between 20 and 32 on 6 cores during the whole session, so wall time was 2 to 10 times the CPU time and varied a lot. CPU time is also affected by the load (shared caches and SMT), which is why some ranges are wide.
- Raw data: `benchmark/archive/results/before.csv` (before any change) and `after_inflate.csv`. Reproduce with `benchmark/archive/bench.sh WORKDIR DRIVER [RUNS]`.

## Limits

- RAR 3 (PPMd) is not in the table. rar 7.20 cannot write RAR 3 archives (`-ma4` is rejected) and the repo has no RAR 3 encoder, so there was no large input to time and no reference to compare with.
- RAR 5 was measured as one stream of a single 24 MB file. Many small files, encrypted archives, multi volume sets and filters were not measured.
- tar was not measured (it only copies bytes).
- Single machine, x86_64 AMD EPYC, 3 runs per case. The sample is too small for a significance claim; differences under about 20 percent are inside the noise.
- The bzip2 binary decode case equals the reference time; the input is random heavy, so this does not generalise.

## Results before any change

CPU seconds, median (min-max) of 3 runs; memory is peak RSS in MB.

| Case | Input | Reference CPU s | Salam CPU s | Salam / ref | Ref MB | Salam MB |
|---|---|---|---|---|---|---|
| gzip decode | text | 0.26 (0.20-0.27) | 0.74 (0.72-0.78) | 2.8x | 12 | 12 |
| gzip decode | binary | 0.12 (0.11-0.15) | 1.09 (1.08-1.18) | 8.8x | 12 | 12 |
| bzip2 decode | text | 2.29 (2.25-2.69) | 3.83 (3.32-4.32) | 1.7x | 12 | 36 |
| bzip2 decode | binary | 2.69 (2.61-2.82) | 2.68 (2.21-3.27) | 1.0x | 12 | 44 |
| xz decode | text | 0.67 (0.51-0.67) | 1.03 (1.00-1.16) | 1.5x | 12 | 33 |
| xz decode | binary | 0.60 (0.57-0.73) | 1.43 (1.40-1.52) | 2.4x | 12 | 38 |
| lzma decode | text | 0.57 (0.51-0.62) | 0.98 (0.93-1.13) | 1.7x | 12 | 33 |
| lzma decode | binary | 0.67 (0.64-0.71) | 1.46 (1.39-1.68) | 2.2x | 12 | 38 |
| zstd -3 decode | text | 0.14 (0.10-0.14) | 0.26 (0.24-0.26) | 1.8x | 12 | 34 |
| zstd -3 decode | binary | 0.08 (0.05-0.11) | 0.21 (0.15-0.23) | 2.7x | 12 | 40 |
| zstd -19 decode | text | 0.19 (0.16-0.20) | 0.40 (0.32-0.48) | 2.0x | 13 | 33 |
| zstd -19 decode | binary | 0.11 (0.10-0.13) | 0.28 (0.24-0.31) | 2.4x | 13 | 39 |
| zip (deflate) extract | text | 0.25 (0.23-0.36) | 1.44 (1.25-2.11) | 5.8x | 12 | 35 |
| zip (deflate) extract | binary | 0.21 (0.21-0.23) | 1.03 (0.96-1.09) | 5.0x | 12 | 40 |
| 7z (LZMA2) extract | text | 0.98 (0.47-1.29) | 1.38 (1.02-1.45) | 1.4x | 37 | 72 |
| 7z (LZMA2) extract | binary | 0.76 (0.59-0.80) | 1.54 (1.52-1.62) | 2.0x | 42 | 77 |
| RAR 5 extract | text | 0.31 (0.29-0.42) | 0.54 (0.52-0.66) | 1.7x | 35 | 57 |
| RAR 5 extract | binary | 0.32 (0.28-0.55) | 0.49 (0.42-0.58) | 1.5x | 33 | 63 |
| gzip -6 encode | text | 1.23 (1.07-1.26) | 1.96 (1.92-2.01) | 1.6x | 12 | 128 |
| gzip -6 encode | binary | 1.89 (1.63-1.92) | 2.90 (2.44-2.96) | 1.5x | 12 | 132 |
| bzip2 -9 encode | text | 2.60 (2.58-2.61) | 5.17 (4.92-5.36) | 2.0x | 12 | 52 |
| bzip2 -9 encode | binary | 3.45 (3.37-3.71) | 8.53 (7.70-11.19) | 2.5x | 12 | 60 |
| xz -6 encode | text | 32.35 (31.42-33.36) | 49.91 (47.36-49.93) | 1.5x | 94 | 67 |
| xz -6 encode | binary | 25.32 (24.17-26.07) | 39.74 (39.20-40.29) | 1.6x | 95 | 71 |
| zstd -3 encode | text | 0.43 (0.36-0.45) | 1.23 (1.17-1.25) | 2.9x | 32 | 48 |
| zstd -3 encode | binary | 0.29 (0.28-0.30) | 1.48 (1.44-1.57) | 5.1x | 39 | 49 |

The worst ratios were gzip and zip decode (2.8x to 8.8x), then zstd -3 encode on binary (5.1x). Everything else was between 1.0x and 2.9x. Salam uses more memory than the reference tools; no case exceeded 135 MB on 24 MB inputs.

## Profiling

callgrind on 3 MB slices of the inputs (before changes):

- gzip and zip decode: the Huffman decoder read one bit per call and walked a binary tree stored in four `Vector`s with bounds checks, and every match byte went through `BufPush(BufGet())`. This was the clear bottleneck.
- xz, lzma and 7z decode: about 120 instructions per output byte, 80 percent in the range decoder bit function and the distance decoder. The decoder state lives in a struct reached through a pointer, so the range and code values are loaded and stored on every bit. Not changed.
- zstd decode: about 54 instructions per byte, split between the block decoder and a bit peek function. Not changed.
- RAR 5 decode: about 57 instructions per byte. Not changed.
- bzip2 decode is already written on raw pointers; ratio 1.0x to 1.7x.

## Change: inflate

`std/compress/raw_inflate.salam` and `bits.salam`: a 10 bit lookup table with a canonical fallback for longer codes replaces the bit-at-a-time tree walk; `BitReadBits` reads whole bytes; literals and matches are written directly into the output buffer; stored blocks are copied in bulk. Behaviour on valid streams is unchanged and the archive, compress, zip and 7z tests pass. Oversubscribed code lengths are now rejected (zlib does the same), where the old tree builder accepted them.

After the change (same machine, 3 runs, `after_inflate.csv`; the reference column is from the earlier table):

| Case | Input | Reference CPU s | Salam CPU s before | Salam CPU s after | Salam MB |
|---|---|---|---|---|---|
| gzip decode | text | 0.26 | 0.74 | 0.40 (0.39-0.56) | 12 |
| gzip decode | binary | 0.12 | 1.09 | 0.30 (0.23-0.37) | 12 |
| zip extract | text | 0.25 | 1.44 | 0.67 (0.43-0.97) | 36 |
| zip extract | binary | 0.21 | 1.03 | 0.27 (0.23-0.27) | 40 |

Gzip and zip decode went from 2.8x to 8.8x the reference to about 1.3x to 2.7x. The after runs were taken while the host was still loaded and the reference was not re-run next to them, so treat the gap as an upper bound. callgrind shows about 33 instructions per output byte for a 3 MB zip text slice after the change.

## What is left

- LZMA and LZMA2 (xz, lzma, 7z): 1.4x to 2.4x. Keeping the range decoder state in locals inside the hot loop is the likely next step.
- zstd: 1.8x to 2.7x decode, 2.9x to 5.1x encode at level 3.
- bzip2 encode: 2.0x to 2.5x. xz encode: about 1.5x, and both take tens of seconds on 24 MB.
- RAR 3 has no measurement.
