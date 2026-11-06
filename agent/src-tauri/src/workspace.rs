use std::path::{Component, Path, PathBuf};

pub struct Workspace {
    root: PathBuf,
}

pub struct Resolved {
    pub path: PathBuf,
    pub inside: bool,
}

fn normalize(path: &Path) -> PathBuf {
    let mut out = PathBuf::new();
    for c in path.components() {
        match c {
            Component::ParentDir => {
                if !out.pop() {
                    out.push("..");
                }
            }
            Component::CurDir => {}
            other => out.push(other.as_os_str()),
        }
    }
    out
}

fn canonical_prefix(path: &Path) -> PathBuf {
    let mut existing = path.to_path_buf();
    let mut rest: Vec<std::ffi::OsString> = Vec::new();
    loop {
        if let Ok(c) = dunce::canonicalize(&existing) {
            let mut out = c;
            for part in rest.iter().rev() {
                out.push(part);
            }
            return out;
        }
        match (existing.file_name().map(|n| n.to_os_string()), existing.parent()) {
            (Some(name), Some(parent)) => {
                rest.push(name);
                existing = parent.to_path_buf();
            }
            _ => return path.to_path_buf(),
        }
    }
}

impl Workspace {
    pub fn open(root: &str) -> Result<Self, String> {
        let root = dunce::canonicalize(root).map_err(|_| format!("The workspace folder {root} does not exist on this computer."))?;
        if !root.is_dir() {
            return Err(format!("The workspace {} is not a folder.", root.display()));
        }
        Ok(Workspace { root })
    }

    pub fn root(&self) -> &Path {
        &self.root
    }

    pub fn resolve(&self, rel: &str) -> Resolved {
        let rel = rel.trim();
        let joined = if rel.is_empty() || rel == "." {
            self.root.clone()
        } else {
            let p = Path::new(rel);
            if p.is_absolute() { p.to_path_buf() } else { self.root.join(p) }
        };
        let path = canonical_prefix(&normalize(&joined));
        let inside = path.starts_with(&self.root);
        Resolved { path, inside }
    }

    pub fn display(&self, path: &Path) -> String {
        match path.strip_prefix(&self.root) {
            Ok(r) if r.as_os_str().is_empty() => ".".to_string(),
            Ok(r) => r.to_string_lossy().replace('\\', "/"),
            Err(_) => path.to_string_lossy().to_string(),
        }
    }
}

pub fn in_git_dir(path: &Path) -> bool {
    path.components().any(|c| c.as_os_str() == ".git")
}

const TOKEN_BREAK: &[char] = &[
    ' ', '\t', '\n', '\r', '"', '\'', '`', ';', '|', '&', '<', '>', '(', ')', '{', '}', '[', ']', ',', '=',
];

enum Target {
    Path(PathBuf),
    Unknown,
}

fn home_dir() -> Option<PathBuf> {
    #[cfg(windows)]
    let home = std::env::var_os("USERPROFILE").or_else(|| {
        let drive = std::env::var_os("HOMEDRIVE")?;
        let path = std::env::var_os("HOMEPATH")?;
        let mut joined = PathBuf::from(drive);
        joined.push(path);
        Some(joined.into_os_string())
    });
    #[cfg(not(windows))]
    let home = std::env::var_os("HOME");
    home.filter(|v| !v.is_empty()).map(PathBuf::from)
}

fn join_rest(mut base: PathBuf, rest: &str) -> PathBuf {
    for part in rest.split(['/', '\\']).filter(|p| !p.is_empty()) {
        base.push(part);
    }
    base
}

fn env_target(token: &str) -> Option<Target> {
    let body = token.strip_prefix('$')?;
    let (name, tail) = if let Some(inner) = body.strip_prefix('{') {
        let (name, tail) = inner.split_once('}')?;
        (name, tail)
    } else {
        let end = body.find(|c: char| !(c.is_ascii_alphanumeric() || c == '_')).unwrap_or(body.len());
        (&body[..end], &body[end..])
    };
    if name.is_empty() {
        return None;
    }
    let as_path = tail.starts_with('/') || tail.starts_with('\\');
    if !as_path && !(tail.is_empty() && matches!(name, "HOME" | "USERPROFILE" | "TMPDIR" | "TEMP" | "TMP")) {
        return None;
    }
    match std::env::var_os(name) {
        Some(value) if !value.is_empty() => Some(Target::Path(join_rest(PathBuf::from(value), tail))),
        _ => Some(Target::Unknown),
    }
}

fn target(token: &str, cwd: &Path) -> Option<Target> {
    if token.is_empty() || token == "-" || token.contains("://") {
        return None;
    }
    if let Some(rest) = token.strip_prefix('~') {
        if !rest.is_empty() && !(rest.starts_with('/') || rest.starts_with('\\')) {
            return Some(Target::Unknown);
        }
        return Some(Target::Path(join_rest(home_dir()?, rest)));
    }
    if token.starts_with('$') {
        return env_target(token);
    }
    if token.starts_with('-') {
        if let Some(i) = token.find(['/', '\\']) {
            let path = Path::new(&token[i..]);
            if path.is_absolute() {
                return Some(Target::Path(path.to_path_buf()));
            }
        }
    }
    let path = Path::new(token);
    if path.is_absolute() {
        return Some(Target::Path(path.to_path_buf()));
    }
    if token.contains('/') || token.contains('\\') {
        return Some(Target::Path(cwd.join(path)));
    }
    None
}

