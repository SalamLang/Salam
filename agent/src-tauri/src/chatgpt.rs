use base64::Engine;
use flate2::Compression;
use flate2::read::DeflateDecoder;
use flate2::write::GzEncoder;
use serde::Serialize;
use serde_json::{Value, json};
use sha2::{Digest, Sha256};
use std::fs::File;
use std::io::{self, Read, Seek, SeekFrom, Write};
use std::path::Path;
use std::time::{Duration, UNIX_EPOCH};
use tauri::{AppHandle, Emitter};

const TARGET_RAW: usize = 3 * 1024 * 1024;
const MAX_EXTRA: u64 = 2 * 1024 * 1024;
const MAX_TOTAL_GZ: usize = 800 * 1024 * 1024;
const MAX_ZIP: u64 = 8 * 1024 * 1024 * 1024;
const MAX_RAW_ENTRY: u64 = 4 * 1024 * 1024 * 1024;
const MAX_RATIO: u64 = 400;
const TRIES: u32 = 5;
const EVENT: &str = "chatgpt-import";

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Entry {
    pub name: String,
    pub method: u16,
    pub packed: u64,
    pub size: u64,
    pub offset: u64,
    pub crc: u32,
    pub flags: u16,
}

pub struct Part {
    pub kind: &'static str,
    pub name: String,
    pub raw: usize,
    pub gz: Vec<u8>,
}

#[derive(Serialize, Clone)]
struct Progress {
    phase: &'static str,
    done: usize,
    total: usize,
    bytes: u64,
}

fn u16_at(b: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([b[at], b[at + 1]])
}

fn u32_at(b: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([b[at], b[at + 1], b[at + 2], b[at + 3]])
}

fn u64_at(b: &[u8], at: usize) -> u64 {
    u64::from_le_bytes(b[at..at + 8].try_into().unwrap())
}

fn bad(message: &str) -> io::Error {
    io::Error::new(io::ErrorKind::InvalidData, message.to_string())
}

