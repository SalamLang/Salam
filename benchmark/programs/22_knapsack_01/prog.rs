fn main() {
    let nitems: i64 = 500;
    let capacity: i64 = 12500;
    let mut weight: Vec<i64> = vec![0; nitems as usize];
    let mut value: Vec<i64> = vec![0; nitems as usize];
    let mut dp: Vec<i64> = vec![0; (capacity + 1) as usize];

    for i in 0..nitems {
        weight[i as usize] = ((i * 7 + 3) % 50) + 1;
        value[i as usize] = ((i * 13 + 5) % 100) + 1;
    }

    for i in 0..nitems {
        let wi = weight[i as usize];
        let vi = value[i as usize];
        let mut w = capacity;
        while w >= wi {
            let cand = dp[(w - wi) as usize] + vi;
            if cand > dp[w as usize] {
                dp[w as usize] = cand;
            }
            w -= 1;
        }
    }

    println!("{}", dp[capacity as usize]);
}
