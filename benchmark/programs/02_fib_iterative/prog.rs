fn main() {
    let mut a: i64 = 0;
    let mut b: i64 = 1;
    for _ in 0..5000000i64 {
        let t = (a + b) % 1000000007;
        a = b;
        b = t;
    }
    println!("{}", a);
}
