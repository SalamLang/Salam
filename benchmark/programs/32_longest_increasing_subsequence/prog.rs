fn main() {
    let n: i64 = 6000;
    let mut seq: Vec<i64> = vec![0; n as usize];
    let mut dp: Vec<i64> = vec![0; n as usize];

    for i in 0..n {
        seq[i as usize] = (i * 37 + 11) % 10007;
    }

    for i in 0..n {
        dp[i as usize] = 1;
    }

    for i in 0..n {
        for j in 0..i {
            if seq[j as usize] < seq[i as usize] {
                if dp[j as usize] + 1 > dp[i as usize] {
                    dp[i as usize] = dp[j as usize] + 1;
                }
            }
        }
    }

    let mut answer = dp[0];
    for i in 0..n {
        if dp[i as usize] > answer {
            answer = dp[i as usize];
        }
    }

    println!("{}", answer);
}
