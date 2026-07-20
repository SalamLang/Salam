fn main() {
    let len: i64 = 3000000;
    let shift: i64 = 13;
    let mut x: i64 = 1;
    let mut checksum: i64 = 0;
    let mut step: i64 = 0;
    while step < len {
        x = (x * 1103515245 + 12345) % 2147483648;
        let letter = x % 26;
        let shifted = (letter + shift) % 26;
        checksum = (checksum + shifted) % 1000000007;
        step = step + 1;
    }
    println!("{}", checksum);
}
