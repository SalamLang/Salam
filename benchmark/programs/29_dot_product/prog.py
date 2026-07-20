"""dot_product benchmark: streaming multiply-accumulate reduction."""

n = 10000000
dot = 0
i = 0
while i < n:
    a = (i * 3 + 1) % 1009
    b = (i * 7 + 2) % 1013
    dot = (dot + a * b) % 1000000007
    i = i + 1
print(dot)
