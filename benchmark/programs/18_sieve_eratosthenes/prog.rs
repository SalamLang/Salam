fn main() {
    let limit: i64 = 3000000;
    let mut is_composite: Vec<i8> = vec![0; (limit + 1) as usize];
    is_composite[0] = 1;
    is_composite[1] = 1;
    let mut i: i64 = 2;
    while i * i <= limit {
        if is_composite[i as usize] == 0 {
            let mut j: i64 = i * i;
            while j <= limit {
                is_composite[j as usize] = 1;
                j = j + i;
            }
        }
        i = i + 1;
    }
    let mut count: i64 = 0;
    let mut k: i64 = 2;
    while k <= limit {
        if is_composite[k as usize] == 0 {
            count = count + 1;
        }
        k = k + 1;
    }
    println!("{}", count);
}
