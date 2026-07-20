fn f(x: f64) -> f64 {
    x * x * x - 2.0 * x * x + 1.0
}

fn main() {
    let a: f64 = 0.0;
    let b: f64 = 10.0;
    let steps: i64 = 10000000;
    let h: f64 = (b - a) / steps as f64;
    let mut sum: f64 = 0.5 * (f(a) + f(b));
    for i in 1..steps {
        let x = a + i as f64 * h;
        sum = sum + f(x);
    }
    let result = sum * h;
    let scaled = result * 1000000.0;
    println!("{}", scaled as i64);
}
