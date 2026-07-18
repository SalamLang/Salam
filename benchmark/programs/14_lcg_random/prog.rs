fn main() {
    let mut x: i64 = 1;
    let mut sum: i64 = 0;
    for _ in 0..5000000i64 {
        x = (x * 1103515245 + 12345) % 2147483648;
        sum = (sum + x) % 1000000007;
    }
    println!("{}", sum);
}
