fn gcd(a: i64, b: i64) -> i64 {
    let mut x = a;
    let mut y = b;
    while y != 0 {
        let t = x % y;
        x = y;
        y = t;
    }
    x
}

fn main() {
    let mut s: i64 = 0;
    for i in 1..=300000i64 {
        s += gcd(i, 987654321);
    }
    println!("{}", s);
}
