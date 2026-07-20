fn main() {
    let n: i32 = 2000000;
    let mut arr: Vec<i64> = Vec::with_capacity(n as usize);
    for i in 0..n {
        arr.push((i as i64) * 2);
    }

    let q: i64 = 5000000;
    let mut x: i64 = 5;
    let mut checksum: i64 = 0;
    for _qi in 0..q {
        x = (x * 1103515245 + 12345) % 2147483648;
        let target: i64 = x % (2 * n as i64);
        let mut lo: i32 = 0;
        let mut hi: i32 = n;
        while lo < hi {
            let mid = (lo + hi) / 2;
            if arr[mid as usize] < target {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        checksum = (checksum + lo as i64) % 1000000007;
    }

    println!("{}", checksum);
}
