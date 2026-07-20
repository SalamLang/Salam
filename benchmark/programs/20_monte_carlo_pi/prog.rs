fn main() {
    let mut x: i64 = 1;
    let mut count: i64 = 0;
    for _ in 0..5000000i64 {
        x = (x * 1103515245 + 12345) % 2147483648;
        let ux: f64 = (x as f64) / 2147483648.0 * 2.0 - 1.0;
        x = (x * 1103515245 + 12345) % 2147483648;
        let uy: f64 = (x as f64) / 2147483648.0 * 2.0 - 1.0;
        if ux * ux + uy * uy <= 1.0 {
            count = count + 1;
        }
    }
    println!("{}", count);
}
