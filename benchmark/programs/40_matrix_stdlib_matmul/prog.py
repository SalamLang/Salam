"""Matrix-stdlib benchmark: real NumPy (@), not a hand-written triple loop."""

import numpy as np

n = 400
i = np.arange(n).reshape(n, 1)
j = np.arange(n).reshape(1, n)
a = ((i * 3 + j * 7 + 1) % 97).astype(np.float64)
b = ((i * 5 + j * 2 + 3) % 89).astype(np.float64)
c = a @ b
checksum = int(c.sum()) % 1000000007
print(checksum)
