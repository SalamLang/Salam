fn main() {
    let mut count: i64 = 0;
    for n in 1..=2000000i32 {
        let mut temp = n;
        let mut rev: i64 = 0;
        while temp > 0 {
            rev = rev * 10 + (temp % 10) as i64;
            temp = temp / 10;
        }
        if rev == n as i64 {
            count = count + 1;
        }
    }
    println!("{}", count);
}
