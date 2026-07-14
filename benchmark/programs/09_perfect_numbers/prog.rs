fn main() {
    let mut count: i64 = 0;
    let mut total: i64 = 0;
    for n in 2..=30000i64 {
        let mut s: i64 = 1;
        let mut j: i64 = 2;
        while j * j <= n {
            if n % j == 0 {
                s += j;
                let q = n / j;
                if q != j {
                    s += q;
                }
            }
            j += 1;
        }
        if s == n {
            count += 1;
            total += n;
        }
    }
    println!("{}", count);
    println!("{}", total);
}
