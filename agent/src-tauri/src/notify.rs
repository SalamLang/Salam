use regex::Regex;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::sync::{Mutex, OnceLock};
use std::time::{Duration, Instant};
use tauri::{AppHandle, Manager, UserAttentionType};

const APP_NAME: &str = "Salam Agent";
const REPEAT_GAP: Duration = Duration::from_secs(4);
const SNIPPET_CHARS: usize = 140;
const TITLE_CHARS: usize = 60;

#[derive(Clone, Copy, Debug, Default, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Lang {
    #[default]
    Fa,
    En,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize, PartialEq, Eq, Hash)]
#[serde(rename_all = "lowercase")]
pub enum Kind {
    Done,
    Failed,
    Network,
    Approval,
    Offline,
    Online,
    Unlinked,
    Test,
}

impl Kind {
    fn urgent(self) -> bool {
        matches!(self, Kind::Failed | Kind::Approval)
    }

    fn failure(self) -> bool {
        matches!(self, Kind::Failed | Kind::Network | Kind::Offline | Kind::Unlinked)
    }
}

#[derive(Clone, Debug, Deserialize, Serialize, PartialEq, Eq)]
#[serde(default)]
pub struct Prefs {
    pub enabled: bool,
    pub sound: bool,
    pub preview: bool,
    pub lang: Lang,
}

impl Default for Prefs {
    fn default() -> Self {
        Prefs {
            enabled: true,
            sound: true,
            preview: true,
            lang: Lang::default(),
        }
    }
}

#[derive(Clone, Debug)]
pub struct Notice {
    pub kind: Kind,
    pub chat: String,
    pub text: String,
}

impl Notice {
    pub fn new(kind: Kind) -> Self {
        Notice {
            kind,
            chat: String::new(),
            text: String::new(),
        }
    }

    pub fn text(mut self, text: impl Into<String>) -> Self {
        self.text = text.into();
        self
    }
}

#[derive(Clone, Copy, Debug, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Delivery {
    Shown,
    Skipped,
}

pub struct Notifier {
    file: PathBuf,
    prefs: Mutex<Prefs>,
    recent: Mutex<HashMap<(Kind, String), Instant>>,
}

impl Notifier {
    pub fn load(app: &AppHandle) -> Self {
        let dir = app
            .path()
            .app_config_dir()
            .unwrap_or_else(|_| std::env::temp_dir().join("salam-agent"));
        let file = dir.join("notifications.json");
        let prefs = std::fs::read_to_string(&file)
            .ok()
            .and_then(|t| serde_json::from_str(&t).ok())
            .unwrap_or_default();
        Notifier {
            file,
            prefs: Mutex::new(prefs),
            recent: Mutex::new(HashMap::new()),
        }
    }

    pub fn prefs(&self) -> Prefs {
        self.prefs.lock().unwrap().clone()
    }

    pub fn set_prefs(&self, prefs: Prefs) -> Prefs {
        *self.prefs.lock().unwrap() = prefs.clone();
        if let Some(dir) = self.file.parent() {
            let _ = std::fs::create_dir_all(dir);
        }
        if let Ok(text) = serde_json::to_string(&prefs) {
            let _ = std::fs::write(&self.file, text);
        }
        prefs
    }

    fn due(&self, key: (Kind, String), now: Instant) -> bool {
        let mut recent = self.recent.lock().unwrap();
        recent.retain(|_, at| now.duration_since(*at) < REPEAT_GAP);
        if recent.contains_key(&key) {
            return false;
        }
        recent.insert(key, now);
        true
    }
}

fn pick<'a>(lang: Lang, fa: &'a str, en: &'a str) -> &'a str {
    match lang {
        Lang::Fa => fa,
        Lang::En => en,
    }
}

fn cut(text: &str, max: usize) -> String {
    if text.chars().count() <= max {
        return text.to_string();
    }
    let head: String = text.chars().take(max.saturating_sub(1)).collect();
    format!("{}…", head.trim_end())
}

fn links() -> &'static Regex {
    static RE: OnceLock<Regex> = OnceLock::new();
    RE.get_or_init(|| Regex::new(r"\[([^\]]*)\]\([^)]*\)").expect("link pattern"))
}

pub fn snippet(markdown: &str, max: usize) -> String {
    let mut out = String::new();
    let mut fenced = false;
    for line in markdown.lines() {
        let line = line.trim();
        if line.starts_with("```") {
            fenced = !fenced;
            continue;
        }
        if fenced || line.is_empty() {
            continue;
        }
        let line = line.trim_start_matches(['#', '>', '-', '*', ' ']);
        let line = links().replace_all(line, "$1");
        let line: String = line.chars().filter(|c| !matches!(c, '*' | '`')).collect();
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        if !out.is_empty() {
            out.push(' ');
        }
        out.push_str(line);
        if out.chars().count() > max {
            break;
        }
    }
    cut(&out, max)
}

