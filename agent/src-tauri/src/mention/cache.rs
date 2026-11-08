use super::index::{Entry, Index, MAX_ENTRIES, MAX_MRU};
use std::fs;
use std::io::Write;
use std::path::{Path, PathBuf};

const MAGIC: &[u8; 4] = b"SMXI";
pub const VERSION: u32 = 1;

pub struct Loaded {
    pub entries: Vec<Entry>,
    pub mru: Vec<Box<str>>,
}

pub fn hash(text: &str) -> u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for b in text.bytes() {
        h ^= u64::from(b);
        h = h.wrapping_mul(0x100000001b3);
    }
    h
}

pub fn file_for(dir: &Path, root: &Path) -> PathBuf {
    dir.join(format!("{:016x}.idx", hash(&root.to_string_lossy())))
}

pub fn encode(root: &str, index: &Index) -> Vec<u8> {
    let mut out = Vec::with_capacity(index.entries.len() * 48 + 256);
    out.extend_from_slice(MAGIC);
    out.extend_from_slice(&VERSION.to_le_bytes());
    out.extend_from_slice(&(root.len() as u32).to_le_bytes());
    out.extend_from_slice(root.as_bytes());
    out.extend_from_slice(&(index.mru.len() as u32).to_le_bytes());
    for p in &index.mru {
        out.extend_from_slice(&(p.len() as u16).to_le_bytes());
        out.extend_from_slice(p.as_bytes());
    }
    out.extend_from_slice(&(index.entries.len() as u32).to_le_bytes());
    for e in &index.entries {
        out.extend_from_slice(&(e.path.len() as u16).to_le_bytes());
        out.extend_from_slice(e.path.as_bytes());
        out.extend_from_slice(&e.mtime.to_le_bytes());
        out.push(u8::from(e.is_dir));
    }
    let sum = hash_bytes(&out);
    out.extend_from_slice(&sum.to_le_bytes());
    out
}

fn hash_bytes(bytes: &[u8]) -> u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for b in bytes {
        h ^= u64::from(*b);
        h = h.wrapping_mul(0x100000001b3);
    }
    h
}

struct Reader<'a> {
    data: &'a [u8],
    at: usize,
}

impl<'a> Reader<'a> {
    fn take(&mut self, n: usize) -> Option<&'a [u8]> {
        let end = self.at.checked_add(n)?;
        let out = self.data.get(self.at..end)?;
        self.at = end;
        Some(out)
    }

    fn u32(&mut self) -> Option<u32> {
        Some(u32::from_le_bytes(self.take(4)?.try_into().ok()?))
    }

    fn u16(&mut self) -> Option<u16> {
        Some(u16::from_le_bytes(self.take(2)?.try_into().ok()?))
    }

    fn text(&mut self, n: usize) -> Option<&'a str> {
        std::str::from_utf8(self.take(n)?).ok()
    }
}

pub fn decode(root: &str, data: &[u8]) -> Option<Loaded> {
    if data.len() < 8 + 8 {
        return None;
    }
    let (body, tail) = data.split_at(data.len() - 8);
    if hash_bytes(body).to_le_bytes() != tail {
        return None;
    }
    let mut r = Reader { data: body, at: 0 };
    if r.take(4)? != MAGIC || r.u32()? != VERSION {
        return None;
    }
    let n = r.u32()? as usize;
    if r.text(n)? != root {
        return None;
    }
    let mut mru = Vec::new();
    for _ in 0..r.u32()?.min(MAX_MRU as u32) {
        let n = r.u16()? as usize;
        mru.push(Box::from(r.text(n)?));
    }
    let count = r.u32()? as usize;
    if count > MAX_ENTRIES {
        return None;
    }
    let mut entries = Vec::with_capacity(count);
    for _ in 0..count {
        let n = r.u16()? as usize;
        let path = r.text(n)?;
        let mtime = r.u32()?;
        let is_dir = r.take(1)?[0] == 1;
        entries.push(Entry::new(path, mtime, is_dir));
    }
    if !entries.windows(2).all(|w| w[0].path < w[1].path) {
        entries.sort_by(|a, b| a.path.cmp(&b.path));
        entries.dedup_by(|a, b| a.path == b.path);
    }
    Some(Loaded { entries, mru })
}

pub fn save(file: &Path, root: &str, index: &Index) -> std::io::Result<()> {
    if let Some(dir) = file.parent() {
        fs::create_dir_all(dir)?;
    }
    let bytes = encode(root, index);
    let tmp = file.with_extension("idx.tmp");
    {
        let mut f = fs::File::create(&tmp)?;
        f.write_all(&bytes)?;
        f.sync_all().ok();
    }
    fs::rename(&tmp, file).inspect_err(|_| {
        let _ = fs::remove_file(&tmp);
    })
}

pub fn load(file: &Path, root: &str) -> Option<Loaded> {
    let data = fs::read(file).ok()?;
    let loaded = decode(root, &data);
    if loaded.is_none() {
        let _ = fs::remove_file(file);
    }
    loaded
}

#[cfg(test)]
mod tests {
    use super::super::index::tests::{build, touch};
    use super::*;

    #[test]
    fn save_and_load_round_trip_with_mru() {
        let tmp = tempfile::tempdir().unwrap();
        let root = tmp.path().join("ws");
        touch(&root.join("a/b.txt"));
        touch(&root.join("فایل.md"));
        let mut index = build(&root);
        index.remember("a/b.txt");
        let file = file_for(&tmp.path().join("cache"), &root);
        save(&file, "ws-root", &index).unwrap();
        assert!(!file.with_extension("idx.tmp").exists());
        let loaded = load(&file, "ws-root").unwrap();
        assert_eq!(loaded.entries, index.entries);
        assert_eq!(&*loaded.mru[0], "a/b.txt");
        assert!(load(&file, "other-root").is_none());
    }

    #[test]
    fn corrupt_or_old_files_are_dropped() {
        let tmp = tempfile::tempdir().unwrap();
        let root = tmp.path().join("ws");
        touch(&root.join("x.txt"));
        let index = build(&root);
        let file = file_for(tmp.path(), &root);
        save(&file, "r", &index).unwrap();
        let mut bytes = fs::read(&file).unwrap();
        let mid = bytes.len() / 2;
        bytes[mid] ^= 0xff;
        fs::write(&file, &bytes).unwrap();
        assert!(load(&file, "r").is_none());
        assert!(!file.exists());
        let mut old = encode("r", &index);
        old[4] = 99;
        let n = old.len() - 8;
        let sum = hash_bytes(&old[..n]);
        old.truncate(n);
        old.extend_from_slice(&sum.to_le_bytes());
        assert!(decode("r", &old).is_none());
        assert!(decode("r", b"junk").is_none());
    }
}
