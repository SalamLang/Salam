use super::rules;
use ignore::WalkBuilder;
use std::path::Path;
use std::sync::atomic::{AtomicBool, Ordering};
use std::time::UNIX_EPOCH;

pub const MAX_ENTRIES: usize = 300_000;
pub const MAX_DEPTH: usize = 24;
pub const MAX_PATH: usize = 1024;
pub const MAX_MRU: usize = 50;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Entry {
    pub path: Box<str>,
    pub name_at: u16,
    pub mtime: u32,
    pub is_dir: bool,
    pub depth: u8,
}

impl Entry {
    pub fn new(path: &str, mtime: u32, is_dir: bool) -> Entry {
        let name_at = path.rfind('/').map_or(0, |i| i + 1);
        Entry {
            path: path.into(),
            name_at: name_at as u16,
            mtime,
            is_dir,
            depth: (path.bytes().filter(|b| *b == b'/').count() + 1).min(255) as u8,
        }
    }

    pub fn name(&self) -> &str {
        &self.path[self.name_at as usize..]
    }
}

#[derive(Default)]
pub struct Index {
    pub entries: Vec<Entry>,
    pub staging: Vec<Entry>,
    pub mru: Vec<Box<str>>,
    pub revision: u64,
    pub truncated: bool,
    pub scanning: bool,
    pub dirty: bool,
}

pub fn mtime_of(meta: &std::fs::Metadata) -> u32 {
    meta.modified()
        .ok()
        .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
        .map_or(0, |d| d.as_secs().min(u32::MAX as u64) as u32)
}

pub fn dir_mtime(path: &Path) -> Option<u32> {
    std::fs::metadata(path).ok().filter(|m| m.is_dir()).map(|m| mtime_of(&m))
}

fn prefix_of(dir: &str) -> String {
    if dir.is_empty() { String::new() } else { format!("{dir}/") }
}

impl Index {
    pub fn len(&self) -> usize {
        self.entries.len() + self.staging.len()
    }

    pub fn touch(&mut self) {
        self.revision += 1;
        self.dirty = true;
    }

    pub fn find(&self, path: &str) -> Result<usize, usize> {
        self.entries.binary_search_by(|e| (*e.path).cmp(path))
    }

    pub fn is_dir(&self, path: &str) -> bool {
        path.is_empty() || self.find(path).is_ok_and(|i| self.entries[i].is_dir)
    }

    pub fn subtree(&self, dir: &str) -> std::ops::Range<usize> {
        if dir.is_empty() {
            return 0..self.entries.len();
        }
        let prefix = prefix_of(dir);
        let start = self.entries.partition_point(|e| *e.path < *prefix);
        let end = start + self.entries[start..].partition_point(|e| e.path.starts_with(&prefix));
        start..end
    }

    pub fn remove_subtree(&mut self, path: &str) {
        let range = self.subtree(path);
        let had = !range.is_empty();
        self.entries.drain(range);
        if let Ok(i) = self.find(path) {
            self.entries.remove(i);
            self.touch();
        } else if had {
            self.touch();
        }
    }

    pub fn insert_many(&mut self, mut fresh: Vec<Entry>) {
        if fresh.is_empty() {
            return;
        }
        fresh.append(&mut self.entries);
        fresh.sort_by(|a, b| a.path.cmp(&b.path));
        fresh.dedup_by(|later, earlier| later.path == earlier.path);
        self.entries = fresh;
        self.touch();
    }

    pub fn dirs(&self) -> Vec<(Box<str>, u32)> {
        self.entries
            .iter()
            .filter(|e| e.is_dir)
            .map(|e| (e.path.clone(), e.mtime))
            .collect()
    }

    pub fn set_mtime(&mut self, path: &str, mtime: u32) {
        if let Ok(i) = self.find(path)
            && self.entries[i].mtime != mtime
        {
            self.entries[i].mtime = mtime;
            self.touch();
        }
    }

    pub fn finish_scan(&mut self) {
        let staged = std::mem::take(&mut self.staging);
        self.insert_many(staged);
        self.scanning = false;
        self.touch();
    }

    pub fn direct_children(&self, dir: &str) -> Vec<usize> {
        let depth = if dir.is_empty() {
            1
        } else {
            dir.bytes().filter(|b| *b == b'/').count() + 2
        };
        self.subtree(dir).filter(|i| self.entries[*i].depth as usize == depth).collect()
    }

