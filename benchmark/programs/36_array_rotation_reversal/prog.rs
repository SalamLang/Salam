fn reverse_range(arr: &mut Vec<i64>, lo: i32, hi: i32) {
    let mut lo = lo;
    let mut hi = hi;
    while lo < hi {
        let tmp = arr[lo as usize];
        arr[lo as usize] = arr[hi as usize];
        arr[hi as usize] = tmp;
        lo = lo + 1;
        hi = hi - 1;
    }
}

fn main() {
    let n: i32 = 4000000;
    let k: i32 = 1234567 % n;
    let mut arr: Vec<i64> = Vec::with_capacity(n as usize);
    let mut i: i32 = 0;
    while i < n {
        arr.push(i as i64);
        i = i + 1;
    }
    reverse_range(&mut arr, 0, k - 1);
    reverse_range(&mut arr, k, n - 1);
    reverse_range(&mut arr, 0, n - 1);
    let mut checksum: i64 = 0;
    i = 0;
    while i < n {
        checksum = (checksum + (arr[i as usize] % 1000000007) * ((i as i64 + 1) % 1000000007))
            % 1000000007;
        i = i + 1;
    }
    println!("{}", arr[0]);
    println!("{}", arr[(n - 1) as usize]);
    println!("{}", checksum);
}
