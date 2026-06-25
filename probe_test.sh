#!/bin/sh

out=$(sh -c 'kill -ABRT $$' 2>/dev/null)
rc=$?
echo "rc=$rc out=[$out]"

if [ "$rc" -ge 128 ]; then
  echo "DETECTED crash, signal $((rc-128))"
else
  echo "no crash detected"
fi
