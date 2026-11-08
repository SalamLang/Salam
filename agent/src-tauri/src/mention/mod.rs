pub mod cache;
pub mod index;
pub mod rules;
pub mod search;
pub mod worker;

use index::Entry;
use search::{Cancel, Hit, Outcome};
use serde::Serialize;
use serde_json::json;
use std::collections::HashMap;
use std::path::{Component, Path, PathBuf};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex, RwLock};
use tauri::{AppHandle, Emitter, Manager};

const MAX_ACTIVE: usize = 3;
const LIVE_LIMIT: usize = 5000;

#[derive(Default)]
pub struct Mentions {
    spaces: Mutex<HashMap<String, worker::Handle>>,
}

#[derive(Serialize)]
pub struct Status {
    root: String,
    indexing: bool,
    count: usize,
    revision: u64,
    truncated: bool,
}

#[derive(Serialize)]
pub struct Reply {
    hits: Vec<Hit>,
    indexing: bool,
    count: usize,
    revision: u64,
    stale: bool,
}

type Shared = (Arc<RwLock<index::Index>>, Arc<AtomicU64>, PathBuf);

pub fn canonical_root(root: &str) -> Result<PathBuf, String> {
    let root = root.trim();
    if root.is_empty() {
        return Err("No workspace folder is selected.".to_string());
    }
    let path = dunce::canonicalize(root).map_err(|e| format!("Cannot open the workspace folder: {e}"))?;
    if !path.is_dir() {
        return Err("The workspace is not a folder.".to_string());
    }
    Ok(path)
}

fn cache_dir(app: &AppHandle) -> Option<PathBuf> {
    app.path().app_data_dir().ok().map(|d| d.join("mention-index"))
}

fn status_of(root: &Path, handle: &worker::Handle) -> Status {
    let i = handle.index.read().unwrap();
    Status {
        root: root.to_string_lossy().to_string(),
        indexing: i.scanning,
        count: i.len(),
        revision: i.revision,
        truncated: i.truncated,
    }
}

impl Mentions {
    fn ensure(&self, app: &AppHandle, root: &Path) -> Shared {
        let key = root.to_string_lossy().to_string();
        let mut spaces = self.spaces.lock().unwrap();
        spaces.retain(|_, h| h.alive());
        if let Some(h) = spaces.get(&key) {
            h.touch();
            return (h.index.clone(), h.query_seq.clone(), root.to_path_buf());
        }
        while spaces.len() >= MAX_ACTIVE {
            let Some(oldest) = spaces.iter().max_by_key(|(_, h)| h.idle_ms()).map(|(k, _)| k.clone()) else {
                break;
            };
            if let Some(h) = spaces.remove(&oldest) {
                h.stop();
            }
        }
        let file = cache_dir(app).map_or_else(
            || std::env::temp_dir().join("salam-mention").join("none.idx"),
            |d| cache::file_for(&d, root),
        );
        let emitter = app.clone();
        let emitted = key.clone();
        let handle = worker::spawn(
            root.to_path_buf(),
            file,
            Arc::new(move |revision| {
                let _ = emitter.emit("ws-index-updated", json!({ "root": emitted, "revision": revision }));
            }),
        );
        let out = (handle.index.clone(), handle.query_seq.clone(), root.to_path_buf());
        spaces.insert(key, handle);
        out
    }

    pub fn open(&self, app: &AppHandle, root: &str) -> Result<Status, String> {
        let root = canonical_root(root)?;
        self.ensure(app, &root);
        let spaces = self.spaces.lock().unwrap();
        spaces
            .get(root.to_string_lossy().as_ref())
            .map(|h| status_of(&root, h))
            .ok_or_else(|| "index unavailable".to_string())
    }

    pub fn close(&self, root: &str) {
        let Ok(root) = canonical_root(root) else { return };
        if let Some(h) = self.spaces.lock().unwrap().remove(root.to_string_lossy().as_ref()) {
            h.stop();
        }
    }

    pub fn stop_all(&self) {
        for (_, h) in self.spaces.lock().unwrap().drain() {
            h.stop();
        }
    }
}

