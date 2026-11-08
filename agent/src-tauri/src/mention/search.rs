use super::index::{Entry, Index};
use serde::Serialize;
use std::sync::atomic::{AtomicU64, Ordering};

#[derive(Serialize, Clone, Debug, PartialEq)]
pub struct Hit {
    pub path: String,
    pub is_dir: bool,
    pub name_at: usize,
    pub ranges: Vec<(usize, usize)>,
    #[serde(skip)]
    pub score: i32,
}

pub struct Cancel<'a> {
    pub seq: &'a AtomicU64,
    pub mine: u64,
}

impl Cancel<'_> {
    fn stale(&self) -> bool {
        self.seq.load(Ordering::Relaxed) != self.mine
    }
}

fn base(c: char) -> Option<char> {
    Some(match c {
        'ي' | 'ى' | 'ئ' => 'ی',
        'ك' => 'ک',
        'ۀ' | 'ة' => 'ه',
        'أ' | 'إ' | 'ٱ' => 'ا',
        '\u{200c}' | '\u{200d}' | '\u{640}' | '\u{200e}' | '\u{200f}' => '\0',
        '\u{300}'..='\u{36f}' | '\u{64b}'..='\u{65f}' | '\u{670}' => '\0',
        '٠'..='٩' => (b'0' + (c as u32 - '٠' as u32) as u8) as char,
        '۰'..='۹' => (b'0' + (c as u32 - '۰' as u32) as u8) as char,
        '\\' => '/',
        _ => return None,
    })
}

pub fn fold(c: char) -> char {
    if c.is_ascii() && c != '\\' {
        return c.to_ascii_lowercase();
    }
    base(c).unwrap_or_else(|| c.to_lowercase().next().unwrap_or(c))
}

fn fold_cs(c: char) -> char {
    base(c).unwrap_or(c)
}

#[derive(Clone)]
pub struct Query {
    chars: Vec<(char, Option<char>)>,
    pub has_slash: bool,
}

impl Query {
    pub fn new(text: &str) -> Query {
        let insensitive = cfg!(any(windows, target_os = "macos"));
        let smart = !insensitive && text.chars().any(|c| c.is_uppercase());
        let mut chars = Vec::new();
        for c in text.chars() {
            let f = fold(c);
            if f == '\0' {
                continue;
            }
            let exact = if smart && c.is_uppercase() { Some(fold_cs(c)) } else { None };
            chars.push((f, exact));
        }
        Query {
            has_slash: chars.iter().any(|(c, _)| *c == '/'),
            chars,
        }
    }

    pub fn len(&self) -> usize {
        self.chars.len()
    }

    pub fn is_empty(&self) -> bool {
        self.chars.is_empty()
    }

    fn matches(&self, qi: usize, c: char, f: char) -> bool {
        let (q, exact) = self.chars[qi];
        f == q && exact.is_none_or(|e| fold_cs(c) == e)
    }
}

#[derive(Default)]
pub struct Scratch {
    orig: Vec<char>,
    folded: Vec<char>,
    pos: Vec<usize>,
}

fn boundary_bonus(orig: &[char], folded: &[char], p: usize) -> i32 {
    if p == 0 {
        return 30;
    }
    let mut j = p;
    while j > 0 {
        j -= 1;
        if folded[j] == '\0' {
            continue;
        }
        let prev = orig[j];
        let cur = orig[p];
        return match prev {
            '/' => 30,
            '_' | '-' | '.' | ' ' | '\u{200c}' => 24,
            _ if prev.is_lowercase() && cur.is_uppercase() => 20,
            _ if prev.is_alphabetic() != cur.is_alphabetic() => 12,
            _ => 0,
        };
    }
    30
}

fn fast_reject(path: &str, off: usize, q: &Query) -> bool {
    if path.len() - off < q.len() {
        return true;
    }
    let mut qi = 0;
    for c in path[off..].chars() {
        if qi < q.chars.len() && q.matches(qi, c, fold(c)) {
            qi += 1;
        } else if c == '\0' {
            continue;
        }
    }
    qi < q.chars.len()
}

pub struct Ctx<'a> {
    pub now: u32,
    pub mru: &'a [(usize, i32)],
}