pub fn read_entries<R: Read + Seek>(file: &mut R) -> io::Result<Vec<Entry>> {
    let len = file.seek(SeekFrom::End(0))?;
    if len < 22 {
        return Err(bad("this file is too small to be a zip"));
    }
    if len > MAX_ZIP {
        return Err(bad("this file is too large to import"));
    }
    file.seek(SeekFrom::Start(0))?;
    let mut first = [0u8; 4];
    file.read_exact(&mut first)?;
    let magic = u32::from_le_bytes(first);
    if magic != 0x0403_4b50 && magic != 0x0605_4b50 {
        return Err(bad("this is not a zip file"));
    }
    let tail_len = len.min(70_000);
    file.seek(SeekFrom::Start(len - tail_len))?;
    let mut tail = vec![0u8; tail_len as usize];
    file.read_exact(&mut tail)?;
    let eocd = (0..tail.len().saturating_sub(21))
        .rev()
        .find(|&i| u32_at(&tail, i) == 0x0605_4b50)
        .ok_or_else(|| bad("this is not a zip file"))?;
    let mut count = u16_at(&tail, eocd + 10) as u64;
    let mut size = u32_at(&tail, eocd + 12) as u64;
    let mut start = u32_at(&tail, eocd + 16) as u64;
    if start == 0xFFFF_FFFF || count == 0xFFFF {
        let locator = eocd
            .checked_sub(20)
            .filter(|&i| u32_at(&tail, i) == 0x0706_4b50)
            .ok_or_else(|| bad("broken zip64 end"))?;
        let at = u64_at(&tail, locator + 8);
        file.seek(SeekFrom::Start(at))?;
        let mut head = [0u8; 56];
        file.read_exact(&mut head)?;
        if u32_at(&head, 0) != 0x0606_4b50 {
            return Err(bad("broken zip64 end"));
        }
        count = u64_at(&head, 32);
        size = u64_at(&head, 40);
        start = u64_at(&head, 48);
    }
    if size > 256 * 1024 * 1024 {
        return Err(bad("the zip index is too large"));
    }
    file.seek(SeekFrom::Start(start))?;
    let mut dir = vec![0u8; size as usize];
    file.read_exact(&mut dir)?;
    let mut out = Vec::new();
    let mut at = 0usize;
    for _ in 0..count {
        if at + 46 > dir.len() || u32_at(&dir, at) != 0x0201_4b50 {
            return Err(bad("broken zip index"));
        }
        let flags = u16_at(&dir, at + 8);
        let method = u16_at(&dir, at + 10);
        let crc = u32_at(&dir, at + 16);
        let mut packed = u32_at(&dir, at + 20) as u64;
        let mut real = u32_at(&dir, at + 24) as u64;
        let name_len = u16_at(&dir, at + 28) as usize;
        let extra_len = u16_at(&dir, at + 30) as usize;
        let note_len = u16_at(&dir, at + 32) as usize;
        let mut offset = u32_at(&dir, at + 42) as u64;
        let (size_wanted, packed_wanted, offset_wanted) = (real == 0xFFFF_FFFF, packed == 0xFFFF_FFFF, offset == 0xFFFF_FFFF);
        let name_at = at + 46;
        if name_at + name_len + extra_len + note_len > dir.len() {
            return Err(bad("broken zip index"));
        }
        let name = String::from_utf8_lossy(&dir[name_at..name_at + name_len]).to_string();
        let mut extra = &dir[name_at + name_len..name_at + name_len + extra_len];
        while extra.len() >= 4 {
            let id = u16_at(extra, 0);
            let n = u16_at(extra, 2) as usize;
            if 4 + n > extra.len() {
                break;
            }
            if id == 1 {
                let mut field = &extra[4..4 + n];
                for (value, wanted) in [(&mut real, size_wanted), (&mut packed, packed_wanted), (&mut offset, offset_wanted)] {
                    if wanted && field.len() >= 8 {
                        *value = u64_at(field, 0);
                        field = &field[8..];
                    }
                }
            }
            extra = &extra[4 + n..];
        }
        if offset.saturating_add(30) > len || offset.saturating_add(packed) > len {
            return Err(bad("the zip is truncated or damaged"));
        }
        out.push(Entry {
            name,
            method,
            packed,
            size: real,
            offset,
            crc,
            flags,
        });
        at = name_at + name_len + extra_len + note_len;
    }
    Ok(out)
}

struct Verified<R: Read> {
    inner: R,
    hasher: crc32fast::Hasher,
    seen: u64,
    size: u64,
    crc: u32,
    name: String,
}

impl<R: Read> Read for Verified<R> {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let n = self.inner.read(buf)?;
        if n == 0 {
            if self.seen != self.size {
                return Err(bad(&format!("{} is incomplete", self.name)));
            }
            if self.hasher.clone().finalize() != self.crc {
                return Err(bad(&format!("{} is corrupted (checksum mismatch)", self.name)));
            }
            return Ok(0);
        }
        self.seen += n as u64;
        if self.seen > self.size {
            return Err(bad(&format!("{} is larger than the zip says", self.name)));
        }
        self.hasher.update(&buf[..n]);
        Ok(n)
    }
}

