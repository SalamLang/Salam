const N: usize = 400;

fn main() {
    let mut a = vec![vec![0i32; N]; N];
    let mut b = vec![vec![0i32; N]; N];
    let mut checksum: i64 = 0;
    for i in 0..N {
        for j in 0..N {
            a[i][j] = (i as i32 * 3 + j as i32 * 7 + 1) % 97;
        }
    }
    for i in 0..N {
        for j in 0..N {
            b[i][j] = (i as i32 * 5 + j as i32 * 2 + 3) % 89;
        }
    }
    for i in 0..N {
        for j in 0..N {
            let mut c: i64 = 0;
            for k in 0..N {
                c = c + (a[i][k] as i64) * (b[k][j] as i64);
            }
            checksum = (checksum + c) % 1000000007;
        }
    }
    println!("{}", checksum);
}
