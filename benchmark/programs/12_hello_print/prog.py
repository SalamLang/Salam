"""Repeated stdout-write benchmark."""

import sys

w = sys.stdout.write
for _ in range(1000000):
    w("hello\n")
