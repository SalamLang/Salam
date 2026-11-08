use super::cache;
use super::index::{self, Index, MAX_ENTRIES};
use notify::{Config, EventKind, RecommendedWatcher, RecursiveMode, Watcher};
use std::collections::HashSet;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::mpsc::{Receiver, RecvTimeoutError, Sender, channel};
use std::sync::{Arc, RwLock};
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

const DEBOUNCE: Duration = Duration::from_millis(250);
const MAX_WAIT: Duration = Duration::from_secs(2);
const SAVE_AFTER: Duration = Duration::from_secs(10);
const RECONCILE_EVERY: Duration = Duration::from_secs(90);
const IDLE_LIMIT: Duration = Duration::from_secs(20 * 60);
const EMIT_EVERY: Duration = Duration::from_millis(300);
const MAX_WATCHES: usize = 4096;

pub type Notifier = Arc<dyn Fn(u64) + Send + Sync>;

enum Msg {
    Fs(notify::Result<notify::Event>),
    Stop,
}

pub struct Handle {
    tx: Sender<Msg>,
    pub index: Arc<RwLock<Index>>,
    pub query_seq: Arc<AtomicU64>,
    stop: Arc<AtomicBool>,
    used: Arc<AtomicU64>,
    alive: Arc<AtomicBool>,
}

fn now_ms() -> u64 {
    SystemTime::now().duration_since(UNIX_EPOCH).map_or(0, |d| d.as_millis() as u64)
}

impl Handle {
    pub fn touch(&self) {
        self.used.store(now_ms(), Ordering::Relaxed);
    }

    pub fn idle_ms(&self) -> u64 {
        now_ms().saturating_sub(self.used.load(Ordering::Relaxed))
    }

    pub fn alive(&self) -> bool {
        self.alive.load(Ordering::Relaxed)
    }

    pub fn stop(&self) {
        self.stop.store(true, Ordering::Relaxed);
        let _ = self.tx.send(Msg::Stop);
    }

}

struct Guard(Arc<AtomicBool>);

impl Drop for Guard {
    fn drop(&mut self) {
        self.0.store(false, Ordering::Relaxed);
    }
}

pub fn spawn(root: PathBuf, cache_file: PathBuf, notifier: Notifier) -> Handle {
    let (tx, rx) = channel();
    let handle = Handle {
        tx: tx.clone(),
        index: Arc::new(RwLock::new(Index::default())),
        query_seq: Arc::new(AtomicU64::new(0)),
        stop: Arc::new(AtomicBool::new(false)),
        used: Arc::new(AtomicU64::new(now_ms())),
        alive: Arc::new(AtomicBool::new(true)),
    };
    let mut worker = Worker {
        root,
        cache_file,
        index: handle.index.clone(),
        stop: handle.stop.clone(),
        used: handle.used.clone(),
        notifier,
        watcher: None,
        watched: HashSet::new(),
        last_emit: Instant::now() - EMIT_EVERY,
        saved_revision: 0,
        changed_at: Instant::now(),
    };
    let guard = Guard(handle.alive.clone());
    let spawned = std::thread::Builder::new().name("mention-index".into()).spawn(move || {
        let _guard = guard;
        worker.run(rx, tx);
    });
    if spawned.is_err() {
        handle.alive.store(false, Ordering::Relaxed);
    }
    handle
}

struct Worker {
    root: PathBuf,
    cache_file: PathBuf,
    index: Arc<RwLock<Index>>,
    stop: Arc<AtomicBool>,
    used: Arc<AtomicU64>,
    notifier: Notifier,
    watcher: Option<RecommendedWatcher>,
    watched: HashSet<String>,
    last_emit: Instant,
    saved_revision: u64,
    changed_at: Instant,
}

pub fn reconcile(index: &RwLock<Index>, root: &Path, stop: &AtomicBool) -> usize {
    let mut walked = 0;
    let mut todo: Vec<(String, Option<u32>)> = vec![(String::new(), None)];
    todo.extend(index.read().unwrap().dirs().into_iter().map(|(p, m)| (p.to_string(), Some(m))));
    for (dir, recorded) in todo {
        if stop.load(Ordering::Relaxed) {
            break;
        }
        if !dir.is_empty() && !index.read().unwrap().is_dir(&dir) {
            continue;
        }
        let abs = if dir.is_empty() { root.to_path_buf() } else { root.join(&dir) };
        let Some(mtime) = index::dir_mtime(&abs) else {
            if !dir.is_empty() {
                index.write().unwrap().remove_subtree(&dir);
            }
            continue;
        };
        if recorded == Some(mtime) {
            continue;
        }
        walked += 1;
        refresh(index, root, &dir, stop, &mut |_| {});
    }
    walked
}