pub fn open_entry<'a, R: Read + Seek + 'a>(mut file: R, entry: &Entry) -> io::Result<Box<dyn Read + 'a>> {
    if entry.flags & 1 != 0 {
        return Err(bad("this zip is password protected"));
    }
    if entry.size > MAX_RAW_ENTRY {
        return Err(bad("a file in this zip is too large"));
    }
    if entry.packed > 0 && entry.size / entry.packed > MAX_RATIO {
        return Err(bad("a file in this zip is suspiciously compressed"));
    }
    file.seek(SeekFrom::Start(entry.offset))?;
    let mut head = [0u8; 30];
    file.read_exact(&mut head)?;
    if u32_at(&head, 0) != 0x0403_4b50 {
        return Err(bad("broken zip entry"));
    }
    let skip = u16_at(&head, 26) as i64 + u16_at(&head, 28) as i64;
    file.seek(SeekFrom::Current(skip))?;
    let packed = file.take(entry.packed);
    let plain: Box<dyn Read + 'a> = match entry.method {
        0 => Box::new(packed),
        8 => Box::new(DeflateDecoder::new(packed)),
        other => return Err(bad(&format!("unsupported zip method {other}"))),
    };
    Ok(Box::new(Verified {
        inner: plain,
        hasher: crc32fast::Hasher::new(),
        seen: 0,
        size: entry.size,
        crc: entry.crc,
        name: entry.name.clone(),
    }))
}

#[derive(Debug, PartialEq, Eq)]
pub enum Kind {
    Conversations,
    Extra,
}

pub fn classify(entry: &Entry) -> Option<Kind> {
    let base = entry.name.rsplit('/').next().unwrap_or("").to_lowercase();
    let stem = base.strip_suffix(".json")?;
    let numbered = stem
        .strip_prefix("conversations-")
        .is_some_and(|n| !n.is_empty() && n.bytes().all(|b| b.is_ascii_digit()));
    if stem == "conversations" || numbered {
        return Some(Kind::Conversations);
    }
    let named = stem.starts_with("memories") || stem.starts_with("memory") || stem.contains("instruction") || stem.starts_with("custom");
    if named && !stem.contains("conversation") && entry.size <= MAX_EXTRA {
        return Some(Kind::Extra);
    }
    None
}

pub struct Splitter<R: Read> {
    source: R,
    chunk: Vec<u8>,
    at: usize,
    filled: usize,
    opened: bool,
    closed: bool,
    depth: u32,
    quoted: bool,
    escaped: bool,
    current: Vec<u8>,
}

impl<R: Read> Splitter<R> {
    pub fn new(source: R) -> Self {
        Splitter {
            source,
            chunk: vec![0; 64 * 1024],
            at: 0,
            filled: 0,
            opened: false,
            closed: false,
            depth: 0,
            quoted: false,
            escaped: false,
            current: Vec::new(),
        }
    }
}

impl<R: Read> Iterator for Splitter<R> {
    type Item = io::Result<Vec<u8>>;

    fn next(&mut self) -> Option<Self::Item> {
        loop {
            if self.at == self.filled {
                match self.source.read(&mut self.chunk) {
                    Ok(0) => {
                        if self.closed {
                            return None;
                        }
                        let why = if self.opened {
                            "conversations.json ends too early"
                        } else {
                            "conversations.json is not a JSON list"
                        };
                        self.closed = true;
                        return Some(Err(bad(why)));
                    }
                    Ok(n) => {
                        self.at = 0;
                        self.filled = n;
                    }
                    Err(e) => return Some(Err(e)),
                }
            }
            while self.at < self.filled {
                if self.closed {
                    self.at = self.filled;
                    break;
                }
                let b = self.chunk[self.at];
                self.at += 1;
                if !self.opened {
                    if b == b'[' {
                        self.opened = true;
                    }
                    continue;
                }
                if self.depth == 0 {
                    if b == b'{' {
                        self.depth = 1;
                        self.current.clear();
                        self.current.push(b);
                    } else if b == b']' {
                        self.closed = true;
                    }
                    continue;
                }
                self.current.push(b);
                if self.quoted {
                    if self.escaped {
                        self.escaped = false;
                    } else if b == b'\\' {
                        self.escaped = true;
                    } else if b == b'"' {
                        self.quoted = false;
                    }
                    continue;
                }
                match b {
                    b'"' => self.quoted = true,
                    b'{' | b'[' => self.depth += 1,
                    b'}' | b']' => {
                        self.depth -= 1;
                        if self.depth == 0 {
                            return Some(Ok(std::mem::take(&mut self.current)));
                        }
                    }
                    _ => {}
                }
            }
        }
    }
}