    pub fn apply_dir_diff(&mut self, dir: &str, fresh: Vec<Entry>, mtime: Option<u32>) -> Vec<String> {
        let mut new_dirs = Vec::new();
        let existing = self.direct_children(dir);
        let mut gone: Vec<(Box<str>, bool)> = Vec::new();
        let mut updates: Vec<(Box<str>, u32)> = Vec::new();
        let by_path: std::collections::HashMap<&str, &Entry> = fresh.iter().map(|f| (&*f.path, f)).collect();
        for i in existing {
            let old = &self.entries[i];
            match by_path.get(&*old.path) {
                Some(f) if f.is_dir == old.is_dir => {
                    if f.mtime != old.mtime && !f.is_dir {
                        updates.push((f.path.clone(), f.mtime));
                    }
                }
                _ => gone.push((old.path.clone(), old.is_dir)),
            }
        }
        drop(by_path);
        for (path, is_dir) in gone {
            if is_dir {
                self.remove_subtree(&path);
            } else if let Ok(i) = self.find(&path) {
                self.entries.remove(i);
                self.touch();
            }
        }
        for (path, m) in updates {
            self.set_mtime(&path, m);
        }
        let mut added = Vec::new();
        for f in fresh {
            if self.find(&f.path).is_err() {
                if f.is_dir {
                    new_dirs.push(f.path.to_string());
                }
                added.push(f);
            }
        }
        self.insert_many(added);
        if let (Some(m), false) = (mtime, dir.is_empty()) {
            self.set_mtime(dir, m);
        }
        new_dirs
    }

    pub fn remember(&mut self, path: &str) {
        self.mru.retain(|p| &**p != path);
        self.mru.insert(0, path.into());
        self.mru.truncate(MAX_MRU);
        self.touch();
    }
}

pub fn rel_string(root: &Path, abs: &Path) -> Option<String> {
    let rel = abs.strip_prefix(root).ok()?;
    let s = rel.to_str()?;
    if cfg!(windows) {
        Some(s.replace('\\', "/"))
    } else {
        Some(s.to_string())
    }
}

pub fn walker(root: &Path, rel: &str, max_rel_depth: Option<usize>) -> ignore::Walk {
    let start = if rel.is_empty() { root.to_path_buf() } else { root.join(rel) };
    let base = if rel.is_empty() { 0 } else { rel.split('/').count() };
    let limit = MAX_DEPTH.saturating_sub(base);
    let depth = max_rel_depth.map_or(limit, |d| d.min(limit));
    let filter_root = root.to_path_buf();
    WalkBuilder::new(start)
        .standard_filters(false)
        .git_ignore(true)
        .git_exclude(true)
        .git_global(false)
        .ignore(true)
        .hidden(false)
        .parents(true)
        .require_git(false)
        .follow_links(false)
        .max_depth(Some(depth))
        .filter_entry(move |e| {
            if e.depth() == 0 {
                return true;
            }
            let name = e.file_name().to_string_lossy();
            if e.file_type().is_some_and(|t| t.is_dir()) {
                let rel = rel_string(&filter_root, e.path()).unwrap_or_default();
                !rules::skip_dir(&rel, &name, e.path())
            } else {
                !rules::skip_file(&name)
            }
        })
        .build()
}

pub fn walk(root: &Path, rel: &str, max_rel_depth: Option<usize>, stop: &AtomicBool, budget: usize, sink: &mut dyn FnMut(Entry)) -> bool {
    let mut count = 0usize;
    for item in walker(root, rel, max_rel_depth) {
        if stop.load(Ordering::Relaxed) {
            return false;
        }
        let Ok(item) = item else { continue };
        if item.depth() == 0 {
            continue;
        }
        let Some(path) = rel_string(root, item.path()) else { continue };
        if path.len() > MAX_PATH {
            continue;
        }
        let Ok(meta) = std::fs::symlink_metadata(item.path()) else {
            continue;
        };
        let is_dir = meta.is_dir();
        sink(Entry::new(&path, mtime_of(&meta), is_dir));
        count += 1;
        if count >= budget {
            return true;
        }
    }
    false
}

#[cfg(test)]
pub fn walk_vec(root: &Path, rel: &str, max_rel_depth: Option<usize>) -> Vec<Entry> {
    let stop = AtomicBool::new(false);
    let mut out = Vec::new();
    walk(root, rel, max_rel_depth, &stop, MAX_ENTRIES, &mut |e| out.push(e));
    out
}

#[cfg(test)]
pub mod tests {
    use super::*;
    use std::fs;

    pub fn touch(p: &Path) {
        fs::create_dir_all(p.parent().unwrap()).unwrap();
        fs::write(p, "x").unwrap();
    }

    pub fn paths(index: &Index) -> Vec<String> {
        index.entries.iter().map(|e| e.path.to_string()).collect()
    }