fn safe_relative(raw: &str) -> Option<String> {
    let norm = raw.replace('\\', "/");
    let norm = norm.trim_matches('/');
    if norm.is_empty() || norm.contains(':') || norm.contains('\0') {
        return None;
    }
    let mut parts = Vec::new();
    for c in Path::new(norm).components() {
        match c {
            Component::Normal(p) => parts.push(p.to_str()?.to_string()),
            _ => return None,
        }
    }
    Some(parts.join("/"))
}

fn inside(root: &Path, rel: &str) -> Option<PathBuf> {
    let full = root.join(rel);
    let canon = dunce::canonicalize(&full).ok()?;
    canon.starts_with(root).then_some(canon)
}

pub fn live_children(root: &Path, dir: &str, rest: &str, limit: usize) -> Vec<Hit> {
    let Some(dir) = safe_relative(dir) else { return Vec::new() };
    let Some(abs) = inside(root, &dir).filter(|p| p.is_dir()) else {
        return Vec::new();
    };
    let Ok(read) = std::fs::read_dir(&abs) else { return Vec::new() };
    let mut entries = Vec::new();
    for item in read.flatten().take(LIVE_LIMIT) {
        let Some(name) = item.file_name().to_str().map(str::to_string) else {
            continue;
        };
        let Ok(meta) = std::fs::symlink_metadata(item.path()) else {
            continue;
        };
        entries.push(Entry::new(&format!("{dir}/{name}"), index::mtime_of(&meta), meta.is_dir()));
    }
    let scratch = search::scratch_index(entries);
    if rest.is_empty() {
        let mut all: Vec<&Entry> = scratch.entries.iter().collect();
        all.sort_by(|a, b| {
            b.is_dir
                .cmp(&a.is_dir)
                .then_with(|| a.name().to_lowercase().cmp(&b.name().to_lowercase()))
        });
        return all
            .into_iter()
            .take(limit)
            .map(|e| Hit {
                path: e.path.to_string(),
                is_dir: e.is_dir,
                name_at: dir.encode_utf16().count() + 1,
                ranges: Vec::new(),
                score: 0,
            })
            .collect();
    }
    let seq = AtomicU64::new(0);
    match search::search(&scratch, rest, limit, &Cancel { seq: &seq, mine: 0 }) {
        Outcome::Hits(h) => h,
        _ => Vec::new(),
    }
}

fn ignored_top_level(root: &Path, query: &str) -> Option<Hit> {
    let name = query.trim();
    if name.is_empty() || name.contains(['/', '\\', ':', '\0']) || name == "." || name == ".." {
        return None;
    }
    let abs = root.join(name);
    let meta = std::fs::symlink_metadata(&abs).ok()?;
    if !meta.is_dir() || !rules::skip_dir(name, name, &abs) {
        return None;
    }
    Some(Hit {
        path: name.to_string(),
        is_dir: true,
        name_at: 0,
        ranges: vec![(0, name.encode_utf16().count())],
        score: 0,
    })
}

#[tauri::command]
pub async fn ws_index_open(app: AppHandle, root: String) -> Result<Status, String> {
    tauri::async_runtime::spawn_blocking(move || app.state::<Mentions>().open(&app, &root))
        .await
        .map_err(|e| format!("Opening the index failed: {e}"))?
}

#[tauri::command]
pub fn ws_index_close(app: AppHandle, root: String) {
    app.state::<Mentions>().close(&root);
}