fn conversation_ok(bytes: &[u8]) -> io::Result<bool> {
    let value: Value = serde_json::from_slice(bytes).map_err(|_| bad("a conversation in this export is not valid JSON"))?;
    let id = value.get("conversation_id").or_else(|| value.get("id")).and_then(Value::as_str);
    Ok(id.is_some_and(|s| !s.is_empty()) && value.get("mapping").is_some_and(Value::is_object))
}

fn gzip(bytes: &[u8]) -> io::Result<Vec<u8>> {
    let mut encoder = GzEncoder::new(Vec::with_capacity(bytes.len() / 4 + 64), Compression::best());
    encoder.write_all(bytes)?;
    encoder.finish()
}

fn pack(items: &[Vec<u8>]) -> Vec<u8> {
    let total: usize = items.iter().map(Vec::len).sum::<usize>() + items.len() + 2;
    let mut out = Vec::with_capacity(total);
    out.push(b'[');
    for (i, item) in items.iter().enumerate() {
        if i > 0 {
            out.push(b',');
        }
        out.extend_from_slice(item);
    }
    out.push(b']');
    out
}

pub fn build_parts(path: &Path, mut tick: impl FnMut(usize, usize)) -> io::Result<Vec<Part>> {
    let mut file = File::open(path)?;
    let entries = read_entries(&mut file)?;
    let mut wanted: Vec<(Entry, Kind)> = entries.into_iter().filter_map(|e| classify(&e).map(|k| (e, k))).collect();
    wanted.sort_by(|a, b| a.0.name.cmp(&b.0.name));
    if !wanted.iter().any(|(_, k)| *k == Kind::Conversations) {
        return Err(bad("no conversations.json was found in this zip"));
    }
    let total = wanted.len();
    let mut parts = Vec::new();
    let mut held = 0usize;
    let mut found = 0usize;
    let mut odd = 0usize;
    for (n, (entry, kind)) in wanted.iter().enumerate() {
        tick(n, total);
        let reader = open_entry(File::open(path)?, entry)?;
        let base = entry.name.rsplit('/').next().unwrap_or("").to_string();
        if *kind == Kind::Extra {
            let mut raw = Vec::new();
            reader.take(MAX_EXTRA + 1).read_to_end(&mut raw)?;
            if serde_json::from_slice::<Value>(&raw).is_err() {
                return Err(bad(&format!("{base} is not valid JSON")));
            }
            let gz = gzip(&raw)?;
            held += gz.len();
            parts.push(Part {
                kind: "file",
                name: base,
                raw: raw.len(),
                gz,
            });
            continue;
        }
        let mut group: Vec<Vec<u8>> = Vec::new();
        let mut size = 0usize;
        let flush = |group: &mut Vec<Vec<u8>>, size: &mut usize, held: &mut usize, parts: &mut Vec<Part>| -> io::Result<()> {
            if group.is_empty() {
                return Ok(());
            }
            let raw = pack(group);
            let gz = gzip(&raw)?;
            *held += gz.len();
            if *held > MAX_TOTAL_GZ {
                return Err(bad("this export is too large to import"));
            }
            parts.push(Part {
                kind: "conversations",
                name: base.clone(),
                raw: raw.len(),
                gz,
            });
            group.clear();
            *size = 0;
            Ok(())
        };
        for item in Splitter::new(reader) {
            let item = item?;
            if conversation_ok(&item)? {
                found += 1;
            } else {
                odd += 1;
            }
            size += item.len();
            group.push(item);
            if size >= TARGET_RAW {
                flush(&mut group, &mut size, &mut held, &mut parts)?;
            }
        }
        flush(&mut group, &mut size, &mut held, &mut parts)?;
    }
    if found == 0 {
        return Err(bad(if odd > 0 {
            "this does not look like a ChatGPT export"
        } else {
            "this export has no conversations"
        }));
    }
    tick(total, total);
    Ok(parts)
}

