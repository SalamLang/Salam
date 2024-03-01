fn main() {
    let mut s: i64 = 0;
    for i in 1..=500000i64 {
        let mut m = i;
        while m > 0 {
            s += m % 10;
            m = m / 10;
        }
    }
    println!("{}", s);
}