    pub fn build(root: &Path) -> Index {
        let mut index = Index::default();
        index.entries = walk_vec(root, "", None);
        index.entries.sort_by(|a, b| a.path.cmp(&b.path));
        index
    }

    #[test]
    fn walk_respects_ignore_files_and_builtin_rules() {
        let tmp = tempfile::tempdir().unwrap();
        let r = tmp.path();
        touch(&r.join("src/main.rs"));
        touch(&r.join("src/vendor-utils/a.rs"));
        touch(&r.join("vendor/lib/x.php"));
        touch(&r.join("packages/app/vendor/y.php"));
        touch(&r.join("apps/web/node_modules/z/index.js"));
        touch(&r.join("secret/key.txt"));
        touch(&r.join("notes.log"));
        touch(&r.join("docs/a.zip"));
        touch(&r.join(".gitignore"));
        fs::write(r.join(".gitignore"), "secret/\n*.log\n").unwrap();
        let p = paths(&build(r));
        assert!(p.contains(&"src/main.rs".to_string()));
        assert!(p.contains(&"src/vendor-utils/a.rs".to_string()));
        assert!(p.contains(&".gitignore".to_string()));
        for gone in [
            "vendor",
            "vendor/lib/x.php",
            "packages/app/vendor",
            "apps/web/node_modules",
            "secret",
            "notes.log",
            "docs/a.zip",
        ] {
            assert!(!p.contains(&gone.to_string()), "{gone} must not be indexed");
        }
        assert!(p.contains(&"packages/app".to_string()));
    }

    #[test]
    fn symlink_loops_are_not_followed() {
        #[cfg(unix)]
        {
            let tmp = tempfile::tempdir().unwrap();
            let r = tmp.path();
            touch(&r.join("a/f.txt"));
            std::os::unix::fs::symlink(r, r.join("a/loop")).unwrap();
            let p = paths(&build(r));
            assert!(p.contains(&"a/loop".to_string()));
            assert!(!p.iter().any(|x| x.starts_with("a/loop/")));
            assert!(!build(r).entries.iter().find(|e| &*e.path == "a/loop").unwrap().is_dir);
        }
    }

    fn refresh(index: &mut Index, root: &Path, dir: &str) {
        let fresh = walk_vec(&root, dir, Some(1));
        let m = dir_mtime(&root.join(dir));
        for d in index.apply_dir_diff(dir, fresh, m) {
            let sub = walk_vec(&root, &d, None);
            index.insert_many(sub);
        }
    }

    #[test]
    fn create_delete_and_rename_update_incrementally() {
        let tmp = tempfile::tempdir().unwrap();
        let r = tmp.path();
        touch(&r.join("a/one.txt"));
        touch(&r.join("a/sub/two.txt"));
        touch(&r.join("b/three.txt"));
        let mut index = build(r);
        touch(&r.join("a/new/deep/four.txt"));
        refresh(&mut index, r, "a");
        let p = paths(&index);
        for want in ["a/new", "a/new/deep", "a/new/deep/four.txt"] {
            assert!(p.contains(&want.to_string()), "{want}");
        }
        fs::rename(r.join("a"), r.join("c")).unwrap();
        refresh(&mut index, r, "");
        let p = paths(&index);
        assert!(!p.iter().any(|x| x == "a" || x.starts_with("a/")));
        for want in ["c", "c/one.txt", "c/sub/two.txt", "c/new/deep/four.txt", "b/three.txt"] {
            assert!(p.contains(&want.to_string()), "{want}");
        }
        fs::remove_dir_all(r.join("c/sub")).unwrap();
        fs::remove_file(r.join("c/one.txt")).unwrap();
        refresh(&mut index, r, "c");
        let p = paths(&index);
        assert!(!p.contains(&"c/sub".to_string()) && !p.contains(&"c/sub/two.txt".to_string()) && !p.contains(&"c/one.txt".to_string()));
        let sorted = {
            let mut s = p.clone();
            s.sort();
            s
        };
        assert_eq!(p, sorted);
        assert_eq!(p.iter().filter(|x| *x == "c").count(), 1);
    }

    #[test]
    fn subtree_ignores_lookalike_names() {
        let mut index = Index::default();
        for (p, d) in [
            ("a", true),
            ("a-b", false),
            ("a.txt", false),
            ("a/x", false),
            ("ab", true),
            ("ab/y", false),
        ] {
            index.entries.push(Entry::new(p, 0, d));
        }
        index.entries.sort_by(|a, b| a.path.cmp(&b.path));
        index.remove_subtree("a");
        assert_eq!(paths(&index), ["a-b", "a.txt", "ab", "ab/y"]);
    }
}
