fn main() {
    let amount: usize = 100000;
    let modv: i64 = 1000000007;
    let coins: [i64; 6] = [1, 5, 10, 25, 50, 100];
    let mut ways: Vec<i64> = vec![0; amount + 1];
    ways[0] = 1;

    for ci in 0..6 {
        let c = coins[ci];
        let mut a = c as usize;
        while a <= amount {
            ways[a] = (ways[a] + ways[a - c as usize]) % modv;
            a += 1;
        }
    }

    println!("{}", ways[amount]);
}
