fn main() {
    let n: i64 = 5000000;
    let range: i64 = 100000;
    let mut x: i64 = 6;
    let mut counts: Vec<i64> = vec![0; range as usize];
    let mut i: i64 = 0;
    while i < n {
        x = (x * 1103515245 + 12345) % 2147483648;
        let v = (x % range) as usize;
        counts[v] = counts[v] + 1;
        i = i + 1;
    }

    let mut checksum: i64 = 0;
    let mut pos: i64 = 0;
    let mut v: i64 = 0;
    while v < range {
        let c = counts[v as usize];
        let mut t: i64 = 0;
        while t < c {
            pos = pos + 1;
            checksum = (checksum + (v % 1000000007) * (pos % 1000000007)) % 1000000007;
            t = t + 1;
        }
        v = v + 1;
    }

    println!("{}", checksum);
}
