fn main() {
    const N: i64 = 1000000;
    let mut arr: Vec<i64> = vec![0; N as usize];
    for i in 0..N {
        arr[i as usize] = (i * i + 3 * i + 7) % 1000000007;
    }
    let mut sum: i64 = 0;
    for pass in 0..5i64 {
        for i in 0..N as usize {
            sum = (sum + arr[i] + pass) % 1000000007;
        }
    }
    println!("{}", sum);
}