fn score_entry(e: &Entry, idx: usize, off: usize, q: &Query, ctx: &Ctx, s: &mut Scratch) -> Option<i32> {
    if fast_reject(&e.path, off, q) {
        return None;
    }
    s.orig.clear();
    s.folded.clear();
    for c in e.path[off..].chars() {
        s.orig.push(c);
        s.folded.push(fold(c));
    }
    let n = s.orig.len();
    let name_at = if (e.name_at as usize) >= off {
        e.path[off..e.name_at as usize].chars().count().min(n)
    } else {
        0
    };
    let mut qi = 0;
    let mut end = 0;
    for (i, &f) in s.folded.iter().enumerate() {
        if f == '\0' {
            continue;
        }
        if qi < q.len() && q.matches(qi, s.orig[i], f) {
            qi += 1;
            if qi == q.len() {
                end = i;
                break;
            }
        }
    }
    if qi < q.len() {
        return None;
    }
    s.pos.clear();
    let mut qj = q.len();
    let mut i = end + 1;
    while qj > 0 && i > 0 {
        i -= 1;
        if s.folded[i] == '\0' {
            continue;
        }
        if q.matches(qj - 1, s.orig[i], s.folded[i]) {
            s.pos.push(i);
            qj -= 1;
        }
    }
    s.pos.reverse();
    let mut score = 0i32;
    let mut prev: Option<usize> = None;
    for &p in &s.pos {
        score += 16 + boundary_bonus(&s.orig, &s.folded, p);
        if p >= name_at {
            score += 8;
        }
        match prev {
            Some(pp) => {
                let gap = s.folded[pp + 1..p].iter().filter(|c| **c != '\0').count();
                if gap == 0 {
                    score += 20;
                } else {
                    score -= (gap.min(10) as i32) * 2;
                }
            }
            None => score -= (s.pos[0].saturating_sub(name_at).min(10) as i32) * 2 * i32::from(s.pos[0] >= name_at),
        }
        prev = Some(p);
    }
    let first = s.pos[0];
    let last = *s.pos.last().unwrap_or(&0);
    let contiguous = s.pos.windows(2).all(|w| s.folded[w[0] + 1..w[1]].iter().all(|c| *c == '\0'));
    let first_name = (name_at..n).find(|i| s.folded[*i] != '\0');
    let last_name = (name_at..n).rev().find(|i| s.folded[*i] != '\0');
    let in_name = first >= name_at && !q.has_slash;
    if in_name && contiguous {
        if Some(first) == first_name {
            score += 700;
            if Some(last) == last_name {
                score += 600;
            } else if s.orig.get(last + 1) == Some(&'.') {
                score += 150;
            }
            if e.is_dir {
                score += 4;
            }
        } else {
            score += 300;
        }
    }
    score -= i32::from(e.depth) * 3;
    score -= (n / 8).min(40) as i32;
    let age = ctx.now.saturating_sub(e.mtime);
    score += match age {
        0..=86_400 => 12,
        86_401..=604_800 => 8,
        604_801..=2_592_000 => 4,
        _ => 0,
    } * i32::from(e.mtime > 0);
    if let Ok(k) = ctx.mru.binary_search_by_key(&idx, |m| m.0) {
        score += ctx.mru[k].1;
    }
    Some(score)
}

fn ranges_of(e: &Entry, off: usize, q: &Query, ctx: &Ctx, s: &mut Scratch) -> Vec<(usize, usize)> {
    if score_entry(e, 0, off, q, ctx, s).is_none() {
        return Vec::new();
    }
    let prefix: usize = e.path[..off].chars().map(char::len_utf16).sum();
    let mut utf16 = Vec::with_capacity(s.orig.len() + 1);
    let mut acc = prefix;
    for c in &s.orig {
        utf16.push(acc);
        acc += c.len_utf16();
    }
    utf16.push(acc);
    let mut out: Vec<(usize, usize)> = Vec::new();
    for &p in &s.pos {
        let (a, b) = (utf16[p], utf16[p + 1]);
        match out.last_mut() {
            Some(last) if last.1 >= a => last.1 = b,
            _ => out.push((a, b)),
        }
    }
    out
}

fn to_hit(e: &Entry, score: i32, ranges: Vec<(usize, usize)>) -> Hit {
    Hit {
        path: e.path.to_string(),
        is_dir: e.is_dir,
        name_at: e.path[..e.name_at as usize].chars().map(char::len_utf16).sum(),
        ranges,
        score,
    }
}

fn better(a: &(i32, usize), b: &(i32, usize)) -> bool {
    a.0 > b.0 || (a.0 == b.0 && a.1 < b.1)
}

