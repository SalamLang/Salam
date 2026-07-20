const N: i64 = 2000000;
const BLOCK: i64 = 1414;

fn update(arr: &mut Vec<i64>, blocksum: &mut Vec<i64>, i: i64, delta: i64) {
    arr[i as usize] = arr[i as usize] + delta;
    let b = i / BLOCK;
    blocksum[b as usize] = blocksum[b as usize] + delta;
}

fn prefix_sum(arr: &Vec<i64>, blocksum: &Vec<i64>, i: i64) -> i64 {
    let b = i / BLOCK;
    let mut total: i64 = 0;
    let mut k: i64 = 0;
    while k < b {
        total = total + blocksum[k as usize];
        k = k + 1;
    }
    let mut start = b * BLOCK;
    while start <= i {
        total = total + arr[start as usize];
        start = start + 1;
    }
    total
}

fn main() {
    let nblocks = (N + BLOCK - 1) / BLOCK;
    let mut arr: Vec<i64> = vec![0; N as usize];
    let mut blocksum: Vec<i64> = vec![0; nblocks as usize];

    let q = 20000;
    let mut x: i64 = 8;
    let mut checksum: i64 = 0;

    let mut t = 0;
    while t < q {
        x = (x * 1103515245 + 12345) % 2147483648;
        let idx = x % N;
        x = (x * 1103515245 + 12345) % 2147483648;
        let delta = (x % 1000) + 1;
        update(&mut arr, &mut blocksum, idx, delta);
        x = (x * 1103515245 + 12345) % 2147483648;
        let qi = x % N;
        let s = prefix_sum(&arr, &blocksum, qi);
        checksum = (checksum + s) % 1000000007;
        t = t + 1;
    }

    println!("{}", checksum);
}
