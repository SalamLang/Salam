fn main() {
    let nitems: i32 = 1000;
    let target: i32 = 30000;
    let mut weight: Vec<i32> = vec![0; nitems as usize];
    let mut reachable: Vec<i32> = vec![0; (target + 1) as usize];

    for i in 0..nitems {
        weight[i as usize] = ((i * 11 + 5) % 40) + 1;
    }

    reachable[0] = 1;

    for i in 0..nitems {
        let w = weight[i as usize];
        let mut s = target;
        while s >= w {
            if reachable[(s - w) as usize] == 1 {
                reachable[s as usize] = 1;
            }
            s -= 1;
        }
    }

    let mut count: i64 = 0;
    for s in 0..=target {
        if reachable[s as usize] == 1 {
            count += 1;
        }
    }

    println!("{}", count);
}
