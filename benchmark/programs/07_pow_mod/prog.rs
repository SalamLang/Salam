fn powmod(base: i64, e: i64, m: i64) -> i64 {
    let mut result: i64 = 1;
    let mut b = base % m;
    let mut ex = e;
    while ex > 0 {
        if ex % 2 == 1 {
            result = (result * b) % m;
        }
        b = (b * b) % m;
        ex = ex / 2;
    }
    result
}

fn main() {
    let mut s: i64 = 0;
    for i in 1..=30000i64 {
        s = (s + powmod(i, 65537, 1000000007)) % 1000000007;
    }
    println!("{}", s);
}
