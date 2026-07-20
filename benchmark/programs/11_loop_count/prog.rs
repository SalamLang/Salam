fn main() {
    let mut acc: i64 = 0;
    let lens: [i64; 7] = [1, 10, 100, 1000, 10000, 100000, 1000000];
    for &l in lens.iter() {
        for i in 1..=l {
            acc = (acc + i) % 1000000007;
        }
    }
    println!("{}", acc);
}
