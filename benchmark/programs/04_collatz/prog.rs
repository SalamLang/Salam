fn main() {
    let mut best_steps: i64 = 0;
    let mut best_n: i64 = 1;
    for n in 1..=30000i64 {
        let mut m = n;
        let mut c: i64 = 0;
        while m != 1 {
            if m % 2 == 0 {
                m = m / 2;
            } else {
                m = 3 * m + 1;
            }
            c += 1;
        }
        if c > best_steps {
            best_steps = c;
            best_n = n;
        }
    }
    println!("{}", best_steps);
    println!("{}", best_n);
}