fn refresh(index: &RwLock<Index>, root: &Path, dir: &str, stop: &AtomicBool, on_new_dir: &mut dyn FnMut(&str)) {
    let abs = if dir.is_empty() { root.to_path_buf() } else { root.join(dir) };
    let Some(mtime) = index::dir_mtime(&abs) else {
        if !dir.is_empty() {
            index.write().unwrap().remove_subtree(dir);
        }
        return;
    };
    let mut fresh = Vec::new();
    index::walk(root, dir, Some(1), stop, MAX_ENTRIES, &mut |e| fresh.push(e));
    if stop.load(Ordering::Relaxed) {
        return;
    }
    let new_dirs = index.write().unwrap().apply_dir_diff(dir, fresh, Some(mtime));
    for d in new_dirs {
        on_new_dir(&d);
        let room = MAX_ENTRIES.saturating_sub(index.read().unwrap().len());
        let mut sub = Vec::new();
        index::walk(root, &d, None, stop, room, &mut |e| sub.push(e));
        for e in sub.iter().filter(|e| e.is_dir) {
            on_new_dir(&e.path);
        }
        index.write().unwrap().insert_many(sub);
    }
}

impl Worker {
    fn emit(&mut self, force: bool) {
        if force || self.last_emit.elapsed() >= EMIT_EVERY {
            self.last_emit = Instant::now();
            let revision = self.index.read().unwrap().revision;
            (self.notifier)(revision);
        }
    }

    fn save(&mut self) {
        let (revision, dirty) = {
            let i = self.index.read().unwrap();
            (i.revision, i.dirty && !i.scanning)
        };
        if !dirty {
            return;
        }
        let result = {
            let i = self.index.read().unwrap();
            cache::save(&self.cache_file, &self.root.to_string_lossy(), &i)
        };
        if result.is_ok() {
            self.saved_revision = revision;
            let mut i = self.index.write().unwrap();
            if i.revision == revision {
                i.dirty = false;
            }
        }
    }

    fn initial(&mut self) {
        let loaded = cache::load(&self.cache_file, &self.root.to_string_lossy());
        {
            let mut i = self.index.write().unwrap();
            i.scanning = true;
            if let Some(l) = &loaded {
                i.entries = l.entries.clone();
                i.mru = l.mru.clone();
            }
            i.touch();
        }
        self.emit(true);
        if loaded.is_some() {
            reconcile(&self.index, &self.root, &self.stop);
            let mut i = self.index.write().unwrap();
            i.scanning = false;
            i.touch();
        } else {
            let index = self.index.clone();
            let mut batch: Vec<index::Entry> = Vec::with_capacity(2000);
            let notifier = self.notifier.clone();
            let mut last = Instant::now();
            let truncated = index::walk(&self.root, "", None, &self.stop, MAX_ENTRIES, &mut |e| {
                batch.push(e);
                if batch.len() >= 2000 {
                    let mut i = index.write().unwrap();
                    i.staging.append(&mut batch);
                    i.revision += 1;
                    if last.elapsed() >= EMIT_EVERY {
                        last = Instant::now();
                        let r = i.revision;
                        drop(i);
                        notifier(r);
                    }
                }
            });
            let mut i = self.index.write().unwrap();
            i.staging.append(&mut batch);
            i.truncated = truncated;
            i.finish_scan();
        }
        self.emit(true);
    }

    fn watch_all(&mut self) {
        if self.stop.load(Ordering::Relaxed) {
            return;
        }
        let dirs: Vec<String> = self.index.read().unwrap().dirs().into_iter().map(|d| d.0.to_string()).collect();
        self.watch_dir("");
        for d in dirs {
            self.watch_dir(&d);
        }
    }

    fn watch_dir(&mut self, rel: &str) {
        if self.watched.len() >= MAX_WATCHES || self.watched.contains(rel) {
            return;
        }
        let Some(w) = self.watcher.as_mut() else { return };
        let abs = if rel.is_empty() { self.root.clone() } else { self.root.join(rel) };
        if w.watch(&abs, RecursiveMode::NonRecursive).is_ok() {
            self.watched.insert(rel.to_string());
        }
    }