pub fn compose(n: &Notice, prefs: &Prefs) -> (String, String) {
    let l = prefs.lang;
    let detail = if prefs.preview {
        cut(n.text.trim(), SNIPPET_CHARS)
    } else {
        String::new()
    };
    let chat = if prefs.preview {
        cut(n.chat.trim(), TITLE_CHARS)
    } else {
        String::new()
    };
    let with_chat = |generic: &str, detail: &str| -> String {
        match (chat.is_empty(), detail.is_empty()) {
            (true, true) => generic.to_string(),
            (true, false) => detail.to_string(),
            (false, true) => format!("{chat}\n{generic}"),
            (false, false) => format!("{chat}\n{detail}"),
        }
    };
    match n.kind {
        Kind::Done => {
            let reply = if prefs.preview {
                snippet(&n.text, SNIPPET_CHARS)
            } else {
                String::new()
            };
            (
                pick(l, "پاسخ آماده است", "Reply ready").to_string(),
                with_chat(pick(l, "پاسخ دستیار سلام آماده شد.", "Your Salam AI reply is ready."), &reply),
            )
        }
        Kind::Failed => (
            pick(l, "پاسخ ناموفق بود", "The reply failed").to_string(),
            with_chat(
                pick(
                    l,
                    "دستیار نتوانست پاسخ بدهد. دوباره تلاش کنید.",
                    "The assistant could not answer. Please try again.",
                ),
                &cut(n.text.trim(), SNIPPET_CHARS),
            ),
        ),
        Kind::Network => (
            pick(l, "ارتباط قطع شد", "Connection lost").to_string(),
            with_chat(
                pick(
                    l,
                    "ارتباط با سرور برقرار نیست. اتصال اینترنت را بررسی کنید. پاسخ ممکن است همچنان برسد.",
                    "Cannot reach the server. Check your internet connection. The reply may still arrive.",
                ),
                "",
            ),
        ),
        Kind::Approval => (
            pick(l, "منتظر اجازه‌ی شما", "Waiting for your approval").to_string(),
            if detail.is_empty() {
                pick(
                    l,
                    "دستیار برای ادامه به اجازه‌ی شما نیاز دارد.",
                    "The assistant needs your approval to continue.",
                )
                .to_string()
            } else {
                detail
            },
        ),
        Kind::Offline => (
            pick(l, "ایجنت سلام آفلاین است", "Salam Agent is offline").to_string(),
            pick(
                l,
                "ارتباط با دستیار قطع شد و برنامه در پس‌زمینه دوباره تلاش می‌کند.",
                "The connection to the assistant dropped. Retrying in the background.",
            )
            .to_string(),
        ),
        Kind::Online => (
            pick(l, "دوباره وصل شد", "Back online").to_string(),
            pick(
                l,
                "ایجنت سلام دوباره به دستیار وصل شد.",
                "Salam Agent is connected to the assistant again.",
            )
            .to_string(),
        ),
        Kind::Unlinked => (
            pick(l, "اتصال این رایانه قطع شد", "This computer was disconnected").to_string(),
            pick(l, "برای وصل شدن دوباره وارد حساب خود شوید.", "Sign in again to reconnect it.").to_string(),
        ),
        Kind::Test => (
            pick(l, "اعلان‌ها کار می‌کنند", "Notifications work").to_string(),
            pick(
                l,
                "اگر این پیام را می‌بینید، اعلان‌های ایجنت سلام درست تنظیم شده‌اند.",
                "If you can read this, Salam Agent notifications are set up correctly.",
            )
            .to_string(),
        ),
    }
}

fn looking_at_window(app: &AppHandle) -> bool {
    let Some(w) = app.get_webview_window("main") else { return false };
    w.is_visible().unwrap_or(false) && !w.is_minimized().unwrap_or(false) && w.is_focused().unwrap_or(false)
}

pub fn raise(app: &AppHandle) {
    if let Some(w) = app.get_webview_window("main") {
        let _ = w.show();
        let _ = w.unminimize();
        let _ = w.set_focus();
    }
}

fn attract(app: &AppHandle, kind: Kind) {
    if let Some(w) = app.get_webview_window("main") {
        let level = if kind.urgent() {
            UserAttentionType::Critical
        } else {
            UserAttentionType::Informational
        };
        let _ = w.request_user_attention(Some(level));
    }
}