fn push_top(top: &mut Vec<(i32, usize)>, item: (i32, usize), limit: usize) {
    if top.len() == limit {
        let worst = top.iter().enumerate().fold(0, |w, (i, t)| if better(&top[w], t) { i } else { w });
        if !better(&item, &top[worst]) {
            return;
        }
        top.swap_remove(worst);
    }
    top.push(item);
}

fn mru_bonuses(index: &Index) -> Vec<(usize, i32)> {
    let mut out: Vec<(usize, i32)> = index
        .mru
        .iter()
        .enumerate()
        .filter_map(|(rank, p)| index.find(p).ok().map(|i| (i, 160 - (rank as i32) * 2)))
        .collect();
    out.sort();
    out
}

fn now_secs() -> u32 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_or(0, |d| d.as_secs().min(u32::MAX as u64) as u32)
}

fn normalize_dir(raw: &str) -> String {
    raw.replace('\\', "/").trim_matches('/').to_string()
}

fn resolve_dir(index: &Index, dir: &str) -> Option<String> {
    if dir.is_empty() {
        return Some(String::new());
    }
    if index.is_dir(dir) {
        return Some(dir.to_string());
    }
    let want: String = dir.chars().map(fold).filter(|c| *c != '\0').collect();
    index
        .entries
        .iter()
        .filter(|e| e.is_dir)
        .find(|e| e.path.chars().map(fold).filter(|c| *c != '\0').eq(want.chars()))
        .map(|e| e.path.to_string())
}

pub enum Outcome {
    Hits(Vec<Hit>),
    UnknownDir(String, String),
    Stale,
}

pub fn search(index: &Index, text: &str, limit: usize, cancel: &Cancel) -> Outcome {
    let limit = limit.clamp(1, 100);
    let text = text.trim_start();
    let mru = mru_bonuses(index);
    let ctx = Ctx {
        now: now_secs(),
        mru: &mru,
    };
    let (dir, rest) = match text.rfind(['/', '\\']) {
        Some(i) => (normalize_dir(&text[..i]), &text[i + 1..]),
        None => (String::new(), text),
    };
    let has_dir = text.contains(['/', '\\']);
    let (range, off) = if has_dir {
        match resolve_dir(index, &dir) {
            Some(real) => {
                if Query::new(rest).is_empty() {
                    return Outcome::Hits(children_listing(index, &real, limit));
                }
                (index.subtree(&real), if real.is_empty() { 0 } else { real.len() + 1 })
            }
            None => return Outcome::UnknownDir(dir, rest.to_string()),
        }
    } else {
        (0..index.entries.len(), 0)
    };
    let q = Query::new(rest);
    if q.is_empty() {
        return Outcome::Hits(default_listing(index, &mru, limit));
    }
    let slice = &index.entries[range.clone()];
    let staged: &[Entry] = if has_dir { &[] } else { &index.staging };
    let workers = if slice.len() + staged.len() > 20_000 {
        std::thread::available_parallelism().map_or(2, |n| n.get().min(4))
    } else {
        1
    };
    let chunk = (slice.len() / workers).max(1);
    let base = range.start;
    let results: Vec<Option<Vec<(i32, usize)>>> = std::thread::scope(|sc| {
        let ctx = &ctx;
        let q = &q;
        let handles: Vec<_> = slice
            .chunks(chunk)
            .enumerate()
            .map(|(ci, part)| {
                sc.spawn(move || {
                    let mut scratch = Scratch::default();
                    let mut top = Vec::with_capacity(limit);
                    for (k, e) in part.iter().enumerate() {
                        if k % 2048 == 0 && cancel.stale() {
                            return None;
                        }
                        let idx = base + ci * chunk + k;
                        if let Some(sc) = score_entry(e, idx, off, q, ctx, &mut scratch) {
                            push_top(&mut top, (sc, idx), limit);
                        }
                    }
                    Some(top)
                })
            })
            .collect();
        handles.into_iter().map(|h| h.join().unwrap_or(None)).collect()
    });
    let mut all: Vec<(i32, usize)> = Vec::new();
    for r in results {
        match r {
            Some(t) => all.extend(t),
            None => return Outcome::Stale,
        }
    }
    let mut scratch = Scratch::default();
    for (k, e) in staged.iter().enumerate() {
        if let Some(sc) = score_entry(e, index.entries.len() + k, 0, &q, &ctx, &mut scratch) {
            push_top(&mut all, (sc, index.entries.len() + k), limit * 2);
        }
    }
    all.sort_by(|a, b| b.0.cmp(&a.0).then(a.1.cmp(&b.1)));
    all.truncate(limit);
    if cancel.stale() {
        return Outcome::Stale;
    }
    let entry_at = |i: usize| {
        if i < index.entries.len() {
            &index.entries[i]
        } else {
            &index.staging[i - index.entries.len()]
        }
    };
    let hits = all
        .into_iter()
        .map(|(score, i)| {
            let e = entry_at(i);
            let o = if i < index.entries.len() { off } else { 0 };
            to_hit(e, score, ranges_of(e, o, &q, &ctx, &mut scratch))
        })
        .collect();
    Outcome::Hits(hits)
}

