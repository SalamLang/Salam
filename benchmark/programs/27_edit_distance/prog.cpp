#include <iostream>
#include <vector>

int main()
{
    long long LEN1 = 2000;
    long long LEN2 = 2000;
    long long i, j;

    std::vector<long long> seq1(LEN1);
    std::vector<long long> seq2(LEN2);
    std::vector<long long> prev(LEN2 + 1);
    std::vector<long long> cur(LEN2 + 1);

    for (i = 0; i < LEN1; i = i + 1) {
        seq1[i] = (i * 31 + 7) % 26;
    }
    for (j = 0; j < LEN2; j = j + 1) {
        seq2[j] = (j * 17 + 11) % 26;
    }

    for (j = 0; j <= LEN2; j = j + 1) {
        prev[j] = j;
    }

    for (i = 1; i <= LEN1; i = i + 1) {
        cur[0] = i;
        for (j = 1; j <= LEN2; j = j + 1) {
            long long cost;
            long long del_cost;
            long long ins_cost;
            long long sub_cost;
            long long m;
            if (seq1[i - 1] == seq2[j - 1]) {
                cost = 0;
            } else {
                cost = 1;
            }
            del_cost = prev[j] + 1;
            ins_cost = cur[j - 1] + 1;
            sub_cost = prev[j - 1] + cost;
            m = del_cost;
            if (ins_cost < m) {
                m = ins_cost;
            }
            if (sub_cost < m) {
                m = sub_cost;
            }
            cur[j] = m;
        }
        for (j = 0; j <= LEN2; j = j + 1) {
            prev[j] = cur[j];
        }
    }

    std::cout << prev[LEN2] << "\n";

    return 0;
}