    fn prune_watches(&mut self) {
        let gone: Vec<String> = {
            let i = self.index.read().unwrap();
            self.watched.iter().filter(|w| !i.is_dir(w)).cloned().collect()
        };
        for g in gone {
            if let Some(w) = self.watcher.as_mut() {
                let _ = w.unwatch(&self.root.join(&g));
            }
            self.watched.remove(&g);
        }
    }

    fn apply(&mut self, events: Vec<notify::Result<notify::Event>>) {
        let mut rescan = false;
        let mut dirty: HashSet<String> = HashSet::new();
        let mut touched: HashSet<String> = HashSet::new();
        for ev in events {
            let ev = match ev {
                Ok(ev) => ev,
                Err(_) => {
                    rescan = true;
                    continue;
                }
            };
            if ev.need_rescan() {
                rescan = true;
            }
            let content = matches!(ev.kind, EventKind::Modify(notify::event::ModifyKind::Data(_) | notify::event::ModifyKind::Metadata(_)));
            if matches!(ev.kind, EventKind::Access(_)) {
                continue;
            }
            for p in &ev.paths {
                let Some(rel) = index::rel_string(&self.root, p) else { continue };
                if rel.is_empty() {
                    continue;
                }
                if rel.ends_with(".gitignore") || rel.ends_with(".ignore") {
                    rescan = true;
                }
                let parent = rel.rsplit_once('/').map_or("", |(a, _)| a).to_string();
                if content {
                    touched.insert(rel);
                } else {
                    dirty.insert(parent);
                }
            }
        }
        if rescan {
            dirty.clear();
            touched.clear();
            reconcile(&self.index, &self.root, &self.stop);
            dirty.insert(String::new());
            self.full_refresh();
            return;
        }
        {
            let i = self.index.read().unwrap();
            dirty.retain(|d| i.is_dir(d));
        }
        for t in touched {
            let abs = self.root.join(&t);
            match std::fs::symlink_metadata(&abs) {
                Ok(m) => {
                    if self.index.read().unwrap().find(&t).is_ok() {
                        self.index.write().unwrap().set_mtime(&t, index::mtime_of(&m));
                    } else {
                        let parent = t.rsplit_once('/').map_or("", |(a, _)| a).to_string();
                        if self.index.read().unwrap().is_dir(&parent) {
                            dirty.insert(parent);
                        }
                    }
                }
                Err(_) => {
                    let parent = t.rsplit_once('/').map_or("", |(a, _)| a).to_string();
                    if self.index.read().unwrap().is_dir(&parent) {
                        dirty.insert(parent);
                    }
                }
            }
        }
        let mut added: Vec<String> = Vec::new();
        for d in dirty {
            let index = self.index.clone();
            refresh(&index, &self.root, &d, &self.stop, &mut |n| added.push(n.to_string()));
        }
        self.after_change(added);
    }

    fn full_refresh(&mut self) {
        let mut added = Vec::new();
        let index = self.index.clone();
        refresh(&index, &self.root, "", &self.stop, &mut |n| added.push(n.to_string()));
        self.after_change(added);
    }

    fn after_change(&mut self, added: Vec<String>) {
        for a in added {
            self.watch_dir(&a);
        }
        self.prune_watches();
        self.changed_at = Instant::now();
        self.emit(true);
    }