fn children_listing(index: &Index, dir: &str, limit: usize) -> Vec<Hit> {
    let mut kids = index.direct_children(dir);
    kids.sort_by(|a, b| {
        let (x, y) = (&index.entries[*a], &index.entries[*b]);
        y.is_dir
            .cmp(&x.is_dir)
            .then_with(|| x.name().to_lowercase().cmp(&y.name().to_lowercase()))
    });
    kids.into_iter()
        .take(limit)
        .map(|i| to_hit(&index.entries[i], 0, Vec::new()))
        .collect()
}

fn default_listing(index: &Index, mru: &[(usize, i32)], limit: usize) -> Vec<Hit> {
    let mut picked: Vec<usize> = mru.iter().map(|m| m.0).collect();
    picked.sort_by_key(|i| std::cmp::Reverse(mru.iter().find(|m| m.0 == *i).map_or(0, |m| m.1)));
    let mut recent: Vec<usize> = (0..index.entries.len())
        .filter(|i| !index.entries[*i].is_dir && index.entries[*i].mtime > 0)
        .collect();
    let keep = 20.min(recent.len());
    if keep > 0 {
        recent.select_nth_unstable_by(keep - 1, |a, b| index.entries[*b].mtime.cmp(&index.entries[*a].mtime));
        recent.truncate(keep);
        recent.sort_by(|a, b| index.entries[*b].mtime.cmp(&index.entries[*a].mtime));
    }
    picked.extend(recent);
    for want_dir in [true, false] {
        picked.extend(
            index
                .direct_children("")
                .into_iter()
                .filter(|i| index.entries[*i].is_dir == want_dir),
        );
    }
    let mut seen = std::collections::HashSet::new();
    picked.retain(|i| seen.insert(*i));
    picked
        .into_iter()
        .take(limit)
        .map(|i| to_hit(&index.entries[i], 0, Vec::new()))
        .collect()
}

