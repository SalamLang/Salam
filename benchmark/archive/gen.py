import random, struct, sys
r = random.Random(42)
words = [''.join(r.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(r.randint(2, 9))) for _ in range(5000)]
out = []
n = 0
while n < 24_000_000:
    line = ' '.join(words[int(r.random() ** 3 * 5000)] for _ in range(r.randint(5, 14))) + '\n'
    if r.random() < 0.1:
        line = '%d,%d,%s\n' % (r.randint(0, 10 ** 6), r.randint(0, 999), line.strip())
    out.append(line)
    n += len(line)
open(sys.argv[1] + '/text.bin', 'w').write(''.join(out))
b = bytearray()
while len(b) < 24_000_000:
    k = r.random()
    if k < 0.6:
        for i in range(2000):
            b += struct.pack('<IHHf', len(b) // 16 + i, r.randint(0, 40), r.choice([1, 2, 4, 8]), r.random() * 100 if r.random() < .5 else 0.0)
    elif k < 0.9:
        b += r.randbytes(20000)
    else:
        b += bytes([r.randint(0, 3)]) * r.randint(100, 5000)
open(sys.argv[1] + '/binary.bin', 'wb').write(bytes(b[:24_000_000]))
