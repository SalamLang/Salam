use crate::workspace::Workspace;
use globset::{GlobBuilder, GlobMatcher};
use ignore::WalkBuilder;
use std::path::Path;

const MAX_FILES: usize = 500;
const MAX_MATCHES: usize = 300;
const MAX_FILE_BYTES: u64 = 2 * 1024 * 1024;

fn matcher(pattern: &str) -> Result<GlobMatcher, String> {
    let p = pattern.trim().trim_start_matches("./");
    GlobBuilder::new(p)
        .literal_separator(true)
        .build()
        .map(|g| g.compile_matcher())
        .map_err(|e| format!("Bad glob pattern: {e}"))
}

fn walker(base: &Path) -> ignore::Walk {
    WalkBuilder::new(base)
        .hidden(false)
        .git_ignore(true)
        .require_git(false)
        .filter_entry(|e| e.file_name() != ".git" && e.file_name() != "node_modules")
        .build()
}

fn rel(base: &Path, path: &Path) -> String {
    path.strip_prefix(base).unwrap_or(path).to_string_lossy().replace('\\', "/")
}

pub fn glob(ws: &Workspace, base: &Path, pattern: &str) -> Result<String, String> {
    let m = matcher(pattern)?;
    let mut hits = Vec::new();
    let mut more = 0;
    for entry in walker(base).filter_map(|e| e.ok()) {
        if !entry.file_type().map(|t| t.is_file()).unwrap_or(false) {
            continue;
        }
        if m.is_match(rel(base, entry.path())) {
            if hits.len() < MAX_FILES {
                hits.push(ws.display(entry.path()));
            } else {
                more += 1;
            }
        }
    }
    if hits.is_empty() {
        return Ok("No files match.".to_string());
    }
    hits.sort();
    let mut out = hits.join("\n");
    if more > 0 {
        out.push_str(&format!("\n[{more} more not shown; narrow the pattern]"));
    }
    Ok(out)
}

pub fn grep(ws: &Workspace, base: &Path, pattern: &str, file_glob: &str, ignore_case: bool) -> Result<String, String> {
    let re = regex::RegexBuilder::new(pattern)
        .case_insensitive(ignore_case)
        .size_limit(1 << 22)
        .build()
        .map_err(|e| format!("Bad regular expression: {e}"))?;
    let only = if file_glob.trim().is_empty() {
        None
    } else {
        let g = file_glob.trim();
        Some(matcher(if g.contains('/') { g.to_string() } else { format!("**/{g}") }.as_str())?)
    };
    let mut out = Vec::new();
    let mut truncated = false;
    let files: Vec<_> = if base.is_file() {
        vec![base.to_path_buf()]
    } else {
        walker(base)
            .filter_map(|e| e.ok())
            .filter(|e| e.file_type().map(|t| t.is_file()).unwrap_or(false))
            .map(|e| e.into_path())
            .collect()
    };
    'files: for path in files {
        if let Some(m) = &only {
            if !m.is_match(rel(base, &path)) {
                continue;
            }
        }
        if std::fs::metadata(&path).map(|m| m.len() > MAX_FILE_BYTES).unwrap_or(true) {
            continue;
        }
        let Ok(bytes) = std::fs::read(&path) else { continue };
        if bytes.iter().take(8000).any(|&b| b == 0) {
            continue;
        }
        let text = String::from_utf8_lossy(&bytes);
        for (i, line) in text.lines().enumerate() {
            if re.is_match(line) {
                if out.len() >= MAX_MATCHES {
                    truncated = true;
                    break 'files;
                }
                let shown: String = line.trim_end().chars().take(300).collect();
                out.push(format!("{}:{}: {}", ws.display(&path), i + 1, shown));
            }
        }
    }
    if out.is_empty() {
        return Ok("No matches.".to_string());
    }
    let mut s = out.join("\n");
    if truncated {
        s.push_str(&format!("\n[stopped at {MAX_MATCHES} matches; narrow the search]"));
    }
    Ok(s)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tree() -> (tempfile::TempDir, Workspace) {
        let d = tempfile::tempdir().unwrap();
        std::fs::create_dir_all(d.path().join("src/deep")).unwrap();
        std::fs::create_dir_all(d.path().join("target")).unwrap();
        std::fs::write(d.path().join("src/a.rs"), "fn alpha() {}\nfn beta() {}\n").unwrap();
        std::fs::write(d.path().join("src/deep/b.rs"), "fn Alpha() {}\n").unwrap();
        std::fs::write(d.path().join("target/c.rs"), "fn alpha() {}\n").unwrap();
        std::fs::write(d.path().join(".gitignore"), "target/\n").unwrap();
        let ws = Workspace::open(d.path().to_str().unwrap()).unwrap();
        (d, ws)
    }

    #[test]
    fn glob_respects_gitignore() {
        let (_d, ws) = tree();
        let out = glob(&ws, ws.root(), "**/*.rs").unwrap();
        assert!(out.contains("src/a.rs") && out.contains("src/deep/b.rs"));
        assert!(!out.contains("target"));
        assert_eq!(glob(&ws, ws.root(), "src/*.rs").unwrap(), "src/a.rs");
    }

    #[test]
    fn grep_finds_lines_with_numbers() {
        let (_d, ws) = tree();
        let out = grep(&ws, ws.root(), "alpha", "", false).unwrap();
        assert_eq!(out, "src/a.rs:1: fn alpha() {}");
        let ci = grep(&ws, ws.root(), "alpha", "*.rs", true).unwrap();
        assert!(ci.contains("src/deep/b.rs:1:"));
        assert!(grep(&ws, ws.root(), "(", "", false).is_err());
    }
}
