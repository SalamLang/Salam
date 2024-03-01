fn main() {
    let width: i32 = 400;
    let height: i32 = 400;
    let max_iter: i64 = 100;
    let mut sum: i64 = 0;
    for py in 0..height {
        for px in 0..width {
            let x0: f64 = -2.0 + 3.0 * (px as f64) / (width as f64);
            let y0: f64 = -1.5 + 3.0 * (py as f64) / (height as f64);
            let mut zx: f64 = 0.0;
            let mut zy: f64 = 0.0;
            let mut iter: i64 = 0;
            while zx * zx + zy * zy <= 4.0 && iter < max_iter {
                let xt: f64 = zx * zx - zy * zy + x0;
                zy = 2.0 * zx * zy + y0;
                zx = xt;
                iter = iter + 1;
            }
            sum = sum + iter;
        }
    }
    println!("{}", sum);
}
