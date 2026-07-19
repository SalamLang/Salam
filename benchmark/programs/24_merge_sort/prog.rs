const N: usize = 300000;

fn mergesort(arr: &mut [i64], aux: &mut [i64], lo: i64, hi: i64)
{
    if lo >= hi {
        return;
    }
    let mid = (lo + hi) / 2;
    mergesort(arr, aux, lo, mid);
    mergesort(arr, aux, mid + 1, hi);
    let mut i = lo;
    let mut j = mid + 1;
    let mut k = lo;
    while i <= mid && j <= hi {
        if arr[i as usize] <= arr[j as usize] {
            aux[k as usize] = arr[i as usize];
            i = i + 1;
        } else {
            aux[k as usize] = arr[j as usize];
            j = j + 1;
        }
        k = k + 1;
    }
    while i <= mid {
        aux[k as usize] = arr[i as usize];
        i = i + 1;
        k = k + 1;
    }
    while j <= hi {
        aux[k as usize] = arr[j as usize];
        j = j + 1;
        k = k + 1;
    }
    let mut m = lo;
    while m <= hi {
        arr[m as usize] = aux[m as usize];
        m = m + 1;
    }
}

fn main()
{
    let mut x: i64 = 2;
    let mut arr: Vec<i64> = vec![0; N];
    let mut aux: Vec<i64> = vec![0; N];

    for i in 0..N {
        x = (x * 1103515245 + 12345) % 2147483648;
        arr[i] = x;
    }

    mergesort(&mut arr, &mut aux, 0, (N - 1) as i64);

    let mut checksum: i64 = 0;
    for i in 0..N {
        checksum = (checksum + (arr[i] % 1000000007) * (((i + 1) as i64) % 1000000007)) % 1000000007;
    }

    println!("{}", arr[0]);
    println!("{}", arr[N - 1]);
    println!("{}", checksum);
}
