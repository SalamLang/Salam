fn main() {
    let mut a = [[0i32; 200]; 200];
    let mut b = [[0i32; 200]; 200];
    let mut checksum: i64 = 0;
    for i in 0..200 {
        for j in 0..200 {
            a[i][j] = (i as i32 * 3 + j as i32 * 7 + 1) % 97;
        }
    }
    for i in 0..200 {
        for j in 0..200 {
            b[i][j] = (i as i32 * 5 + j as i32 * 2 + 3) % 89;
        }
    }
    for i in 0..200 {
        for j in 0..200 {
            let mut c: i64 = 0;
            for k in 0..200 {
                c = c + (a[i][k] as i64) * (b[k][j] as i64);
            }
            checksum = (checksum + c) % 1000000007;
        }
    }
    println!("{}", checksum);
}
