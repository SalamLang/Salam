fn main() {
    let limit: i64 = 2000000;
    let mut total: i64 = 0;
    let mut n: i64 = 2;
    while n <= limit {
        let mut temp: i64 = n;
        let mut d: i64 = 2;
        while d * d <= temp {
            while temp % d == 0 {
                total += 1;
                temp /= d;
            }
            d += 1;
        }
        if temp > 1 {
            total += 1;
        }
        n += 1;
    }
    println!("{}", total);
}
