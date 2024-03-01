use std::io::{self, BufWriter, Write};

fn main() {
    let stdout = io::stdout();
    let mut w = BufWriter::new(stdout.lock());
    for _ in 0..1000000 {
        w.write_all(b"hello\n").unwrap();
    }
}
