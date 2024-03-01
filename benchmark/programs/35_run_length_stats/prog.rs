fn main() {
    let n: i64 = 5000000;
    let mut total_runs: i64 = 0;
    let mut sumsq: i64 = 0;
    let mut curval: i64 = -1;
    let mut curlen: i64 = 0;
    let mut i: i64 = 0;
    while i < n {
        let v: i64 = (i / 50) % 7;
        if v == curval {
            curlen = curlen + 1;
        } else {
            if curlen > 0 {
                total_runs = total_runs + 1;
                sumsq = (sumsq + curlen * curlen) % 1000000007;
            }
            curval = v;
            curlen = 1;
        }
        i += 1;
    }
    total_runs = total_runs + 1;
    sumsq = (sumsq + curlen * curlen) % 1000000007;
    println!("{}", total_runs);
    println!("{}", sumsq);
}
