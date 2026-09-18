"""Many small NumPy calls: build a 4x4, transpose, multiply, trace, repeat."""

import numpy as np

reps = 200000
checksum = 0
idx = np.arange(4)
row = idx[:, None] * 3
col = idx[None, :] * 7 + 1
for r in range(reps):
    a = ((r + row + col) % 97).astype(np.float64)
    c = a @ a.T
    tr = int(np.trace(c))
    checksum = (checksum + tr) % 1000000007
print(checksum)
