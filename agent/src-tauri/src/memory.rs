use serde::Serialize;
use std::io::Read;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

const DIR: &str = ".salam-agent";
const MAX_SLUG: usize = 60;
const MAX_INSTRUCTIONS: usize = 32 * 1024;
const MAX_INDEX: usize = 16 * 1024;
const CUT: &str = "[... cut, read the file for the rest ...]";
const INSTRUCTION_FILES: [&str; 2] = ["AGENTS.md", "AGENT.md"];

#[derive(Clone, Debug, Default, Serialize, PartialEq, Eq)]
#[serde(rename_all = "snake_case")]
pub struct ProjectContext {
    pub instructions_file: String,
    pub instructions: String,
    pub memory_index: String,
    pub memories: usize,
}

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Memory {
    pub slug: String,
    pub title: String,
    pub description: String,
    pub created: String,
    pub updated: String,
    pub content: String,
}

pub struct Draft {
    pub slug: String,
    pub before: String,
    pub after: String,
}

fn memory_dir(root: &Path) -> PathBuf {
    root.join(DIR).join("memory")
}

fn index_path(root: &Path) -> PathBuf {
    root.join(DIR).join("index.md")
}

pub fn path_of(root: &Path, slug: &str) -> PathBuf {
    memory_dir(root).join(format!("{slug}.md"))
}

pub fn rel(slug: &str) -> String {
    format!("{DIR}/memory/{slug}.md")
}

fn rfc3339(t: SystemTime) -> String {
    let secs = t.duration_since(UNIX_EPOCH).map(|d| d.as_secs() as i64).unwrap_or(0);
    let (days, rem) = (secs.div_euclid(86_400), secs.rem_euclid(86_400));
    let z = days + 719_468;
    let era = z.div_euclid(146_097);
    let doe = z - era * 146_097;
    let yoe = (doe - doe / 1460 + doe / 36_524 - doe / 146_096) / 365;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let day = doy - (153 * mp + 2) / 5 + 1;
    let month = if mp < 10 { mp + 3 } else { mp - 9 };
    let year = yoe + era * 400 + if month <= 2 { 1 } else { 0 };
    format!(
        "{year:04}-{month:02}-{day:02}T{:02}:{:02}:{:02}Z",
        rem / 3600,
        rem % 3600 / 60,
        rem % 60
    )
}

fn now() -> String {
    rfc3339(SystemTime::now())
}

fn one_line(s: &str) -> String {
    s.split(['\r', '\n'])
        .map(str::trim)
        .filter(|l| !l.is_empty())
        .collect::<Vec<_>>()
        .join(" ")
}

pub fn slugify(title: &str) -> String {
    let mut out = String::new();
    for c in title.chars().flat_map(char::to_lowercase) {
        if c.is_alphanumeric() {
            out.push(c);
        } else if !out.is_empty() && !out.ends_with('-') {
            out.push('-');
        }
    }
    let out: String = out.trim_end_matches('-').chars().take(MAX_SLUG).collect();
    out.trim_end_matches('-').to_string()
}

pub fn check_slug(slug: &str) -> Result<String, String> {
    let slug = slug.trim();
    let slug = slug.strip_suffix(".md").unwrap_or(slug);
    if slug.is_empty() {
        return Err("slug is empty.".to_string());
    }
    if slug.contains(['/', '\\']) || slug.contains("..") || slug.starts_with('.') {
        return Err(format!(
            "The slug {slug} may not contain path separators or ..; pass a plain name like build-steps."
        ));
    }
    if slug.chars().any(|c| c.is_control() || "<>:\"|?*".contains(c)) {
        return Err(format!("The slug {slug} contains characters that are not allowed in a file name."));
    }
    Ok(slug.to_string())
}

fn resolve_slug(slug: Option<&str>, title: &str) -> Result<String, String> {
    if let Some(s) = slug.filter(|s| !s.trim().is_empty()) {
        return check_slug(s);
    }
    let derived = slugify(title);
    if !derived.is_empty() {
        return Ok(derived);
    }
    let secs = SystemTime::now().duration_since(UNIX_EPOCH).map(|d| d.as_secs()).unwrap_or(0);
    Ok(format!("memory-{secs}"))
}