pub fn scratch_index(entries: Vec<Entry>) -> Index {
    let mut index = Index {
        entries,
        ..Index::default()
    };
    index.entries.sort_by(|a, b| a.path.cmp(&b.path));
    index
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make(paths: &[&str]) -> Index {
        let mut index = Index::default();
        for p in paths {
            let is_dir = p.ends_with('/');
            index.entries.push(Entry::new(p.trim_end_matches('/'), 0, is_dir));
        }
        index.entries.sort_by(|a, b| a.path.cmp(&b.path));
        index
    }

    fn run(index: &Index, q: &str) -> Vec<String> {
        let seq = AtomicU64::new(1);
        match search(index, q, 50, &Cancel { seq: &seq, mine: 1 }) {
            Outcome::Hits(h) => h.into_iter().map(|h| h.path).collect(),
            _ => panic!("no hits"),
        }
    }

    #[test]
    fn basename_exact_and_prefix_beat_path_matches() {
        let index = make(&[
            "src/main.rs",
            "src/domain/rules.rs",
            "docs/mainly-about.md",
            "main.rs",
            "src/a/m/a/i/n.txt",
            "tests/main_test.rs",
        ]);
        let r = run(&index, "main.rs");
        assert_eq!(r[0], "main.rs");
        assert_eq!(r[1], "src/main.rs");
        let r = run(&index, "main");
        assert_eq!(r[0], "main.rs");
        assert!(r.iter().position(|p| p == "docs/mainly-about.md").unwrap() < r.iter().position(|p| p == "src/a/m/a/i/n.txt").unwrap());
    }

    #[test]
    fn camel_case_and_segment_starts_rank_high() {
        let index = make(&["src/UserProfileCard.tsx", "src/unrelated_poorly_chosen_cards.tsx", "src/usage.rs"]);
        assert_eq!(run(&index, "upc")[0], "src/UserProfileCard.tsx");
        let index = make(&["src/main/rules.rs", "q/xmxrxsx.txt"]);
        assert_eq!(run(&index, "mrs")[0], "src/main/rules.rs");
    }

    #[test]
    fn smart_case_and_subsequence() {
        let index = make(&["README.md", "readme.txt", "a/read/me.txt"]);
        if cfg!(not(any(windows, target_os = "macos"))) {
            assert_eq!(run(&index, "README"), ["README.md"]);
        }
        assert_eq!(run(&index, "readme").len(), 3);
        assert!(run(&index, "zzz").is_empty());
    }

    #[test]
    fn persian_names_match_across_letter_variants() {
        let index = make(&["docs/گزارش\u{200c}ها.md", "docs/کتاب.txt", "docs/other.txt"]);
        assert_eq!(run(&index, "گزارشها")[0], "docs/گزارش\u{200c}ها.md");
        assert_eq!(run(&index, "كتاب")[0], "docs/کتاب.txt");
        let seq = AtomicU64::new(1);
        let Outcome::Hits(h) = search(&index, "کتاب", 5, &Cancel { seq: &seq, mine: 1 }) else {
            panic!()
        };
        assert_eq!(h[0].ranges, vec![(h[0].name_at, h[0].name_at + 4)]);
    }

    #[test]
    fn trailing_slash_lists_children_and_dir_prefix_restricts() {
        let index = make(&["src/", "src/a.rs", "src/b/", "src/b/c.rs", "other/", "other/a.rs"]);
        assert_eq!(run(&index, "src/"), ["src/b", "src/a.rs"]);
        assert_eq!(run(&index, "src/c"), ["src/b/c.rs"]);
        assert_eq!(run(&index, "SRC/"), ["src/b", "src/a.rs"]);
    }

    #[test]
    fn unknown_dir_is_reported_for_live_listing() {
        let index = make(&["src/a.rs"]);
        let seq = AtomicU64::new(1);
        match search(&index, "vendor/", 50, &Cancel { seq: &seq, mine: 1 }) {
            Outcome::UnknownDir(d, rest) => assert_eq!((d.as_str(), rest.as_str()), ("vendor", "")),
            _ => panic!("expected an unknown dir"),
        }
    }

    #[test]
    fn empty_query_prefers_picked_then_recent_then_top_level() {
        let mut index = make(&["a/", "a/deep.txt", "b.txt", "z/", "old.txt", "new.txt"]);
        for e in &mut index.entries {
            e.mtime = match &*e.path {
                "new.txt" => 3000,
                "old.txt" => 1000,
                "b.txt" => 2000,
                _ => 0,
            };
        }
        index.mru = vec!["a/deep.txt".into()];
        let r = run(&index, "");
        assert_eq!(&r[..4], ["a/deep.txt", "new.txt", "b.txt", "old.txt"]);
        assert!(r.contains(&"a".to_string()) && r.contains(&"z".to_string()));
    }

    #[test]
    fn picked_entries_get_a_boost() {
        let mut index = make(&["x/config.json", "y/config.json"]);
        index.mru = vec!["y/config.json".into()];
        assert_eq!(run(&index, "config")[0], "y/config.json");
    }

    #[test]
    fn stale_queries_stop() {
        let index = make(&["a.txt"]);
        let seq = AtomicU64::new(2);
        assert!(matches!(search(&index, "a", 5, &Cancel { seq: &seq, mine: 1 }), Outcome::Stale));
    }

    #[test]
    fn search_over_100k_entries_is_fast() {
        let mut index = Index::default();
        for i in 0..100_000usize {
            let p = format!("packages/pkg{}/src/module{}/component_{}/file_{}.rs", i % 97, i % 1013, i % 31, i);
            index.entries.push(Entry::new(&p, 1_700_000_000 + i as u32, false));
        }
        index.entries.sort_by(|a, b| a.path.cmp(&b.path));
        let seq = AtomicU64::new(1);
        let mut best = u128::MAX;
        for q in ["file_99999", "cmp5", "pkg9module7", "zzzq"] {
            let t = std::time::Instant::now();
            let _ = search(&index, q, 50, &Cancel { seq: &seq, mine: 1 });
            best = best.min(t.elapsed().as_millis());
            let took = t.elapsed().as_millis();
            let cap = if cfg!(debug_assertions) { 400 } else { 40 };
            assert!(took < cap, "{q} took {took} ms");
        }
        eprintln!("fastest query: {best} ms");
    }
}
