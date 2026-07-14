count = 1
for i in range(3, 100000, 2):
    is_p = 1
    j = 3
    while j * j <= i:
        if i % j == 0:
            is_p = 0
            break
        j += 2
    count += is_p
print(count)
