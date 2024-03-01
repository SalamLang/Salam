fn main() {
    let n: i64 = 10000000;
    let mut dot: i64 = 0;
    let mut i: i64 = 0;
    while i < n {
        let a: i64 = (i * 3 + 1) % 1009;
        let b: i64 = (i * 7 + 2) % 1013;
        dot = (dot + a * b) % 1000000007;
        i += 1;
    }
    println!("{}", dot);
}
