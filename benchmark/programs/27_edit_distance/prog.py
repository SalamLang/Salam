"""edit_distance benchmark: pure-integer Levenshtein DP."""

LEN1 = 2000
LEN2 = 2000

seq1 = [0] * LEN1
seq2 = [0] * LEN2
prev = [0] * (LEN2 + 1)
cur = [0] * (LEN2 + 1)

for i in range(LEN1):
    seq1[i] = (i * 31 + 7) % 26

for j in range(LEN2):
    seq2[j] = (j * 17 + 11) % 26

for j in range(LEN2 + 1):
    prev[j] = j

for i in range(1, LEN1 + 1):
    cur[0] = i
    for j in range(1, LEN2 + 1):
        if seq1[i - 1] == seq2[j - 1]:
            cost = 0
        else:
            cost = 1
        del_cost = prev[j] + 1
        ins_cost = cur[j - 1] + 1
        sub_cost = prev[j - 1] + cost
        m = del_cost
        if ins_cost < m:
            m = ins_cost
        if sub_cost < m:
            m = sub_cost
        cur[j] = m
    for j in range(LEN2 + 1):
        prev[j] = cur[j]

print(prev[LEN2])
