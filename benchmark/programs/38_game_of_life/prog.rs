const W: i32 = 150;
const H: i32 = 150;
const GENERATIONS: i32 = 150;

fn main() {
    let w = W as usize;
    let h = H as usize;
    let mut grid = vec![0i32; w * h];
    let mut next_grid = vec![0i32; w * h];

    for r in 0..H {
        for c in 0..W {
            let idx = (r * W + c) as usize;
            grid[idx] = if (r * 31 + c * 17 + 7) % 5 == 0 { 1 } else { 0 };
        }
    }

    for _gen in 0..GENERATIONS {
        for r in 0..H {
            for c in 0..W {
                let mut count = 0;
                for dr in -1..=1 {
                    for dc in -1..=1 {
                        if dr == 0 && dc == 0 {
                            continue;
                        }
                        let nr = r + dr;
                        let nc = c + dc;
                        if nr >= 0 && nr < H && nc >= 0 && nc < W {
                            count = count + grid[(nr * W + nc) as usize];
                        }
                    }
                }
                let idx = (r * W + c) as usize;
                let alive = grid[idx];
                if alive == 1 {
                    next_grid[idx] = if count == 2 || count == 3 { 1 } else { 0 };
                } else {
                    next_grid[idx] = if count == 3 { 1 } else { 0 };
                }
            }
        }
        std::mem::swap(&mut grid, &mut next_grid);
    }

    let mut live_count: i64 = 0;
    for i in 0..(w * h) {
        live_count = live_count + grid[i] as i64;
    }
    println!("{}", live_count);
}