#[cfg_attr(not(windows), allow(dead_code))]
fn under_any(exe: &Path, roots: &[PathBuf]) -> bool {
    let norm = |p: &Path| p.to_string_lossy().replace('\\', "/").to_lowercase();
    let exe = norm(exe);
    roots.iter().any(|r| {
        let r = norm(r);
        let r = r.trim_end_matches('/');
        !r.is_empty() && exe.starts_with(&format!("{r}/"))
    })
}

#[cfg(windows)]
fn installed() -> bool {
    let Ok(exe) = std::env::current_exe() else { return false };
    let roots: Vec<PathBuf> = ["LOCALAPPDATA", "ProgramFiles", "ProgramFiles(x86)", "ProgramW6432"]
        .iter()
        .filter_map(std::env::var_os)
        .map(PathBuf::from)
        .collect();
    under_any(&exe, &roots)
}

#[cfg(all(unix, not(target_os = "macos")))]
fn icon_file(app: &AppHandle) -> Option<PathBuf> {
    static ICON: OnceLock<Option<PathBuf>> = OnceLock::new();
    ICON.get_or_init(|| {
        let bytes = include_bytes!("../icons/128x128.png");
        let dir = app
            .path()
            .app_cache_dir()
            .unwrap_or_else(|_| std::env::temp_dir().join("salam-agent"));
        let file = dir.join("notification-icon.png");
        let current = std::fs::metadata(&file).map(|m| m.len() == bytes.len() as u64).unwrap_or(false);
        if !current {
            std::fs::create_dir_all(&dir).ok()?;
            std::fs::write(&file, bytes).ok()?;
        }
        Some(file)
    })
    .clone()
}

fn show(app: &AppHandle, kind: Kind, title: &str, body: &str, sound: bool) -> Result<(), String> {
    let mut n = notify_rust::Notification::new();
    n.appname(APP_NAME).summary(title).body(body);

    #[cfg(windows)]
    {
        if installed() {
            n.app_id(&app.config().identifier);
        }
        if sound {
            n.sound_name(if kind.failure() || kind.urgent() { "Reminder" } else { "Default" });
        }
    }

    #[cfg(target_os = "macos")]
    {
        let id = if tauri::is_dev() {
            "com.apple.Terminal".to_string()
        } else {
            app.config().identifier.clone()
        };
        let _ = notify_rust::set_application(&id);
        if sound {
            n.sound_name(if kind.failure() || kind.urgent() { "Basso" } else { "Glass" });
        }
    }

    #[cfg(all(unix, not(target_os = "macos")))]
    {
        use notify_rust::Hint;
        if let Some(icon) = icon_file(app) {
            n.icon(&icon.to_string_lossy());
        }
        n.hint(Hint::Category(
            if kind.failure() { "network.error" } else { "im.received" }.to_string(),
        ));
        if sound {
            n.hint(Hint::SoundName(
                (if kind.failure() || kind.urgent() {
                    "dialog-warning"
                } else {
                    "message-new-instant"
                })
                .to_string(),
            ));
        } else {
            n.hint(Hint::SuppressSound(true));
        }
        n.action("default", "Open");
    }
    fire(app, &n)
}

#[cfg(all(unix, not(target_os = "macos")))]
fn fire(app: &AppHandle, n: &notify_rust::Notification) -> Result<(), String> {
    let handle = n.show().map_err(|e| e.to_string())?;
    let app = app.clone();
    tauri::async_runtime::spawn_blocking(move || {
        handle.wait_for_action(|action| {
            if action != "__closed" {
                raise(&app);
            }
        });
    });
    Ok(())
}

#[cfg(any(windows, target_os = "macos"))]
fn fire(_app: &AppHandle, n: &notify_rust::Notification) -> Result<(), String> {
    n.show().map(|_| ()).map_err(|e| e.to_string())
}

pub async fn deliver(app: &AppHandle, notice: Notice) -> Result<Delivery, String> {
    let prefs = app.state::<Notifier>().prefs();
    let kind = notice.kind;
    let test = kind == Kind::Test;
    if !test && kind != Kind::Approval && !prefs.enabled {
        return Ok(Delivery::Skipped);
    }
    if !test && looking_at_window(app) {
        return Ok(Delivery::Skipped);
    }
    let key = (kind, notice.chat.clone());
    if !test && !app.state::<Notifier>().due(key, Instant::now()) {
        return Ok(Delivery::Skipped);
    }
    attract(app, kind);
    if !prefs.enabled && !test {
        return Ok(Delivery::Skipped);
    }
    let (title, body) = compose(&notice, &prefs);
    let handle = app.clone();
    let sound = prefs.sound;
    tauri::async_runtime::spawn_blocking(move || show(&handle, kind, &title, &body, sound))
        .await
        .map_err(|e| e.to_string())??;
    Ok(Delivery::Shown)
}