pub fn fingerprint(path: &Path) -> String {
    let name = path.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();
    let meta = std::fs::metadata(path).ok();
    let size = meta.as_ref().map(|m| m.len()).unwrap_or(0);
    let modified = meta
        .and_then(|m| m.modified().ok())
        .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
        .map(|d| d.as_millis())
        .unwrap_or(0);
    let digest = Sha256::digest(format!("{name}:{size}:{modified}").as_bytes());
    hex(&digest)[..32].to_string()
}

fn hex(bytes: &[u8]) -> String {
    bytes.iter().map(|b| format!("{b:02x}")).collect()
}

struct Api {
    http: reqwest::Client,
    base: String,
    token: String,
}

impl Api {
    async fn send(&self, method: reqwest::Method, route: &str, body: Option<&Value>) -> Result<Value, String> {
        let mut wait = Duration::from_secs(1);
        for attempt in 1..=TRIES {
            let mut request = self
                .http
                .request(method.clone(), format!("{}{}", self.base, route))
                .bearer_auth(&self.token)
                .header("X-Salam-Client", "agent");
            if let Some(body) = body {
                request = request.json(body);
            }
            match request.send().await {
                Ok(response) => {
                    let status = response.status();
                    let text = response.text().await.unwrap_or_default();
                    let value: Value = serde_json::from_str(&text).unwrap_or(Value::Null);
                    if status.is_success() {
                        return Ok(value);
                    }
                    let message = value["message"].as_str().unwrap_or("The server refused the request.").to_string();
                    if status.is_server_error() && attempt < TRIES {
                        tokio::time::sleep(wait).await;
                        wait = (wait * 2).min(Duration::from_secs(10));
                        continue;
                    }
                    return Err(message);
                }
                Err(_) if attempt < TRIES => {
                    tokio::time::sleep(wait).await;
                    wait = (wait * 2).min(Duration::from_secs(10));
                }
                Err(_) => return Err("Could not reach the server. Check your internet connection.".to_string()),
            }
        }
        Err("Could not reach the server.".to_string())
    }
}

fn part_body(part: &Part) -> (Value, String) {
    let data = base64::engine::general_purpose::STANDARD.encode(&part.gz);
    let sha = hex(&Sha256::digest(data.as_bytes()));
    (
        json!({"kind": part.kind, "name": part.name, "size": part.gz.len(), "sha256": sha, "data": data}),
        sha,
    )
}

pub async fn run(app: AppHandle, path: String, token: String) -> Result<Value, String> {
    let base = crate::config::api_url();
    run_with(
        move |p| {
            let _ = app.emit(EVENT, p);
        },
        path,
        token,
        base,
    )
    .await
}