fn unquote(v: &str) -> &str {
    let v = v.trim();
    for q in ['"', '\''] {
        if v.len() >= 2 && v.starts_with(q) && v.ends_with(q) {
            return &v[1..v.len() - 1];
        }
    }
    v
}

pub fn parse(slug: &str, text: &str) -> Memory {
    let text = text.strip_prefix('\u{feff}').unwrap_or(text).replace("\r\n", "\n");
    let lines: Vec<&str> = text.split('\n').collect();
    let mut m = Memory {
        slug: slug.to_string(),
        ..Memory::default()
    };
    let close = if lines.first().is_some_and(|l| l.trim() == "---") {
        lines.iter().skip(1).position(|l| l.trim() == "---").map(|i| i + 1)
    } else {
        None
    };
    let body = match close {
        Some(end) => {
            for line in &lines[1..end] {
                let Some((key, value)) = line.split_once(':') else { continue };
                let value = unquote(value).to_string();
                match key.trim().to_lowercase().as_str() {
                    "title" => m.title = value,
                    "description" => m.description = value,
                    "created" => m.created = value,
                    "updated" => m.updated = value,
                    _ => {}
                }
            }
            lines[end + 1..].join("\n")
        }
        None => text.clone(),
    };
    m.content = body.trim_start_matches(['\n', ' ', '\t']).trim_end().to_string();
    if m.title.trim().is_empty() {
        m.title = slug.to_string();
    }
    m
}

pub fn render(m: &Memory) -> String {
    format!(
        "---\ntitle: {}\ndescription: {}\ncreated: {}\nupdated: {}\n---\n\n{}\n",
        one_line(&m.title),
        one_line(&m.description),
        m.created,
        m.updated,
        m.content.replace("\r\n", "\n").trim_end()
    )
}

fn read_lossy(path: &Path) -> Option<String> {
    std::fs::read(path).ok().map(|b| String::from_utf8_lossy(&b).to_string())
}

fn files(root: &Path) -> Vec<(String, PathBuf)> {
    let Ok(rd) = std::fs::read_dir(memory_dir(root)) else {
        return Vec::new();
    };
    let mut out: Vec<(String, PathBuf)> = rd
        .filter_map(|e| e.ok())
        .map(|e| e.path())
        .filter(|p| p.is_file() && p.extension().is_some_and(|x| x == "md"))
        .filter_map(|p| Some((p.file_stem()?.to_string_lossy().to_string(), p)))
        .collect();
    out.sort();
    out
}

pub fn list(root: &Path) -> Vec<Memory> {
    let mut out: Vec<Memory> = files(root)
        .into_iter()
        .filter_map(|(slug, p)| Some(parse(&slug, &read_lossy(&p)?)))
        .collect();
    out.sort_by(|a, b| b.updated.cmp(&a.updated).then_with(|| a.slug.cmp(&b.slug)));
    out
}

fn link_target(slug: &str) -> String {
    let mut out = String::from("memory/");
    for c in slug.chars() {
        match c {
            '%' => out.push_str("%25"),
            ' ' => out.push_str("%20"),
            '(' => out.push_str("%28"),
            ')' => out.push_str("%29"),
            _ => out.push(c),
        }
    }
    out.push_str(".md");
    out
}

fn shown_time(stamp: &str) -> String {
    let b = stamp.as_bytes();
    if b.len() >= 16 && b[10] == b'T' && stamp.is_char_boundary(16) {
        format!("{} {} UTC", &stamp[..10], &stamp[11..16])
    } else {
        stamp.to_string()
    }
}

fn index_text(memories: &[Memory]) -> String {
    let mut out =
        String::from("# Salam Agent memory\n\nWhat Salam Agent remembers about this project. Each memory is one file under memory/.\n\n");
    if memories.is_empty() {
        out.push_str("No memories yet.\n");
    }
    for m in memories {
        let title = one_line(&m.title).replace('[', "\\[").replace(']', "\\]");
        out.push_str(&format!("- [{title}]({})", link_target(&m.slug)));
        let description = one_line(&m.description);
        if !description.is_empty() {
            out.push_str(&format!(" - {description}"));
        }
        if !m.updated.trim().is_empty() {
            out.push_str(&format!(" (updated {})", shown_time(m.updated.trim())));
        }
        out.push('\n');
    }
    out
}

