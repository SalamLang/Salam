fn main() {
    let len1: i64 = 2000;
    let len2: i64 = 2000;

    let mut seq1: Vec<i64> = vec![0; len1 as usize];
    let mut seq2: Vec<i64> = vec![0; len2 as usize];
    let mut prev: Vec<i64> = vec![0; (len2 + 1) as usize];
    let mut cur: Vec<i64> = vec![0; (len2 + 1) as usize];

    let mut i: i64 = 0;
    while i < len1 {
        seq1[i as usize] = (i * 31 + 7) % 26;
        i = i + 1;
    }

    let mut j: i64 = 0;
    while j < len2 {
        seq2[j as usize] = (j * 17 + 11) % 26;
        j = j + 1;
    }

    j = 0;
    while j <= len2 {
        prev[j as usize] = j;
        j = j + 1;
    }

    i = 1;
    while i <= len1 {
        cur[0] = i;
        j = 1;
        while j <= len2 {
            let cost: i64;
            if seq1[(i - 1) as usize] == seq2[(j - 1) as usize] {
                cost = 0;
            } else {
                cost = 1;
            }
            let del_cost = prev[j as usize] + 1;
            let ins_cost = cur[(j - 1) as usize] + 1;
            let sub_cost = prev[(j - 1) as usize] + cost;
            let mut m = del_cost;
            if ins_cost < m {
                m = ins_cost;
            }
            if sub_cost < m {
                m = sub_cost;
            }
            cur[j as usize] = m;
            j = j + 1;
        }
        j = 0;
        while j <= len2 {
            prev[j as usize] = cur[j as usize];
            j = j + 1;
        }
        i = i + 1;
    }

    println!("{}", prev[len2 as usize]);
}