    fn run(&mut self, rx: Receiver<Msg>, tx: Sender<Msg>) {
        self.initial();
        if self.stop.load(Ordering::Relaxed) {
            return;
        }
        let events = tx.clone();
        self.watcher = RecommendedWatcher::new(
            move |res| {
                let _ = events.send(Msg::Fs(res));
            },
            Config::default(),
        )
        .ok();
        self.watch_all();
        let mut pending: Vec<notify::Result<notify::Event>> = Vec::new();
        let mut first_at = Instant::now();
        let mut last_at = Instant::now();
        let mut next_reconcile = Instant::now() + RECONCILE_EVERY;
        loop {
            let now = Instant::now();
            let dirty = {
                let i = self.index.read().unwrap();
                i.dirty
            };
            let mut wait = next_reconcile.saturating_duration_since(now);
            if !pending.is_empty() {
                let due = (last_at + DEBOUNCE).min(first_at + MAX_WAIT);
                wait = wait.min(due.saturating_duration_since(now));
            } else if dirty {
                wait = wait.min((self.changed_at + SAVE_AFTER).saturating_duration_since(now));
            }
            wait = wait.min(Duration::from_secs(60));
            match rx.recv_timeout(wait) {
                Ok(Msg::Stop) | Err(RecvTimeoutError::Disconnected) => break,
                Ok(Msg::Fs(ev)) => {
                    if pending.is_empty() {
                        first_at = Instant::now();
                    }
                    last_at = Instant::now();
                    pending.push(ev);
                    if pending.len() < 4096 {
                        continue;
                    }
                }
                Err(RecvTimeoutError::Timeout) => {}
            }
            let now = Instant::now();
            if !pending.is_empty() && (now >= last_at + DEBOUNCE || now >= first_at + MAX_WAIT || pending.len() >= 4096) {
                let batch = std::mem::take(&mut pending);
                self.apply(batch);
            }
            if now >= next_reconcile {
                next_reconcile = now + RECONCILE_EVERY;
                if reconcile(&self.index, &self.root, &self.stop) > 0 {
                    self.prune_watches();
                    self.watch_all();
                    self.changed_at = Instant::now();
                    self.emit(true);
                }
            }
            if self.index.read().unwrap().dirty && self.changed_at.elapsed() >= SAVE_AFTER {
                self.save();
            }
            if Duration::from_millis(now_ms().saturating_sub(self.used.load(Ordering::Relaxed))) >= IDLE_LIMIT {
                break;
            }
        }
        self.watcher = None;
        if !self.stop.load(Ordering::Relaxed) || self.index.read().unwrap().dirty {
            self.save();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::super::cache;
    use super::super::index::tests::{build, paths, touch};
    use super::*;
    use std::fs;

    #[test]
    fn reconcile_after_cache_load_walks_only_changed_dirs() {
        let tmp = tempfile::tempdir().unwrap();
        let root = tmp.path().join("ws");
        touch(&root.join("keep/one.txt"));
        touch(&root.join("keep/inner/x.txt"));
        touch(&root.join("change/two.txt"));
        let saved = build(&root);
        let file = cache::file_for(tmp.path(), &root);
        cache::save(&file, "r", &saved).unwrap();
        std::thread::sleep(Duration::from_millis(1100));
        touch(&root.join("change/three.txt"));
        fs::remove_file(root.join("change/two.txt")).unwrap();
        touch(&root.join("fresh/deep/new.txt"));
        let loaded = cache::load(&file, "r").unwrap();
        let mut index = Index::default();
        index.entries = loaded.entries;
        let lock = RwLock::new(index);
        let walked = reconcile(&lock, &root, &AtomicBool::new(false));
        assert_eq!(walked, 2, "only the root and the changed directory are walked");
        let p = paths(&lock.read().unwrap());
        assert!(p.contains(&"change/three.txt".to_string()));
        assert!(!p.contains(&"change/two.txt".to_string()));
        assert!(p.contains(&"fresh/deep/new.txt".to_string()));
        assert!(p.contains(&"keep/inner/x.txt".to_string()));
    }

    #[test]
    fn watcher_follows_renames_and_removals() {
        let tmp = tempfile::tempdir().unwrap();
        let root = fs::canonicalize(tmp.path()).unwrap().join("ws");
        touch(&root.join("a/one.txt"));
        touch(&root.join("a/sub/two.txt"));
        let seen = Arc::new(AtomicU64::new(0));
        let s2 = seen.clone();
        let h = spawn(root.clone(), tmp.path().join("c.idx"), Arc::new(move |r| s2.store(r, Ordering::Relaxed)));
        let wait = |ok: &dyn Fn(&Index) -> bool| {
            for _ in 0..100 {
                if ok(&h.index.read().unwrap()) {
                    return true;
                }
                std::thread::sleep(Duration::from_millis(100));
            }
            false
        };
        assert!(wait(&|i| !i.scanning && i.find("a/sub/two.txt").is_ok()));
        std::thread::sleep(Duration::from_millis(500));
        fs::rename(root.join("a"), root.join("b")).unwrap();
        assert!(wait(&|i| i.find("b/sub/two.txt").is_ok() && i.find("a").is_err() && i.find("a/one.txt").is_err()));
        touch(&root.join("b/sub/more/deep.txt"));
        assert!(wait(&|i| i.find("b/sub/more/deep.txt").is_ok()));
        fs::remove_dir_all(root.join("b/sub")).unwrap();
        assert!(wait(&|i| i.find("b/sub").is_err() && i.find("b/sub/more/deep.txt").is_err() && i.find("b/one.txt").is_ok()));
        h.stop();
        for _ in 0..50 {
            if !h.alive() {
                break;
            }
            std::thread::sleep(Duration::from_millis(50));
        }
        assert!(!h.alive());
    }
}