#[tauri::command]
pub async fn ws_search(app: AppHandle, root: String, query: String, limit: Option<usize>) -> Result<Reply, String> {
    let root = canonical_root(&root)?;
    let limit = limit.unwrap_or(50);
    tauri::async_runtime::spawn_blocking(move || {
        let (index, seq, root) = app.state::<Mentions>().ensure(&app, &root);
        let mine = seq.fetch_add(1, Ordering::SeqCst) + 1;
        let outcome = {
            let i = index.read().unwrap();
            search::search(&i, &query, limit, &Cancel { seq: &seq, mine })
        };
        let (indexing, count, revision) = {
            let i = index.read().unwrap();
            (i.scanning, i.len(), i.revision)
        };
        let reply = |hits: Vec<Hit>, stale: bool| Reply {
            hits,
            indexing,
            count,
            revision,
            stale,
        };
        Ok(match outcome {
            Outcome::Stale => reply(Vec::new(), true),
            Outcome::UnknownDir(dir, rest) => reply(live_children(&root, &dir, &rest, limit.clamp(1, 100)), false),
            Outcome::Hits(mut hits) => {
                if let Some(top) = ignored_top_level(&root, &query)
                    && !hits.iter().any(|h| h.path == top.path)
                {
                    hits.insert(0, top);
                    hits.truncate(limit.clamp(1, 100));
                }
                reply(hits, false)
            }
        })
    })
    .await
    .map_err(|e| format!("Search failed: {e}"))?
}

#[tauri::command]
pub async fn ws_mention_picked(app: AppHandle, root: String, path: String) -> Result<(), String> {
    let root = canonical_root(&root)?;
    let rel = safe_relative(&path).ok_or_else(|| "invalid path".to_string())?;
    tauri::async_runtime::spawn_blocking(move || {
        let parent = rel.rsplit_once('/').map_or("", |(a, _)| a);
        let inside_root = if parent.is_empty() {
            Some(root.clone())
        } else {
            inside(&root, parent)
        };
        if inside_root.is_none() || std::fs::symlink_metadata(root.join(&rel)).is_err() {
            return Err("that path is not inside the workspace".to_string());
        }
        let (index, _, _) = app.state::<Mentions>().ensure(&app, &root);
        index.write().unwrap().remember(&rel);
        Ok(())
    })
    .await
    .map_err(|e| format!("Saving the pick failed: {e}"))?
}

#[cfg(test)]
mod tests {
    use super::*;
    use index::tests::touch;

    #[test]
    fn relative_paths_cannot_escape() {
        assert_eq!(safe_relative("a/b.txt").as_deref(), Some("a/b.txt"));
        assert_eq!(safe_relative("a\\b/").as_deref(), Some("a/b"));
        for bad in ["", "..", "../x", "a/../../x", "C:/x", "a\0b"] {
            assert!(safe_relative(bad).is_none(), "{bad}");
        }
    }

    #[cfg(unix)]
    #[test]
    fn symlinks_cannot_leave_the_workspace() {
        let tmp = tempfile::tempdir().unwrap();
        let root = dunce::canonicalize(tmp.path()).unwrap().join("ws");
        let outside = dunce::canonicalize(tmp.path()).unwrap().join("outside");
        touch(&root.join("ok/a.txt"));
        touch(&outside.join("secret.txt"));
        std::os::unix::fs::symlink(&outside, root.join("link")).unwrap();
        assert!(inside(&root, "ok").is_some());
        assert!(inside(&root, "link").is_none());
        assert!(live_children(&root, "link", "", 50).is_empty());
        assert_eq!(live_children(&root, "ok", "", 50).len(), 1);
    }

    #[test]
    fn ignored_folders_list_on_demand_without_indexing() {
        let tmp = tempfile::tempdir().unwrap();
        let root = dunce::canonicalize(tmp.path()).unwrap();
        touch(&root.join("vendor/laravel/framework/a.php"));
        touch(&root.join("vendor/autoload.php"));
        touch(&root.join("src/main.rs"));
        let idx = index::tests::build(&root);
        assert!(idx.find("vendor").is_err());
        let seq = AtomicU64::new(1);
        let Outcome::UnknownDir(dir, rest) = search::search(&idx, "vendor/", 50, &Cancel { seq: &seq, mine: 1 }) else {
            panic!()
        };
        let kids = live_children(&root, &dir, &rest, 50);
        let names: Vec<&str> = kids.iter().map(|h| h.path.as_str()).collect();
        assert_eq!(names, ["vendor/laravel", "vendor/autoload.php"]);
        let some = live_children(&root, "vendor", "auto", 50);
        assert_eq!(some[0].path, "vendor/autoload.php");
        assert!(ignored_top_level(&root, "vendor").is_some());
        assert!(ignored_top_level(&root, "src").is_none());
    }
}
