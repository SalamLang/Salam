fn main() {
    let mut count: i64 = 1;
    let mut i: i64 = 3;
    while i < 100000 {
        let mut is_p: i64 = 1;
        let mut j: i64 = 3;
        while j * j <= i {
            if i % j == 0 {
                is_p = 0;
                break;
            }
            j += 2;
        }
        count += is_p;
        i += 2;
    }
    println!("{}", count);
}