pub fn rebuild_index(root: &Path) -> Result<usize, String> {
    let memories = list(root);
    let path = index_path(root);
    std::fs::create_dir_all(root.join(DIR)).map_err(|e| e.to_string())?;
    std::fs::write(&path, index_text(&memories)).map_err(|e| e.to_string())?;
    Ok(memories.len())
}

fn count(n: usize) -> String {
    format!("{n} {}", if n == 1 { "memory" } else { "memories" })
}

pub fn draft(root: &Path, slug: Option<&str>, title: &str, description: &str, content: &str) -> Result<Draft, String> {
    let title = one_line(title);
    if title.is_empty() {
        return Err("title is empty.".to_string());
    }
    if content.trim().is_empty() {
        return Err("content is empty.".to_string());
    }
    let slug = resolve_slug(slug, &title)?;
    let path = path_of(root, &slug);
    if path.is_dir() {
        return Err(format!("{} is a folder.", rel(&slug)));
    }
    let before = read_lossy(&path).unwrap_or_default();
    let stamp = now();
    let old = parse(&slug, &before);
    let created = if before.is_empty() || old.created.trim().is_empty() {
        stamp.clone()
    } else {
        old.created
    };
    let after = render(&Memory {
        slug: slug.clone(),
        title,
        description: one_line(description),
        created,
        updated: stamp,
        content: content.to_string(),
    });
    Ok(Draft { slug, before, after })
}

pub fn save(root: &Path, slug: Option<&str>, title: &str, description: &str, content: &str) -> Result<String, String> {
    let d = draft(root, slug, title, description, content)?;
    std::fs::create_dir_all(memory_dir(root)).map_err(|e| e.to_string())?;
    std::fs::write(path_of(root, &d.slug), &d.after).map_err(|e| e.to_string())?;
    let n = rebuild_index(root)?;
    Ok(format!("Saved {} ({} in {DIR}/index.md).", rel(&d.slug), count(n)))
}

pub fn forget(root: &Path, slug: &str) -> Result<String, String> {
    let slug = check_slug(slug)?;
    let path = path_of(root, &slug);
    if !path.is_file() {
        return Err(format!("{} does not exist.", rel(&slug)));
    }
    std::fs::remove_file(&path).map_err(|e| e.to_string())?;
    let n = rebuild_index(root)?;
    Ok(format!("Forgot {} ({} left in {DIR}/index.md).", rel(&slug), count(n)))
}

fn read_capped(path: &Path, max: usize) -> Option<String> {
    let mut bytes = Vec::new();
    std::fs::File::open(path).ok()?.take(max as u64 + 4).read_to_end(&mut bytes).ok()?;
    let mut text = String::from_utf8_lossy(&bytes).to_string();
    if text.len() > max {
        let mut cut = max - CUT.len() - 1;
        while !text.is_char_boundary(cut) {
            cut -= 1;
        }
        text.truncate(cut);
        if !text.ends_with('\n') {
            text.push('\n');
        }
        text.push_str(CUT);
    }
    Some(text)
}

