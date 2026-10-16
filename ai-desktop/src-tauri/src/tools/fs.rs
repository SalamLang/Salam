use crate::workspace::Workspace;
use std::path::Path;

const MAX_READ: u64 = 10 * 1024 * 1024;
const DEFAULT_LINES: usize = 2000;
const MAX_ENTRIES: usize = 1000;

fn is_binary(bytes: &[u8]) -> bool {
    bytes.iter().take(8000).any(|&b| b == 0)
}

pub fn read_text(path: &Path) -> Result<String, String> {
    let meta = std::fs::metadata(path).map_err(|_| format!("{} does not exist.", path.display()))?;
    if meta.is_dir() {
        return Err(format!("{} is a folder; use list_dir.", path.display()));
    }
    if meta.len() > MAX_READ {
        return Err(format!(
            "{} is {} bytes, too big to read whole; use grep or a shell command.",
            path.display(),
            meta.len()
        ));
    }
    let bytes = std::fs::read(path).map_err(|e| e.to_string())?;
    if is_binary(&bytes) {
        return Err(format!("{} is a binary file of {} bytes.", path.display(), bytes.len()));
    }
    Ok(String::from_utf8_lossy(&bytes).to_string())
}

pub fn read_file(path: &Path, offset: usize, limit: usize) -> Result<String, String> {
    let text = read_text(path)?;
    let start = offset.max(1);
    let limit = if limit == 0 { DEFAULT_LINES } else { limit };
    let lines: Vec<&str> = text.lines().collect();
    if lines.is_empty() {
        return Ok("(empty file)".to_string());
    }
    let mut out = String::new();
    for (i, line) in lines.iter().enumerate().skip(start - 1).take(limit) {
        let line: String = line.chars().take(2000).collect();
        out.push_str(&format!("{:>6}\t{}\n", i + 1, line));
    }
    let shown_end = (start - 1 + limit).min(lines.len());
    if shown_end < lines.len() {
        out.push_str(&format!(
            "[lines {start} to {shown_end} of {}; pass offset to read more]\n",
            lines.len()
        ));
    }
    if out.is_empty() {
        out = format!("[the file has {} lines]", lines.len());
    }
    Ok(out)
}

pub fn old_content(path: &Path) -> String {
    std::fs::read(path)
        .ok()
        .filter(|b| !is_binary(b))
        .map(|b| String::from_utf8_lossy(&b).to_string())
        .unwrap_or_default()
}

pub fn write_file(path: &Path, content: &str) -> Result<String, String> {
    let existed = path.exists();
    if path.is_dir() {
        return Err(format!("{} is a folder.", path.display()));
    }
    if let Some(dir) = path.parent() {
        std::fs::create_dir_all(dir).map_err(|e| e.to_string())?;
    }
    std::fs::write(path, content).map_err(|e| e.to_string())?;
    Ok(format!(
        "{} {} ({} bytes).",
        if existed { "Updated" } else { "Created" },
        path.display(),
        content.len()
    ))
}

pub fn apply_edit(old: &str, old_text: &str, new_text: &str, all: bool) -> Result<(String, usize), String> {
    if old_text.is_empty() {
        return Err("old_text is empty; use write_file to create a file.".to_string());
    }
    let count = old.matches(old_text).count();
    if count == 0 {
        return Err("old_text was not found in the file. Read the file again and copy the text exactly.".to_string());
    }
    if count > 1 && !all {
        return Err(format!(
            "old_text appears {count} times. Add more surrounding lines to make it unique, or set replace_all."
        ));
    }
    let out = if all {
        old.replace(old_text, new_text)
    } else {
        old.replacen(old_text, new_text, 1)
    };
    Ok((out, if all { count } else { 1 }))
}

pub fn list_dir(ws: &Workspace, path: &Path) -> Result<String, String> {
    let rd = std::fs::read_dir(path).map_err(|_| format!("{} is not a readable folder.", path.display()))?;
    let mut names: Vec<String> = rd
        .filter_map(|e| e.ok())
        .map(|e| {
            let name = e.file_name().to_string_lossy().to_string();
            if e.file_type().map(|t| t.is_dir()).unwrap_or(false) {
                format!("{name}/")
            } else {
                name
            }
        })
        .collect();
    names.sort_by_key(|n| (!n.ends_with('/'), n.to_lowercase()));
    let total = names.len();
    names.truncate(MAX_ENTRIES);
    let mut out = format!("{} ({} entries)\n", ws.display(path), total);
    out.push_str(&names.join("\n"));
    if total > MAX_ENTRIES {
        out.push_str(&format!("\n[{} more not shown]", total - MAX_ENTRIES));
    }
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn edits_must_match_exactly_once() {
        assert_eq!(apply_edit("a b a", "b", "c", false).unwrap().0, "a c a");
        assert!(apply_edit("a b a", "a", "c", false).unwrap_err().contains("2 times"));
        assert_eq!(apply_edit("a b a", "a", "c", true).unwrap(), ("c b c".to_string(), 2));
        assert!(apply_edit("abc", "x", "y", false).unwrap_err().contains("not found"));
        assert!(apply_edit("abc", "", "y", false).is_err());
    }

    #[test]
    fn reads_number_lines_and_page() {
        let d = tempfile::tempdir().unwrap();
        let p = d.path().join("f.txt");
        std::fs::write(&p, "one\ntwo\nthree\n").unwrap();
        let all = read_file(&p, 0, 0).unwrap();
        assert!(all.contains("     1\tone") && all.contains("     3\tthree"));
        let part = read_file(&p, 2, 1).unwrap();
        assert!(part.contains("     2\ttwo") && !part.contains("three\n     ") && part.contains("lines 2 to 2 of 3"));
        std::fs::write(&p, [0u8, 1, 2]).unwrap();
        assert!(read_file(&p, 0, 0).unwrap_err().contains("binary"));
    }

    #[test]
    fn writes_create_parent_folders() {
        let d = tempfile::tempdir().unwrap();
        let p = d.path().join("a/b/c.txt");
        assert!(write_file(&p, "x").unwrap().starts_with("Created"));
        assert!(write_file(&p, "y").unwrap().starts_with("Updated"));
        assert_eq!(std::fs::read_to_string(&p).unwrap(), "y");
    }

    #[test]
    fn lists_folders_first() {
        let d = tempfile::tempdir().unwrap();
        std::fs::create_dir(d.path().join("zdir")).unwrap();
        std::fs::write(d.path().join("a.txt"), "").unwrap();
        let ws = Workspace::open(d.path().to_str().unwrap()).unwrap();
        let out = list_dir(&ws, ws.root()).unwrap();
        assert!(out.find("zdir/").unwrap() < out.find("a.txt").unwrap());
    }
}
