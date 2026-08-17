fn main() {
    let reps = 200000i64;
    let mut checksum: i64 = 0;
    for r in 0..reps {
        let mut a = [[0.0f64; 4]; 4];
        for i in 0..4i64 {
            for j in 0..4i64 {
                a[i as usize][j as usize] = ((r + i * 3 + j * 7 + 1) % 97) as f64;
            }
        }
        let mut c = [[0.0f64; 4]; 4];
        for i in 0..4 {
            for j in 0..4 {
                let mut s = 0.0f64;
                for k in 0..4 {
                    s += a[i][k] * a[j][k];
                }
                c[i][j] = s;
            }
        }
        let mut tr = 0.0f64;
        for i in 0..4 {
            tr += c[i][i];
        }
        checksum = (checksum + tr as i64) % 1000000007;
    }
    println!("{}", checksum);
}
