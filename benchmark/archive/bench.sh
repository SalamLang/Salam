#!/bin/bash
# usage: bench.sh WORKDIR DRIVER [RUNS]
W=${1:?workdir}
DRV=${2:?driver}
RUNS=${3:-3}
O=$W/out
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$O"
ulimit -v 12000000

row() {
  local label=$1 side=$2 file=$3
  shift 3
  local i
  for i in $(seq "$RUNS"); do
    rm -rf "$O"/*
    python3 -I "$HERE/measure.py" "$O/.t" timeout 900 "$@"
    read -r rc wall user sys rss < "$O/.t"
    echo "$label,$side,$file,$i,$rc,$wall,$user,$sys,$rss"
  done
}

pair() {
  local label=$1 file=$2 ref=$3 ours=$4
  row "$label" ref "$file" bash -c "$ref"
  row "$label" salam "$file" bash -c "$ours"
}

echo "format,side,input,run,rc,wall_s,user_s,sys_s,peak_rss_kb"
for d in text binary; do
  D=$W/data
  pair gunzip $d "gzip -dc $D/$d.gz > $O/o" "$DRV gunzip $D/$d.gz $O/o"
  pair bunzip2 $d "bzip2 -dc $D/$d.bz2 > $O/o" "$DRV bunzip2 $D/$d.bz2 $O/o"
  pair unxz $d "xz -dc $D/$d.xz > $O/o" "$DRV unxz $D/$d.xz $O/o"
  pair unlzma $d "xz -dc --format=lzma $D/$d.lzma > $O/o" "$DRV unlzma $D/$d.lzma $O/o"
  pair unzstd3 $d "zstd -dc -q $D/$d.zst > $O/o" "$DRV unzstd $D/$d.zst $O/o"
  pair unzstd19 $d "zstd -dc -q $D/$d.19.zst > $O/o" "$DRV unzstd $D/$d.19.zst $O/o"
  pair unzip $d "unzip -qq -o $D/$d.zip -d $O/x" "$DRV unzip $D/$d.zip $O/x"
  pair un7z $d "7z x -bso0 -bsp0 -o$O/x $D/$d.7z" "$DRV un7z $D/$d.7z $O/x"
  pair unrar5 $d "unrar x -idq -o+ $D/$d.r5.rar $O/x/" "$DRV unrar $D/$d.r5.rar $O/x"
done
for d in text binary; do
  D=$W/data
  pair gzip $d "gzip -6 -c $D/$d.bin > $O/o" "$DRV gzip $D/$d.bin $O/o"
  pair bzip2 $d "bzip2 -9 -c $D/$d.bin > $O/o" "$DRV bzip2 $D/$d.bin $O/o"
  pair xz $d "xz -6 -T1 -c $D/$d.bin > $O/o" "$DRV xz $D/$d.bin $O/o"
  pair zstd $d "zstd -3 -q -c $D/$d.bin > $O/o" "$DRV zstd $D/$d.bin $O/o"
done
