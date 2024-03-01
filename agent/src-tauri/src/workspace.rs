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
}