pub fn post(app: &AppHandle, notice: Notice) {
    let app = app.clone();
    tauri::async_runtime::spawn(async move {
        let _ = deliver(&app, notice).await;
    });
}

#[cfg(test)]
mod tests {
    use super::*;

    fn prefs(lang: Lang, preview: bool) -> Prefs {
        Prefs {
            lang,
            preview,
            ..Prefs::default()
        }
    }

    #[test]
    fn markdown_is_flattened_to_one_readable_line() {
        let md = "## Done\n\nI fixed **the build** in [main.salam](https://x.y/z).\n\n```salam\nprintln 1\n```\n- all tests pass";
        assert_eq!(snippet(md, 200), "Done I fixed the build in main.salam. all tests pass");
        assert_eq!(snippet("```\nonly code\n```", 50), "");
    }

    #[test]
    fn long_text_is_cut_on_a_character_boundary() {
        let s = snippet(&"پاسخ ".repeat(100), 20);
        assert_eq!(s.chars().count(), 20);
        assert!(s.ends_with('…'));
    }

    #[test]
    fn done_shows_chat_and_preview_only_when_allowed() {
        let n = Notice {
            kind: Kind::Done,
            chat: "Fix the parser".into(),
            text: "All **12** tests pass now.".into(),
        };
        let (title, body) = compose(&n, &prefs(Lang::En, true));
        assert_eq!(title, "Reply ready");
        assert_eq!(body, "Fix the parser\nAll 12 tests pass now.");
        let (_, private) = compose(&n, &prefs(Lang::En, false));
        assert_eq!(private, "Your Salam AI reply is ready.");
    }

    #[test]
    fn every_kind_has_text_in_both_languages() {
        let kinds = [
            Kind::Done,
            Kind::Failed,
            Kind::Network,
            Kind::Approval,
            Kind::Offline,
            Kind::Online,
            Kind::Unlinked,
            Kind::Test,
        ];
        for kind in kinds {
            for lang in [Lang::Fa, Lang::En] {
                let (title, body) = compose(&Notice::new(kind), &prefs(lang, true));
                assert!(!title.is_empty() && !body.is_empty(), "{kind:?} {lang:?}");
                assert!(!title.contains('—') && !body.contains('—'));
            }
        }
    }

    #[test]
    fn approval_shows_what_is_being_asked_for() {
        let n = Notice::new(Kind::Approval).text("Run: cargo test");
        assert_eq!(compose(&n, &prefs(Lang::En, true)).1, "Run: cargo test");
        assert_eq!(
            compose(&n, &prefs(Lang::En, false)).1,
            "The assistant needs your approval to continue."
        );
    }

    #[test]
    fn prefs_fill_in_missing_fields() {
        let p: Prefs = serde_json::from_str(r#"{"sound":false}"#).unwrap();
        assert!(p.enabled && p.preview && !p.sound);
        assert_eq!(p.lang, Lang::Fa);
        let p: Prefs = serde_json::from_str(r#"{"lang":"en"}"#).unwrap();
        assert_eq!(p.lang, Lang::En);
    }

    #[test]
    fn the_same_notice_is_not_repeated_in_a_burst() {
        let n = Notifier {
            file: PathBuf::new(),
            prefs: Mutex::new(Prefs::default()),
            recent: Mutex::new(HashMap::new()),
        };
        let t0 = Instant::now();
        assert!(n.due((Kind::Done, "a".into()), t0));
        assert!(!n.due((Kind::Done, "a".into()), t0 + Duration::from_secs(1)));
        assert!(n.due((Kind::Done, "b".into()), t0 + Duration::from_secs(1)));
        assert!(n.due((Kind::Failed, "a".into()), t0 + Duration::from_secs(1)));
        assert!(n.due((Kind::Done, "a".into()), t0 + Duration::from_secs(5)));
    }

    #[test]
    fn only_installed_windows_apps_use_their_own_identity() {
        let roots = vec![PathBuf::from(r"C:\Users\me\AppData\Local"), PathBuf::from(r"C:\Program Files")];
        assert!(under_any(
            Path::new(r"C:\Users\me\AppData\Local\Salam Agent\salam-agent.exe"),
            &roots
        ));
        assert!(under_any(Path::new(r"c:\program files\Salam Agent\salam-agent.exe"), &roots));
        assert!(!under_any(Path::new(r"C:\Users\me\Downloads\salam-agent.exe"), &roots));
        assert!(!under_any(Path::new(r"C:\Program Files Extra\x.exe"), &roots));
        assert!(!under_any(Path::new(r"C:\x.exe"), &[PathBuf::new()]));
    }
}