async fn run_with(emit: impl Fn(Progress) + Clone + Send + 'static, path: String, token: String, base: String) -> Result<Value, String> {
    let source = std::path::PathBuf::from(&path);
    let reading = emit.clone();
    let scan = source.clone();
    let parts = tauri::async_runtime::spawn_blocking(move || {
        build_parts(&scan, |done, total| {
            reading(Progress {
                phase: "reading",
                done,
                total,
                bytes: 0,
            });
        })
    })
    .await
    .map_err(|e| e.to_string())?
    .map_err(|e| e.to_string())?;
    let raw: usize = parts.iter().map(|p| p.raw).sum();
    let packed: usize = parts.iter().map(|p| p.gz.len()).sum();
    let api = Api {
        http: reqwest::Client::builder()
            .timeout(Duration::from_secs(120))
            .user_agent(concat!("salam-agent/", env!("CARGO_PKG_VERSION")))
            .build()
            .map_err(|e| e.to_string())?,
        base: base.trim_end_matches('/').to_string(),
        token,
    };
    let filename = source.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();
    let made = api
        .send(
            reqwest::Method::POST,
            "/imports/chatgpt",
            Some(&json!({"fingerprint": fingerprint(&source), "filename": filename, "parts": parts.len(), "raw_bytes": raw})),
        )
        .await?;
    let id = made["import"]["id"].as_u64().ok_or("The server did not start an import.")?;
    let have: Vec<(u64, String)> = made["import"]["have"]
        .as_array()
        .map(|list| {
            list.iter()
                .filter_map(|h| Some((h["n"].as_u64()?, h["sha256"].as_str()?.to_string())))
                .collect()
        })
        .unwrap_or_default();
    let total = parts.len();
    let mut sent = 0u64;
    for (n, part) in parts.iter().enumerate() {
        let (body, sha) = part_body(part);
        if !have.iter().any(|(i, s)| *i == n as u64 && *s == sha) {
            api.send(reqwest::Method::PUT, &format!("/imports/chatgpt/{id}/parts/{n}"), Some(&body))
                .await?;
        }
        sent += part.gz.len() as u64;
        emit(Progress {
            phase: "uploading",
            done: n + 1,
            total,
            bytes: sent,
        });
    }
    let started = api
        .send(reqwest::Method::POST, &format!("/imports/chatgpt/{id}/start"), None)
        .await?;
    emit(Progress {
        phase: "processing",
        done: 0,
        total: 0,
        bytes: packed as u64,
    });
    Ok(started)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    fn entry(name: &str, size: u64) -> Entry {
        Entry {
            name: name.to_string(),
            method: 8,
            packed: 1,
            size,
            offset: 0,
            crc: 0,
            flags: 0,
        }
    }

    #[test]
    fn splits_a_json_array_into_its_objects() {
        let text = br#"[ {"a": "x}\"{", "b": [1, {"c": 2}]}, {"d": "]"} ,{} ]"#;
        let items: Vec<String> = Splitter::new(&text[..]).map(|i| String::from_utf8(i.unwrap()).unwrap()).collect();
        assert_eq!(items, vec![r#"{"a": "x}\"{", "b": [1, {"c": 2}]}"#, r#"{"d": "]"}"#, "{}"]);
        let packed = pack(&items.iter().map(|s| s.as_bytes().to_vec()).collect::<Vec<_>>());
        let parsed: Value = serde_json::from_slice(&packed).unwrap();
        assert_eq!(parsed.as_array().unwrap().len(), 3);
    }

    #[test]
    fn splits_across_tiny_reads() {
        struct Drip<'a>(&'a [u8]);
        impl Read for Drip<'_> {
            fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
                let n = self.0.len().min(1).min(buf.len());
                buf[..n].copy_from_slice(&self.0[..n]);
                self.0 = &self.0[n..];
                Ok(n)
            }
        }
        let text = br#"[{"k": "\\"}, {"k": "\""}]"#;
        let items: Vec<_> = Splitter::new(Drip(text)).map(|i| i.unwrap()).collect();
        assert_eq!(items.len(), 2);
    }

    #[test]
    fn picks_the_files_that_matter() {
        assert_eq!(classify(&entry("conversations-012.json", 5)), Some(Kind::Conversations));
        assert_eq!(classify(&entry("conversations.json", 5)), Some(Kind::Conversations));
        assert_eq!(classify(&entry("export/memories.json", 5)), Some(Kind::Extra));
        assert_eq!(classify(&entry("custom_instructions.json", 5)), Some(Kind::Extra));
        assert_eq!(classify(&entry("shared_conversations.json", 5)), None);
        assert_eq!(classify(&entry("sectioned_conversations.json", 5)), None);
        assert_eq!(classify(&entry("file-abc.dat", 5)), None);
        assert_eq!(classify(&entry("user.json", 5)), None);
        assert_eq!(classify(&entry("memories.json", MAX_EXTRA + 1)), None);
    }

    fn stored_zip(files: &[(&str, &[u8])]) -> Vec<u8> {
        let mut out = Vec::new();
        let mut dir = Vec::new();
        for (name, data) in files {
            let offset = out.len() as u32;
            out.extend_from_slice(&0x0403_4b50u32.to_le_bytes());
            out.extend_from_slice(&[20, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
            out.extend_from_slice(&crc32fast::hash(data).to_le_bytes());
            out.extend_from_slice(&(data.len() as u32).to_le_bytes());
            out.extend_from_slice(&(data.len() as u32).to_le_bytes());
            out.extend_from_slice(&(name.len() as u16).to_le_bytes());
            out.extend_from_slice(&0u16.to_le_bytes());
            out.extend_from_slice(name.as_bytes());
            out.extend_from_slice(data);
            dir.extend_from_slice(&0x0201_4b50u32.to_le_bytes());
            dir.extend_from_slice(&[20, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
            dir.extend_from_slice(&crc32fast::hash(data).to_le_bytes());
            dir.extend_from_slice(&(data.len() as u32).to_le_bytes());
            dir.extend_from_slice(&(data.len() as u32).to_le_bytes());
            dir.extend_from_slice(&(name.len() as u16).to_le_bytes());
            dir.extend_from_slice(&[0; 12]);
            dir.extend_from_slice(&offset.to_le_bytes());
            dir.extend_from_slice(name.as_bytes());
        }
        let start = out.len() as u32;
        let size = dir.len() as u32;
        out.extend_from_slice(&dir);
        out.extend_from_slice(&0x0605_4b50u32.to_le_bytes());
        out.extend_from_slice(&[0, 0, 0, 0]);
        out.extend_from_slice(&(files.len() as u16).to_le_bytes());
        out.extend_from_slice(&(files.len() as u16).to_le_bytes());
        out.extend_from_slice(&size.to_le_bytes());
        out.extend_from_slice(&start.to_le_bytes());
        out.extend_from_slice(&0u16.to_le_bytes());
        out
    }

    #[test]
    fn reads_a_zip_and_builds_parts() {
        let conversations = br#"[{"id": "1", "mapping": {}}, {"id": "2", "mapping": {"x": 1}}]"#;
        let zip = stored_zip(&[
            ("conversations-000.json", conversations),
            ("file-x.dat", b"binary"),
            ("memories.json", b"{\"memories\": [\"x\"]}"),
        ]);
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("export.zip");
        std::fs::write(&path, &zip).unwrap();
        let parts = build_parts(&path, |_, _| {}).unwrap();
        assert_eq!(parts.len(), 2);
        assert_eq!(parts[0].kind, "conversations");
        let mut text = String::new();
        flate2::read::GzDecoder::new(&parts[0].gz[..]).read_to_string(&mut text).unwrap();
        let parsed: Value = serde_json::from_str(&text).unwrap();
        assert_eq!(parsed.as_array().unwrap().len(), 2);
        assert_eq!(parts[1].kind, "file");
        assert_eq!(fingerprint(&path).len(), 32);
    }

    fn build(files: &[(&str, &[u8])]) -> io::Result<Vec<Part>> {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("export.zip");
        std::fs::write(&path, stored_zip(files)).unwrap();
        build_parts(&path, |_, _| {})
    }

    fn message(r: io::Result<Vec<Part>>) -> String {
        r.err().map(|e| e.to_string()).unwrap_or_default()
    }

    #[test]
    fn rejects_bad_content() {
        let good = br#"[{"id": "1", "mapping": {}}]"#;
        assert!(build(&[("conversations.json", good)]).is_ok());
        assert!(message(build(&[("conversations.json", br#"[{"id": "1", "mapping": {}"#)])).contains("ends too early"));
        assert!(message(build(&[("conversations.json", br#"{"id": "1"}"#)])).contains("not a JSON list"));
        assert!(message(build(&[("conversations.json", br#"[{"id": "1", "mapping": {,}}]"#)])).contains("not valid JSON"));
        assert!(message(build(&[("conversations.json", br#"[{"title": "x"}]"#)])).contains("does not look like"));
        assert!(message(build(&[("conversations.json", b"[]")])).contains("no conversations"));
        assert!(message(build(&[("conversations.json", good), ("memories.json", b"{oops")])).contains("not valid JSON"));
    }

    #[test]
    fn rejects_a_corrupted_entry() {
        let good = br#"[{"id": "1", "mapping": {}}]"#;
        let mut zip = stored_zip(&[("conversations.json", good)]);
        let at = zip.windows(good.len()).position(|w| w == good).unwrap();
        zip[at + 10] ^= 0x20;
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("export.zip");
        std::fs::write(&path, &zip).unwrap();
        assert!(message(build_parts(&path, |_, _| {})).contains("checksum"));
    }

    #[test]
    fn rejects_a_truncated_or_foreign_file() {
        let zip = stored_zip(&[("conversations.json", br#"[{"id": "1", "mapping": {}}]"#)]);
        let mut cut = Cursor::new(zip[..zip.len() - 30].to_vec());
        assert!(read_entries(&mut cut).is_err());
        let mut text = Cursor::new(vec![b'x'; 200]);
        assert!(read_entries(&mut text).is_err());
    }

    #[test]
    fn a_zip_without_conversations_is_refused() {
        let zip = stored_zip(&[("user.json", b"{}")]);
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("export.zip");
        std::fs::write(&path, &zip).unwrap();
        assert!(build_parts(&path, |_, _| {}).is_err());
        let mut broken = Cursor::new(vec![1u8, 2, 3]);
        assert!(read_entries(&mut broken).is_err());
    }

    #[test]
    #[ignore = "set SALAM_TEST_CHATGPT_ZIP to a real export"]
    fn reads_a_real_export() {
        let path = std::env::var("SALAM_TEST_CHATGPT_ZIP").expect("SALAM_TEST_CHATGPT_ZIP");
        let started = std::time::Instant::now();
        let parts = build_parts(Path::new(&path), |_, _| {}).unwrap();
        let mut conversations = 0usize;
        for part in parts.iter().filter(|p| p.kind == "conversations") {
            let mut text = String::new();
            flate2::read::GzDecoder::new(&part.gz[..]).read_to_string(&mut text).unwrap();
            conversations += serde_json::from_str::<Value>(&text).unwrap().as_array().unwrap().len();
        }
        let raw: usize = parts.iter().map(|p| p.raw).sum();
        let gz: usize = parts.iter().map(|p| p.gz.len()).sum();
        println!(
            "parts={} conversations={conversations} raw={raw} gz={gz} in {:?}",
            parts.len(),
            started.elapsed()
        );
        assert!(conversations > 0);
    }

    #[tokio::test]
    #[ignore = "needs SALAM_TEST_CHATGPT_ZIP, SALAM_TEST_API and SALAM_TEST_TOKEN"]
    async fn uploads_a_real_export() {
        let zip = std::env::var("SALAM_TEST_CHATGPT_ZIP").expect("SALAM_TEST_CHATGPT_ZIP");
        let api = std::env::var("SALAM_TEST_API").expect("SALAM_TEST_API");
        let token = std::env::var("SALAM_TEST_TOKEN").expect("SALAM_TEST_TOKEN");
        let seen = std::sync::Arc::new(std::sync::Mutex::new(Vec::<(String, usize)>::new()));
        let log = seen.clone();
        let started = run_with(move |p| log.lock().unwrap().push((p.phase.to_string(), p.done)), zip, token, api)
            .await
            .unwrap();
        let events = seen.lock().unwrap();
        println!(
            "import={} status={} events={}",
            started["import"]["id"],
            started["import"]["status"],
            events.len()
        );
        assert_eq!(started["import"]["status"], "queued");
        assert_eq!(events.last().map(|e| e.0.as_str()), Some("processing"));
    }
}
