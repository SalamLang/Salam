const N: usize = 300000;

fn quicksort(a: &mut [i64], lo: i64, hi: i64) {
    if lo >= hi {
        return;
    }
    let pivot = a[((lo + hi) / 2) as usize];
    let mut i = lo;
    let mut j = hi;
    while i <= j {
        while a[i as usize] < pivot {
            i = i + 1;
        }
        while a[j as usize] > pivot {
            j = j - 1;
        }
        if i <= j {
            a.swap(i as usize, j as usize);
            i = i + 1;
            j = j - 1;
        }
    }
    quicksort(a, lo, j);
    quicksort(a, i, hi);
}

fn main() {
    let mut arr: Vec<i64> = vec![0; N];
    let mut x: i64 = 1;
    for i in 0..N {
        x = (x * 1103515245 + 12345) % 2147483648;
        arr[i] = x;
    }
    quicksort(&mut arr, 0, (N - 1) as i64);
    let mut checksum: i64 = 0;
    for i in 0..N {
        checksum =
            (checksum + (arr[i] % 1000000007) * (((i + 1) as i64) % 1000000007)) % 1000000007;
    }
    println!("{}", arr[0]);
    println!("{}", arr[N - 1]);
    println!("{}", checksum);
}
