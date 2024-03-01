const N: i32 = 12;

fn solve(row: i32, cols: &mut [i32], diag1: &mut [i32], diag2: &mut [i32], solutions: &mut i64) {
    if row == N {
        *solutions += 1;
        return;
    }
    for col in 0..N {
        let d1 = (row + col) as usize;
        let d2 = (row - col + N - 1) as usize;
        let c = col as usize;
        if cols[c] == 0 && diag1[d1] == 0 && diag2[d2] == 0 {
            cols[c] = 1;
            diag1[d1] = 1;
            diag2[d2] = 1;
            solve(row + 1, cols, diag1, diag2, solutions);
            cols[c] = 0;
            diag1[d1] = 0;
            diag2[d2] = 0;
        }
    }
}

fn main() {
    let mut cols = [0i32; 12];
    let mut diag1 = [0i32; 23];
    let mut diag2 = [0i32; 23];
    let mut solutions: i64 = 0;
    solve(0, &mut cols, &mut diag1, &mut diag2, &mut solutions);
    println!("{}", solutions);
}
