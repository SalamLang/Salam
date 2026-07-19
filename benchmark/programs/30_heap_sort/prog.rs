fn heapify(arr: &mut [i64], n: i64, i: i64) {
    let mut largest = i;
    let l = 2 * i + 1;
    let r = 2 * i + 2;
    if l < n && arr[l as usize] > arr[largest as usize] {
        largest = l;
    }
    if r < n && arr[r as usize] > arr[largest as usize] {
        largest = r;
    }
    if largest != i {
        arr.swap(i as usize, largest as usize);
        heapify(arr, n, largest);
    }
}

fn main() {
    let n: i64 = 300000;
    let mut x: i64 = 3;
    let mut arr: Vec<i64> = vec![0; n as usize];
    let mut i: i64 = 0;
    while i < n {
        x = (x * 1103515245 + 12345) % 2147483648;
        arr[i as usize] = x;
        i += 1;
    }
    let mut j = n / 2 - 1;
    while j >= 0 {
        heapify(&mut arr, n, j);
        j -= 1;
    }
    let mut end = n - 1;
    while end >= 1 {
        arr.swap(0, end as usize);
        heapify(&mut arr, end, 0);
        end -= 1;
    }
    let mut checksum: i64 = 0;
    i = 0;
    while i < n {
        checksum = (checksum + (arr[i as usize] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007;
        i += 1;
    }
    println!("{}", arr[0]);
    println!("{}", arr[(n - 1) as usize]);
    println!("{}", checksum);
}
