fn main() {
    let mut sum: f64 = 0.0;
    let mut sign: f64 = 1.0;
    let mut den: f64 = 1.0;
    for _ in 0..3000000i64 {
        sum = sum + sign / den;
        sign = 0.0 - sign;
        den = den + 2.0;
    }
    let pi = 4.0 * sum;
    let v = pi * 1000000.0;
    println!("{}", v as i64);
}
