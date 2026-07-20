"""Palindrome-count benchmark."""

count = 0
for n in range(1, 2000001):
    temp = n
    rev = 0
    while temp > 0:
        rev = rev * 10 + temp % 10
        temp = temp // 10
    if rev == n:
        count = count + 1
print(count)
