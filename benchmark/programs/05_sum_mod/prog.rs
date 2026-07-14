fn main() {
    let mut s: i64 = 0;
    for i in 1..=3000000i64 {
        s = (s + i * i) % 1000000007;
    }
    println!("{}", s);
}