impl Workspace {
    /// Host paths named in a command that live outside the workspace.
    ///
    /// Approval decisions use this for shell, docker and WSL calls, so a
    /// command such as `cat /tmp/report` asks like `read_file` would, and a
    /// relative path through a symlink out of the workspace is caught too.
    pub fn outside_paths(&self, command: &str, cwd: &Path) -> Vec<String> {
        let mut out = Vec::new();
        for token in command.split(|c: char| TOKEN_BREAK.contains(&c)) {
            let token = token.trim_matches(|c: char| c == '\'' || c == '"' || c == '`');
            match target(token, cwd) {
                Some(Target::Unknown) => out.push(token.to_string()),
                Some(Target::Path(p)) if !canonical_prefix(&normalize(&p)).starts_with(&self.root) => {
                    out.push(token.to_string());
                }
                _ => {}
            }
        }
        out.sort();
        out.dedup();
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn setup() -> (tempfile::TempDir, Workspace) {
        let dir = tempfile::tempdir().unwrap();
        std::fs::create_dir_all(dir.path().join("src")).unwrap();
        std::fs::write(dir.path().join("src/main.rs"), "fn main() {}").unwrap();
        let ws = Workspace::open(dir.path().to_str().unwrap()).unwrap();
        (dir, ws)
    }

    #[test]
    fn relative_paths_stay_inside() {
        let (_d, ws) = setup();
        let r = ws.resolve("src/main.rs");
        assert!(r.inside);
        assert_eq!(ws.display(&r.path), "src/main.rs");
        assert!(ws.resolve("").inside);
        assert!(ws.resolve("src/new/deep/file.txt").inside);
    }

    #[test]
    fn parent_escapes_are_outside() {
        let (_d, ws) = setup();
        assert!(!ws.resolve("../secret.txt").inside);
        assert!(!ws.resolve("src/../../x").inside);
        assert!(ws.resolve("src/../src/main.rs").inside);
    }

    #[test]
    fn absolute_paths_outside_are_flagged() {
        let (_d, ws) = setup();
        let other = tempfile::tempdir().unwrap();
        assert!(!ws.resolve(other.path().to_str().unwrap()).inside);
        let inside = ws.root().join("src");
        assert!(ws.resolve(inside.to_str().unwrap()).inside);
    }

    #[cfg(unix)]
    #[test]
    fn symlinks_out_of_the_workspace_are_outside() {
        let (d, ws) = setup();
        let other = tempfile::tempdir().unwrap();
        std::fs::write(other.path().join("x.txt"), "x").unwrap();
        std::os::unix::fs::symlink(other.path(), d.path().join("link")).unwrap();
        assert!(!ws.resolve("link/x.txt").inside);
        assert!(!ws.resolve("link/new.txt").inside);
    }

    #[test]
    fn missing_workspace_is_an_error() {
        assert!(Workspace::open("/definitely/not/here/salam").is_err());
    }

    #[test]
    fn git_internals_are_detected() {
        assert!(in_git_dir(Path::new("/w/.git/config")));
        assert!(!in_git_dir(Path::new("/w/.github/ci.yml")));
    }

    #[test]
    fn commands_naming_outside_paths_are_found() {
        let (_d, ws) = setup();
        let other = tempfile::tempdir().unwrap();
        let secret = other.path().join("secret.txt");
        std::fs::write(&secret, "s").unwrap();

        assert_eq!(ws.outside_paths(&format!("cat {}", secret.display()), ws.root()).len(), 1);
        assert_eq!(ws.outside_paths("cat ../secret.txt", ws.root()).len(), 1);
        assert!(ws.outside_paths("cat src/main.rs", ws.root()).is_empty());
        assert!(ws.outside_paths("cat ./src/../src/main.rs", ws.root()).is_empty());
        assert!(ws.outside_paths("curl https://example.com/a -o src/out.txt", ws.root()).is_empty());
        let inside = ws.root().join("src/new.txt");
        assert!(ws.outside_paths(&format!("echo hi > {}", inside.display()), ws.root()).is_empty());
        assert_eq!(ws.outside_paths(&format!("echo hi > {}", secret.display()), ws.root()).len(), 1);
    }

    #[cfg(unix)]
    #[test]
    fn commands_through_a_symlink_out_are_found() {
        let (d, ws) = setup();
        let other = tempfile::tempdir().unwrap();
        std::fs::write(other.path().join("x.txt"), "x").unwrap();
        std::os::unix::fs::symlink(other.path(), d.path().join("link")).unwrap();
        assert_eq!(ws.outside_paths("cat link/x.txt", ws.root()).len(), 1);
    }

    #[test]
    fn home_and_temp_variables_are_resolved() {
        let (_d, ws) = setup();
        if home_dir().is_some_and(|h| !ws.root().starts_with(&h)) {
            assert_eq!(ws.outside_paths("cat $HOME/x", ws.root()).len(), 1);
            assert_eq!(ws.outside_paths("cat ~/x", ws.root()).len(), 1);
        }
        assert!(ws.outside_paths("echo $PATH", ws.root()).is_empty());
    }

    #[cfg(unix)]
    #[test]
    fn attached_option_paths_are_found() {
        let (_d, ws) = setup();
        assert_eq!(ws.outside_paths("gcc -I/tmp/inc -c a.c", ws.root()).len(), 1);
        assert_eq!(ws.outside_paths("gcc -o/tmp/a.out -c a.c", ws.root()).len(), 1);
        assert!(ws.outside_paths("gcc -Isrc -c a.c", ws.root()).is_empty());
    }
}