pub fn project_context(root: &Path) -> ProjectContext {
    let mut ctx = ProjectContext::default();
    for name in INSTRUCTION_FILES {
        let path = root.join(name);
        if !path.is_file() {
            continue;
        }
        if let Some(text) = read_capped(&path, MAX_INSTRUCTIONS) {
            ctx.instructions_file = name.to_string();
            ctx.instructions = text;
            break;
        }
    }
    let index = index_path(root);
    if index.is_file() {
        ctx.memory_index = read_capped(&index, MAX_INDEX).unwrap_or_default();
    }
    ctx.memories = files(root).len();
    ctx
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Duration;

    fn root() -> tempfile::TempDir {
        tempfile::tempdir().unwrap()
    }

    fn read(d: &tempfile::TempDir, rel: &str) -> String {
        std::fs::read_to_string(d.path().join(rel)).unwrap()
    }

    #[test]
    fn times_are_rfc3339_utc() {
        assert_eq!(rfc3339(UNIX_EPOCH), "1970-01-01T00:00:00Z");
        assert_eq!(rfc3339(UNIX_EPOCH + Duration::from_secs(951_782_400)), "2000-02-29T00:00:00Z");
        assert_eq!(rfc3339(UNIX_EPOCH + Duration::from_secs(1_792_981_800)), "2026-10-26T02:30:00Z");
    }

    #[test]
    fn slugs_keep_letters_of_any_script() {
        assert_eq!(slugify("Build steps (Linux)"), "build-steps-linux");
        assert_eq!(slugify("  --Hello,   World!--  "), "hello-world");
        assert_eq!(slugify("روش ساخت پروژه"), "روش-ساخت-پروژه");
        assert_eq!(slugify("نسخه ۲ API"), "نسخه-۲-api");
        assert_eq!(slugify("!!!"), "");
        assert_eq!(slugify(&format!("{}-bcd", "a".repeat(59))), "a".repeat(59));
        assert_eq!(slugify(&"ب".repeat(100)).chars().count(), 60);
        assert!(resolve_slug(None, "???").unwrap().starts_with("memory-"));
        assert_eq!(resolve_slug(Some("Own Slug"), "x").unwrap(), "Own Slug");
        assert_eq!(resolve_slug(Some("notes.md"), "x").unwrap(), "notes");
        for bad in ["../x", "a/b", "a\\b", "..", ".hidden", "x..y"] {
            assert!(check_slug(bad).is_err(), "{bad} should be refused");
        }
    }

    #[test]
    fn save_writes_the_memory_and_a_linked_index() {
        let d = root();
        let msg = save(
            d.path(),
            None,
            "Build steps",
            "How to build\nthe app",
            "Run cargo build.\r\nThen test.",
        )
        .unwrap();
        assert_eq!(msg, "Saved .salam-agent/memory/build-steps.md (1 memory in .salam-agent/index.md).");
        let file = read(&d, ".salam-agent/memory/build-steps.md");
        assert!(file.starts_with("---\ntitle: Build steps\ndescription: How to build the app\ncreated: 20"));
        assert!(file.ends_with("---\n\nRun cargo build.\nThen test.\n"));
        let index = read(&d, ".salam-agent/index.md");
        assert!(index.starts_with("# Salam Agent memory\n\n"));
        let line = index.lines().find(|l| l.starts_with("- [")).unwrap();
        assert!(line.starts_with("- [Build steps](memory/build-steps.md) - How to build the app (updated 20"));
        assert!(line.ends_with(" UTC)"));
        let target = line.split("](").nth(1).unwrap().split(')').next().unwrap();
        assert!(d.path().join(".salam-agent").join(target).is_file());
        assert!(save(d.path(), None, " ", "", "x").is_err());
        assert!(save(d.path(), None, "t", "", "\n ").is_err());
        assert!(save(d.path(), Some("../escape"), "t", "", "x").is_err());
        save(d.path(), Some("my notes (old)"), "Notes", "", "n").unwrap();
        let index = read(&d, ".salam-agent/index.md");
        assert!(index.contains("](memory/my%20notes%20%28old%29.md)"));
        assert!(index.contains("- [Notes](memory/my%20notes%20%28old%29.md) (updated"));
    }

    #[test]
    fn resave_keeps_created_and_moves_updated() {
        let d = root();
        let path = d.path().join(".salam-agent/memory/x.md");
        std::fs::create_dir_all(path.parent().unwrap()).unwrap();
        std::fs::write(
            &path,
            "---\ntitle: Old\ndescription: d\ncreated: 2020-01-02T03:04:05Z\nupdated: 2020-01-02T03:04:05Z\n---\n\nold\n",
        )
        .unwrap();
        save(d.path(), Some("x"), "New", "", "new text").unwrap();
        let m = parse("x", &std::fs::read_to_string(&path).unwrap());
        assert_eq!(m.created, "2020-01-02T03:04:05Z");
        assert_ne!(m.updated, "2020-01-02T03:04:05Z");
        assert!(m.updated > m.created);
        assert_eq!(
            (m.title.as_str(), m.description.as_str(), m.content.as_str()),
            ("New", "", "new text")
        );
    }

    #[test]
    fn index_lists_newest_first_and_forget_rebuilds_it() {
        let d = root();
        let dir = d.path().join(".salam-agent/memory");
        std::fs::create_dir_all(&dir).unwrap();
        std::fs::write(dir.join("old.md"), "---\ntitle: Old\nupdated: 2020-01-01T00:00:00Z\n---\nold").unwrap();
        std::fs::write(dir.join("new.md"), "---\ntitle: New\nupdated: 2025-05-06T07:08:09Z\n---\nnew").unwrap();
        assert_eq!(rebuild_index(d.path()).unwrap(), 2);
        let index = read(&d, ".salam-agent/index.md");
        assert!(index.find("[New]").unwrap() < index.find("[Old]").unwrap());
        assert!(index.contains("- [New](memory/new.md) (updated 2025-05-06 07:08 UTC)"));
        assert_eq!(
            forget(d.path(), "new").unwrap(),
            "Forgot .salam-agent/memory/new.md (1 memory left in .salam-agent/index.md)."
        );
        assert!(!dir.join("new.md").exists());
        let index = read(&d, ".salam-agent/index.md");
        assert!(!index.contains("[New]") && index.contains("[Old]"));
        assert!(forget(d.path(), "new").unwrap_err().contains("does not exist"));
        assert!(forget(d.path(), "../old").is_err());
        forget(d.path(), "old").unwrap();
        assert!(read(&d, ".salam-agent/index.md").ends_with("under memory/.\n\nNo memories yet.\n"));
    }

    #[test]
    fn hand_edited_files_parse_tolerantly() {
        let m = parse(
            "notes",
            "\u{feff}---\r\nTitle:  \"Quoted: title\" \r\nupdated: 2024-01-01T00:00:00Z\r\nextra: ignored\r\nno colon line\r\n---\r\n\r\nBody line\r\n",
        );
        assert_eq!(m.title, "Quoted: title");
        assert_eq!(m.updated, "2024-01-01T00:00:00Z");
        assert_eq!(m.description, "");
        assert_eq!(m.content, "Body line");
        let plain = parse("plain-notes", "# Just text\n\nno frontmatter\n");
        assert_eq!(plain.title, "plain-notes");
        assert_eq!(plain.content, "# Just text\n\nno frontmatter");
        let open = parse("open", "---\ntitle: never closed\n");
        assert_eq!(open.title, "open");
        assert!(open.content.contains("never closed"));
    }

    #[test]
    fn project_context_prefers_agents_md_and_caps_it() {
        let d = root();
        assert_eq!(project_context(d.path()), ProjectContext::default());
        std::fs::write(d.path().join("AGENT.md"), "agent").unwrap();
        let ctx = project_context(d.path());
        assert_eq!((ctx.instructions_file.as_str(), ctx.instructions.as_str()), ("AGENT.md", "agent"));
        let big = "سلام\n".repeat(10_000);
        std::fs::write(d.path().join("AGENTS.md"), &big).unwrap();
        save(d.path(), None, "One", "first", "1").unwrap();
        save(d.path(), None, "Two", "", "2").unwrap();
        let ctx = project_context(d.path());
        assert_eq!(ctx.instructions_file, "AGENTS.md");
        assert!(ctx.instructions.len() <= MAX_INSTRUCTIONS);
        assert!(ctx.instructions.ends_with(&format!("\n{CUT}")));
        assert!(big.starts_with(ctx.instructions.strip_suffix(CUT).unwrap().trim_end_matches('\n')));
        assert_eq!(ctx.memories, 2);
        assert!(ctx.memory_index.contains("[One](memory/one.md) - first"));
        let json = serde_json::to_value(&ctx).unwrap();
        assert!(json.get("instructions_file").is_some() && json.get("memory_index").is_some());
    }
}
