use super::cdp::{self, Cdp};
use super::{Image, Kind, exec, first_line, flag, number, text};
use crate::workspace::Workspace;
use serde_json::{Value, json};
use std::collections::{HashMap, HashSet, VecDeque};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, LazyLock, Mutex, Once};
use std::time::{Duration, Instant};
use tokio::sync::Notify;

const BUFFER: usize = 200;
const IDLE: Duration = Duration::from_secs(10 * 60);
const MAX_IMAGE: usize = 1_500_000;
const MAX_PAGE_HEIGHT: f64 = 8000.0;
const MAX_TEXT: usize = 20_000;
const DEFAULT_WAIT: u64 = 1500;
const MAX_WAIT: u64 = 15_000;
const MAX_WAIT_FOR: u64 = 30_000;

const NOT_FOUND: &str = "No Chrome-family browser was found on this computer. Install Google Chrome or Microsoft Edge, \
or set SALAM_AGENT_BROWSER to the path of a Chrome, Chromium, Edge or Brave program.";

const ONE_TAB: &str = "document.addEventListener('click', e => { const a = e.target && e.target.closest && e.target.closest('a[target]'); \
if (a) a.removeAttribute('target'); }, true); window.open = u => { if (u) location.assign(u); return null; };";

const OUTLINE: &str = r#"(() => {
  const vis = e => { const r = e.getBoundingClientRect(); const s = getComputedStyle(e); return r.width > 0 && r.height > 0 && s.visibility !== 'hidden'; };
  const t = (s, n) => (s || '').replace(/\s+/g, ' ').trim().slice(0, n || 100);
  const out = { text: document.body ? document.body.innerText : '', headings: [], links: [], buttons: [], inputs: [], images: [] };
  const add = (list, v) => { if (list.length < 100) list.push(v); };
  for (const h of document.querySelectorAll('h1,h2,h3,h4,h5,h6')) if (vis(h)) add(out.headings, h.tagName.toLowerCase() + ' ' + t(h.innerText));
  for (const a of document.querySelectorAll('a[href]')) if (vis(a)) add(out.links, [t(a.innerText || a.getAttribute('aria-label') || a.title, 80), a.href.slice(0, 200)]);
  for (const b of document.querySelectorAll('button,[role=button],input[type=submit],input[type=button]')) if (vis(b)) add(out.buttons, t(b.innerText || b.value || b.getAttribute('aria-label') || b.title, 80) + (b.disabled ? ' (disabled)' : ''));
  for (const i of document.querySelectorAll('input:not([type=hidden]):not([type=submit]):not([type=button]),textarea,select')) if (vis(i)) add(out.inputs, { tag: i.tagName.toLowerCase(), type: i.type || '', name: i.name || i.id || '', placeholder: i.placeholder || '', label: t(i.labels && i.labels[0] ? i.labels[0].innerText : i.getAttribute('aria-label'), 60) });
  for (const im of document.images) { const broken = im.complete && im.naturalWidth === 0 && !!(im.currentSrc || im.src); const noalt = !im.hasAttribute('alt'); if (broken || noalt) add(out.images, { src: (im.currentSrc || im.src || '').slice(0, 200), broken, noalt }); }
  return out;
})()"#;

const SELECT: &str = r#"((selector, want) => {
  const e = document.querySelector(selector);
  if (!e) return null;
  if (e.tagName !== 'SELECT') return { tag: e.tagName.toLowerCase() };
  const opts = Array.from(e.options);
  const o = opts.find(o => o.value === want) || opts.find(o => o.text.trim() === want) || opts.find(o => o.text.trim().toLowerCase() === want.toLowerCase());
  if (!o) return { options: opts.map(o => o.text.trim() === o.value ? o.value : o.text.trim() + ' (' + o.value + ')') };
  e.scrollIntoView({ block: 'center', behavior: 'instant' });
  e.focus();
  Object.getOwnPropertyDescriptor(HTMLSelectElement.prototype, 'value').set.call(e, o.value);
  e.dispatchEvent(new Event('input', { bubbles: true }));
  e.dispatchEvent(new Event('change', { bubbles: true }));
  return { selected: o.text.trim() };
})"#;

const VISIBLE: &str = "(selector => { const e = document.querySelector(selector); if (!e) return false; const r = e.getBoundingClientRect(); \
const s = getComputedStyle(e); return r.width > 0 && r.height > 0 && s.visibility !== 'hidden' && s.display !== 'none'; })";

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Act {
    Open,
    Screenshot,
    Click,
    Type,
    Press,
    Scroll,
    Eval,
    Text,
    Console,
    Network,
    Hover,
    Select,
    Wait,
    Reload,
    Back,
    Close,
}

impl Act {
    fn parse(s: &str) -> Option<Act> {
        Some(match s.trim().to_ascii_lowercase().as_str() {
            "open" | "goto" | "navigate" => Act::Open,
            "screenshot" => Act::Screenshot,
            "click" => Act::Click,
            "type" | "fill" => Act::Type,
            "press" => Act::Press,
            "scroll" => Act::Scroll,
            "eval" | "evaluate" => Act::Eval,
            "text" => Act::Text,
            "console" => Act::Console,
            "network" | "requests" => Act::Network,
            "hover" => Act::Hover,
            "select" => Act::Select,
            "wait" => Act::Wait,
            "reload" | "refresh" => Act::Reload,
            "back" => Act::Back,
            "close" => Act::Close,
            _ => return None,
        })
    }

    pub fn kind(self) -> Kind {
        if self == Act::Eval { Kind::Exec } else { Kind::Read }
    }
}

pub struct Request {
    pub key: String,
    pub act: Act,
    pub url: String,
    pub selector: String,
    pub text: String,
    pub key_name: String,
    pub script: String,
    pub x: Option<f64>,
    pub y: Option<f64>,
    pub full_page: bool,
    pub width: Option<u32>,
    pub height: Option<u32>,
    pub wait_ms: Option<u64>,
    pub timeout: Duration,
}

pub struct Outcome {
    pub text: String,
    pub image: Option<Image>,
}

fn float(args: &Value, key: &str) -> Option<f64> {
    args.get(key)
        .and_then(|v| v.as_f64().or_else(|| v.as_str().and_then(|s| s.trim().parse().ok())))
        .filter(|f| f.is_finite())
}

fn size(args: &Value, key: &str) -> Result<Option<u32>, String> {
    match number(args, key) {
        None => Ok(None),
        Some(n) if (100..=4000).contains(&n) => Ok(Some(n as u32)),
        Some(n) => Err(format!("{key} {n} is out of range; use 100 to 4000 pixels.")),
    }
}

pub fn normalize_url(raw: &str, ws: &Workspace) -> Result<String, String> {
    let raw = raw.trim();
    if raw.is_empty() {
        return Err("url is empty.".to_string());
    }
    let lower = raw.to_ascii_lowercase();
    if lower.starts_with("data:") || lower == "about:blank" {
        return Ok(raw.to_string());
    }
    if let Ok(u) = tauri::Url::parse(raw) {
        match u.scheme() {
            "http" | "https" | "file" => return Ok(u.to_string()),
            s if s.len() > 1 && (raw.contains("://") || matches!(s, "javascript" | "chrome" | "about" | "blob" | "view-source")) => {
                return Err(format!("Only http, https and file pages can be opened, not {s}: links."));
            }
            _ => {}
        }
    }
    let local = ws.resolve(raw);
    if Path::new(raw).is_absolute() || local.path.is_file() {
        if !local.path.exists() {
            return Err(format!("{} does not exist.", local.path.display()));
        }
        return tauri::Url::from_file_path(&local.path)
            .map(|u| u.to_string())
            .map_err(|_| format!("{} cannot be opened as a page.", local.path.display()));
    }
    let host = lower.split(['/', ':']).next().unwrap_or("");
    if matches!(host, "localhost" | "0.0.0.0" | "[") || host.starts_with("127.") || lower.starts_with("[::1]") {
        Ok(format!("http://{raw}"))
    } else {
        Ok(format!("https://{raw}"))
    }
}

pub fn parse(args: &Value, chat: &str, ws: &Workspace) -> Result<Request, String> {
    let name = text(args, "action");
    let act = Act::parse(&name).ok_or_else(|| {
        format!(
            "Unknown browser action {:?}; use open, screenshot, click, type, press, scroll, eval, text, console, network, hover, select, wait, reload, back or close.",
            name
        )
    })?;
    let session: String = text(args, "session").trim().chars().take(64).collect();
    let mut r = Request {
        key: if session.is_empty() {
            chat.to_string()
        } else {
            format!("{chat}#{session}")
        },
        act,
        url: String::new(),
        selector: text(args, "selector").trim().to_string(),
        text: text(args, "text"),
        key_name: text(args, "key").trim().to_string(),
        script: text(args, "script"),
        x: float(args, "x"),
        y: float(args, "y"),
        full_page: flag(args, "full_page"),
        width: size(args, "width")?,
        height: size(args, "height")?,
        wait_ms: number(args, "wait_ms").map(|w| w.min(if act == Act::Wait { MAX_WAIT_FOR } else { MAX_WAIT })),
        timeout: Duration::from_secs(number(args, "timeout").unwrap_or(30).clamp(5, 120)),
    };
    match act {
        Act::Open => r.url = normalize_url(&text(args, "url"), ws)?,
        Act::Click | Act::Hover if r.selector.is_empty() && (r.x.is_none() || r.y.is_none()) => {
            let name = if act == Act::Click { "click" } else { "hover" };
            return Err(format!("{name} needs a selector, or both x and y."));
        }
        Act::Select if r.selector.is_empty() => return Err("select needs the selector of a <select> element.".to_string()),
        Act::Select if r.text.trim().is_empty() => return Err("select needs text: the option's value or visible text.".to_string()),
        Act::Wait if r.selector.is_empty() && r.text.trim().is_empty() => {
            return Err("wait needs a selector, or text to look for on the page.".to_string());
        }
        Act::Type if r.text.is_empty() && r.key_name.is_empty() => return Err("type needs text.".to_string()),
        Act::Press if r.key_name.is_empty() => return Err("press needs key, for example Enter.".to_string()),
        Act::Eval if r.script.trim().is_empty() => return Err("eval needs script.".to_string()),
        _ => {}
    }
    if !r.key_name.is_empty() {
        parse_key(&r.key_name)?;
    }
    Ok(r)
}

pub fn describe(r: &Request) -> (String, String, String) {
    let (summary, detail, lang) = match r.act {
        Act::Open => (format!("browser open {}", r.url), String::new(), ""),
        Act::Screenshot if !r.selector.is_empty() => (format!("browser screenshot {}", r.selector), String::new(), ""),
        Act::Screenshot if r.full_page => ("browser screenshot (full page)".to_string(), String::new(), ""),
        Act::Screenshot => ("browser screenshot".to_string(), String::new(), ""),
        Act::Click if !r.selector.is_empty() => (format!("browser click {}", r.selector), String::new(), ""),
        Act::Click => (
            format!("browser click at {}, {}", r.x.unwrap_or(0.0), r.y.unwrap_or(0.0)),
            String::new(),
            "",
        ),
        Act::Type if !r.selector.is_empty() => (format!("browser type into {}", r.selector), r.text.clone(), ""),
        Act::Type => ("browser type".to_string(), r.text.clone(), ""),
        Act::Press => (format!("browser press {}", r.key_name), String::new(), ""),
        Act::Scroll if !r.selector.is_empty() => (format!("browser scroll to {}", r.selector), String::new(), ""),
        Act::Scroll => ("browser scroll".to_string(), String::new(), ""),
        Act::Eval => (format!("browser eval {}", r.script.trim()), r.script.clone(), "javascript"),
        Act::Text => ("browser text".to_string(), String::new(), ""),
        Act::Console => ("browser console".to_string(), String::new(), ""),
        Act::Network => ("browser network".to_string(), String::new(), ""),
        Act::Hover if !r.selector.is_empty() => (format!("browser hover {}", r.selector), String::new(), ""),
        Act::Hover => (
            format!("browser hover at {}, {}", r.x.unwrap_or(0.0), r.y.unwrap_or(0.0)),
            String::new(),
            "",
        ),
        Act::Select => (format!("browser select {} in {}", r.text.trim(), r.selector), String::new(), ""),
        Act::Wait if !r.selector.is_empty() => (format!("browser wait for {}", r.selector), String::new(), ""),
        Act::Wait => (format!("browser wait for text \"{}\"", r.text.trim()), String::new(), ""),
        Act::Reload => ("browser reload".to_string(), String::new(), ""),
        Act::Back => ("browser back".to_string(), String::new(), ""),
        Act::Close => ("browser close".to_string(), String::new(), ""),
    };
    (first_line(&summary, 160), detail, lang.to_string())
}

#[derive(Debug, PartialEq, Eq)]
pub struct Key {
    key: String,
    code: String,
    key_code: u32,
    text: String,
    modifiers: u32,
}

pub fn parse_key(spec: &str) -> Result<Key, String> {
    let parts: Vec<&str> = spec.split('+').map(str::trim).collect();
    let (name, mods) = match parts.split_last() {
        Some((&"", rest)) if !rest.is_empty() => ("+", &rest[..rest.len() - 1]),
        Some((name, rest)) => (*name, rest),
        None => ("", &[][..]),
    };
    let mut modifiers = 0;
    for m in mods {
        modifiers |= match m.to_ascii_lowercase().as_str() {
            "alt" | "option" => 1,
            "ctrl" | "control" => 2,
            "meta" | "cmd" | "command" | "super" => 4,
            "shift" => 8,
            _ => return Err(format!("Unknown modifier {m} in key {spec}; use Alt, Control, Meta or Shift.")),
        };
    }
    let named: &[(&str, &str, u32, &str)] = &[
        ("Enter", "Enter", 13, "\r"),
        ("Tab", "Tab", 9, ""),
        ("Escape", "Escape", 27, ""),
        ("Backspace", "Backspace", 8, ""),
        ("Delete", "Delete", 46, ""),
        ("ArrowUp", "ArrowUp", 38, ""),
        ("ArrowDown", "ArrowDown", 40, ""),
        ("ArrowLeft", "ArrowLeft", 37, ""),
        ("ArrowRight", "ArrowRight", 39, ""),
        ("Home", "Home", 36, ""),
        ("End", "End", 35, ""),
        ("PageUp", "PageUp", 33, ""),
        ("PageDown", "PageDown", 34, ""),
        ("Insert", "Insert", 45, ""),
        (" ", "Space", 32, " "),
    ];
    let alias = match name.to_ascii_lowercase().as_str() {
        "return" => "enter".to_string(),
        "esc" => "escape".to_string(),
        "del" => "delete".to_string(),
        "space" | "spacebar" => " ".to_string(),
        "up" | "down" | "left" | "right" => format!("arrow{}", name.to_ascii_lowercase()),
        other => other.to_string(),
    };
    let mut key = if let Some((k, c, n, t)) = named.iter().find(|(k, ..)| k.to_ascii_lowercase() == alias) {
        Key {
            key: k.to_string(),
            code: c.to_string(),
            key_code: *n,
            text: t.to_string(),
            modifiers,
        }
    } else if let Some(n) = alias
        .strip_prefix('f')
        .and_then(|d| d.parse::<u32>().ok())
        .filter(|n| (1..=12).contains(n))
    {
        Key {
            key: format!("F{n}"),
            code: format!("F{n}"),
            key_code: 111 + n,
            text: String::new(),
            modifiers,
        }
    } else {
        let mut chars = name.chars();
        match (chars.next(), chars.next()) {
            (Some(c), None) => {
                let up = c.to_ascii_uppercase();
                let (code, key_code) = if c.is_ascii_alphabetic() {
                    (format!("Key{up}"), up as u32)
                } else if c.is_ascii_digit() {
                    (format!("Digit{c}"), c as u32)
                } else {
                    (String::new(), 0)
                };
                let shown = if modifiers & 8 != 0 { up } else { c };
                Key {
                    key: shown.to_string(),
                    code,
                    key_code,
                    text: shown.to_string(),
                    modifiers,
                }
            }
            _ => {
                return Err(format!(
                    "Unknown key {name}; use a single character or Enter, Tab, Escape, Backspace, Delete, ArrowUp, ArrowDown, ArrowLeft, ArrowRight, Home, End, PageUp, PageDown, Space or F1 to F12."
                ));
            }
        }
    };
    if modifiers & 0b0111 != 0 {
        key.text.clear();
    }
    Ok(key)
}

#[derive(Clone, Debug, PartialEq)]
pub struct Entry {
    pub level: String,
    pub text: String,
    pub count: u32,
}

impl Entry {
    pub fn new(level: &str, text: impl Into<String>) -> Entry {
        Entry {
            level: level.to_string(),
            text: text.into(),
            count: 1,
        }
    }

    fn problem(&self) -> bool {
        matches!(self.level.as_str(), "error" | "warn" | "exception" | "failed")
    }
}

fn console_level(kind: &str) -> &'static str {
    match kind {
        "warning" | "warn" => "warn",
        "error" | "assert" => "error",
        "info" => "info",
        "debug" | "verbose" => "debug",
        _ => "log",
    }
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct NetEntry {
    pub method: String,
    pub status: Option<i64>,
    pub kind: String,
    pub url: String,
    pub ms: Option<u64>,
    pub error: String,
}

#[derive(Clone, Default)]
struct Req {
    method: String,
    url: String,
    kind: String,
    start: f64,
    status: Option<i64>,
}

impl Req {
    fn done(self, end: Option<f64>, error: &str) -> NetEntry {
        let ms = end
            .filter(|_| self.start > 0.0)
            .map(|e| ((e - self.start).max(0.0) * 1000.0).round() as u64);
        NetEntry {
            method: self.method,
            status: self.status,
            kind: self.kind,
            url: self.url,
            ms,
            error: error.to_string(),
        }
    }
}

pub fn format_requests(entries: &[NetEntry]) -> String {
    let mut out = Vec::new();
    let skip = entries.len().saturating_sub(BUFFER);
    if skip > 0 {
        out.push(format!("[... {skip} earlier requests ...]"));
    }
    for e in &entries[skip..] {
        let status = match (e.status, e.error.is_empty(), e.ms) {
            (Some(code), ..) => code.to_string(),
            (None, false, _) => "failed".to_string(),
            (None, true, None) => "pending".to_string(),
            (None, true, Some(_)) => "-".to_string(),
        };
        let mut line = format!("{} {status} {} {}", e.method, e.kind, clip(&e.url, 200));
        if let Some(ms) = e.ms {
            line.push_str(&format!(" {ms}ms"));
        }
        if !e.error.is_empty() {
            line.push_str(&format!(" {}", e.error));
        }
        out.push(line);
    }
    out.join("\n")
}

fn clip(s: &str, max: usize) -> String {
    if s.chars().count() <= max {
        return s.to_string();
    }
    let mut out: String = s.chars().take(max).collect();
    out.push_str(" …");
    out
}

fn push_capped(q: &mut VecDeque<Entry>, e: Entry) {
    if let Some(last) = q.back_mut() {
        if last.level == e.level && last.text == e.text {
            last.count += 1;
            return;
        }
    }
    if q.len() >= BUFFER {
        q.pop_front();
    }
    q.push_back(e);
}

pub fn format_entries(entries: &[Entry], max: usize) -> String {
    let mut out = Vec::new();
    let skip = entries.len().saturating_sub(max);
    if skip > 0 {
        out.push(format!("[... {skip} earlier entries ...]"));
    }
    for e in &entries[skip..] {
        let times = if e.count > 1 { format!(" (x{})", e.count) } else { String::new() };
        out.push(format!("{}: {}{}", e.level, clip(&e.text, 500), times));
    }
    out.join("\n")
}

pub fn format_problems(entries: &[Entry], max: usize) -> String {
    format_entries(entries, max)
        .lines()
        .map(|l| format!("- {l}"))
        .collect::<Vec<_>>()
        .join("\n")
}

pub fn state_line(url: &str, status: Option<i64>, error: &str, title: &str, loading: bool) -> String {
    let mut s = if url.is_empty() {
        "about:blank".to_string()
    } else {
        clip(url, 300)
    };
    if !error.is_empty() {
        s.push_str(&format!(" ({error})"));
    } else if let Some(code) = status.filter(|c| *c > 0) {
        s.push_str(&format!(" ({code})"));
    }
    s.push_str(&format!(" \"{}\"", clip(title.trim(), 120)));
    if loading {
        s.push_str(" (still loading)");
    }
    s
}

fn remote_text(v: &Value) -> String {
    if let Some(val) = v.get("value") {
        return match val {
            Value::String(s) => s.clone(),
            other => other.to_string(),
        };
    }
    if let Some(props) = v["preview"]["properties"].as_array() {
        let inner: Vec<String> = props
            .iter()
            .map(|p| format!("{}: {}", p["name"].as_str().unwrap_or(""), p["value"].as_str().unwrap_or("")))
            .collect();
        let (open, close) = if v["subtype"] == "array" { ("[", "]") } else { ("{", "}") };
        return format!("{open}{}{close}", inner.join(", "));
    }
    v.get("description")
        .or_else(|| v.get("unserializableValue"))
        .and_then(Value::as_str)
        .unwrap_or_else(|| v["type"].as_str().unwrap_or(""))
        .to_string()
}

fn place(url: &str, line: Option<i64>) -> String {
    match (url.is_empty(), line) {
        (true, _) => String::new(),
        (false, Some(l)) => format!(" ({url}:{})", l + 1),
        (false, None) => format!(" ({url})"),
    }
}

pub fn exception_text(d: &Value) -> String {
    let ex = &d["exception"];
    let desc = ex["description"].as_str().map(String::from).or_else(|| {
        ex.get("value")
            .map(|v| v.as_str().map(String::from).unwrap_or_else(|| v.to_string()))
    });
    match desc {
        Some(desc) => {
            let mut lines = desc.lines();
            let msg = lines.next().unwrap_or("").trim().to_string();
            let msg = if msg.starts_with("Uncaught") {
                msg
            } else {
                format!("Uncaught {msg}")
            };
            match lines.map(str::trim).find(|l| l.starts_with("at ")) {
                Some(at) => format!("{msg} {at}"),
                None => format!("{msg}{}", place(d["url"].as_str().unwrap_or(""), d["lineNumber"].as_i64())),
            }
        }
        None => format!(
            "{}{}",
            d["text"].as_str().unwrap_or("Uncaught exception"),
            place(d["url"].as_str().unwrap_or(""), d["lineNumber"].as_i64())
        ),
    }
}

#[derive(Default)]
pub struct PageState {
    page: String,
    main_frame: String,
    url: String,
    main_request: String,
    status: Option<i64>,
    doc_error: String,
    loaded: bool,
    navs: u64,
    inflight: HashSet<String>,
    activity: Option<Instant>,
    requests: HashMap<String, Req>,
    net: VecDeque<NetEntry>,
    log: VecDeque<Entry>,
    unseen: VecDeque<Entry>,
}

impl PageState {
    fn push(&mut self, e: Entry) {
        if e.problem() {
            push_capped(&mut self.unseen, e.clone());
        }
        push_capped(&mut self.log, e);
    }

    fn record(&mut self, e: NetEntry) {
        if self.net.len() >= BUFFER {
            self.net.pop_front();
        }
        self.net.push_back(e);
    }

    fn touch(&mut self) {
        self.activity = Some(Instant::now());
    }

    pub fn on_event(&mut self, method: &str, p: &Value) {
        let id = p["requestId"].as_str().unwrap_or("").to_string();
        match method {
            "Runtime.consoleAPICalled" => {
                let level = console_level(p["type"].as_str().unwrap_or("log"));
                let args: Vec<String> = p["args"]
                    .as_array()
                    .map(|a| a.iter().map(remote_text).collect())
                    .unwrap_or_default();
                let frame = &p["stackTrace"]["callFrames"][0];
                let at = place(frame["url"].as_str().unwrap_or(""), frame["lineNumber"].as_i64());
                self.push(Entry::new(level, format!("{}{at}", args.join(" "))));
            }
            "Runtime.exceptionThrown" => self.push(Entry::new("exception", exception_text(&p["exceptionDetails"]))),
            "Log.entryAdded" => {
                let e = &p["entry"];
                if e["source"] == "network" {
                    return;
                }
                let level = console_level(e["level"].as_str().unwrap_or("info"));
                let at = place(e["url"].as_str().unwrap_or(""), e["lineNumber"].as_i64());
                self.push(Entry::new(level, format!("{}{at}", e["text"].as_str().unwrap_or(""))));
            }
            "Network.requestWillBeSent" => {
                if self.requests.len() > 2000 {
                    self.requests.clear();
                }
                if self.inflight.len() > 500 {
                    self.inflight.clear();
                }
                let at = p["timestamp"].as_f64();
                if let Some(redirect) = p["redirectResponse"].as_object() {
                    if let Some(mut old) = self.requests.remove(&id) {
                        old.status = redirect.get("status").and_then(Value::as_i64);
                        self.record(old.done(at, ""));
                    }
                }
                let req = &p["request"];
                self.requests.insert(
                    id.clone(),
                    Req {
                        method: req["method"].as_str().unwrap_or("GET").to_string(),
                        url: req["url"].as_str().unwrap_or("").to_string(),
                        kind: p["type"].as_str().unwrap_or("other").to_ascii_lowercase(),
                        start: at.unwrap_or(0.0),
                        status: None,
                    },
                );
                self.inflight.insert(id.clone());
                self.touch();
                if p["type"] == "Document" && p["frameId"].as_str() == Some(&self.main_frame) && p["loaderId"].as_str() == Some(&id) {
                    self.main_request = id;
                    self.status = None;
                    self.doc_error.clear();
                }
            }
            "Network.responseReceived" => {
                let status = p["response"]["status"].as_i64().unwrap_or(0);
                if id == self.main_request {
                    self.status = Some(status);
                }
                let req = self.requests.entry(id.clone()).or_default();
                req.status = Some(status);
                if req.url.is_empty() {
                    req.url = p["response"]["url"].as_str().unwrap_or("").to_string();
                }
                if status >= 400 {
                    let line = format!("{status} {} {}", req.method, clip(&req.url, 300));
                    self.push(Entry::new("failed", line));
                }
            }
            "Network.loadingFinished" => {
                self.inflight.remove(&id);
                self.touch();
                if let Some(req) = self.requests.remove(&id) {
                    self.record(req.done(p["timestamp"].as_f64(), ""));
                }
            }
            "Network.loadingFailed" => {
                self.inflight.remove(&id);
                self.touch();
                let error = p["errorText"].as_str().unwrap_or("failed");
                if id == self.main_request {
                    self.doc_error = error.to_string();
                }
                let req = self.requests.remove(&id).unwrap_or_default();
                if p["canceled"].as_bool() != Some(true) && p["blockedReason"].is_null() {
                    self.push(Entry::new("failed", format!("{} {} {error}", req.method, clip(&req.url, 300))));
                }
                self.record(req.done(p["timestamp"].as_f64(), error));
            }
            "Page.frameStartedLoading" if p["frameId"].as_str() == Some(&self.main_frame) => {
                self.loaded = false;
                self.navs += 1;
            }
            "Page.loadEventFired" => self.loaded = true,
            "Page.frameNavigated" if p["frame"]["parentId"].is_null() => {
                self.url = p["frame"]["url"].as_str().unwrap_or("").to_string();
            }
            "Page.navigatedWithinDocument" if p["frameId"].as_str() == Some(&self.main_frame) => {
                self.url = p["url"].as_str().unwrap_or("").to_string();
            }
            "Page.javascriptDialogOpening" => {
                let kind = p["type"].as_str().unwrap_or("alert");
                self.push(Entry::new(
                    "info",
                    format!("{kind} dialog \"{}\" was accepted", clip(p["message"].as_str().unwrap_or(""), 300)),
                ));
            }
            _ => {}
        }
    }
}

fn split_paths(env: &dyn Fn(&str) -> Option<String>) -> Vec<PathBuf> {
    env("PATH").map(|p| std::env::split_paths(&p).collect()).unwrap_or_default()
}

fn on_path(name: &str, env: &dyn Fn(&str) -> Option<String>, exists: &dyn Fn(&Path) -> bool) -> Option<PathBuf> {
    let exts: &[&str] = if cfg!(windows) { &[".exe", ""] } else { &[""] };
    split_paths(env)
        .into_iter()
        .flat_map(|dir| exts.iter().map(move |e| dir.join(format!("{name}{e}"))))
        .find(|p| exists(p))
}

#[cfg(windows)]
fn candidates(env: &dyn Fn(&str) -> Option<String>) -> Vec<PathBuf> {
    let bases: Vec<PathBuf> = ["PROGRAMFILES", "PROGRAMFILES(X86)", "LOCALAPPDATA"]
        .iter()
        .filter_map(|k| env(k))
        .map(PathBuf::from)
        .collect();
    [
        "Google\\Chrome\\Application\\chrome.exe",
        "Chromium\\Application\\chrome.exe",
        "Microsoft\\Edge\\Application\\msedge.exe",
        "BraveSoftware\\Brave-Browser\\Application\\brave.exe",
    ]
    .iter()
    .flat_map(|rel| bases.iter().map(move |b| b.join(rel)))
    .collect()
}

#[cfg(target_os = "macos")]
fn candidates(env: &dyn Fn(&str) -> Option<String>) -> Vec<PathBuf> {
    let mut roots = vec![PathBuf::from("/Applications")];
    if let Some(home) = env("HOME") {
        roots.push(Path::new(&home).join("Applications"));
    }
    ["Google Chrome", "Chromium", "Microsoft Edge", "Brave Browser"]
        .iter()
        .flat_map(|app| roots.iter().map(move |r| r.join(format!("{app}.app/Contents/MacOS/{app}"))))
        .collect()
}

#[cfg(not(any(windows, target_os = "macos")))]
fn candidates(env: &dyn Fn(&str) -> Option<String>) -> Vec<PathBuf> {
    let dirs = split_paths(env);
    [
        "google-chrome",
        "google-chrome-stable",
        "chromium",
        "chromium-browser",
        "microsoft-edge",
        "microsoft-edge-stable",
        "brave-browser",
    ]
    .iter()
    .flat_map(|name| dirs.iter().map(move |d| d.join(name)))
    .collect()
}

pub fn find_with(env: &dyn Fn(&str) -> Option<String>, exists: &dyn Fn(&Path) -> bool) -> Result<PathBuf, String> {
    if let Some(v) = env("SALAM_AGENT_BROWSER").filter(|v| !v.trim().is_empty()) {
        let p = PathBuf::from(v.trim());
        if exists(&p) {
            return Ok(p);
        }
        return on_path(v.trim(), env, exists)
            .ok_or_else(|| format!("SALAM_AGENT_BROWSER is set to {v}, but no browser program was found there."));
    }
    candidates(env).into_iter().find(|p| exists(p)).ok_or_else(|| NOT_FOUND.to_string())
}

pub fn find_browser() -> Result<PathBuf, String> {
    find_with(&|k| std::env::var(k).ok(), &|p| p.is_file())
}

pub fn launch_args(dir: &Path, no_sandbox: bool) -> Vec<String> {
    let mut args: Vec<String> = [
        "--headless=new",
        "--remote-debugging-port=0",
        "--no-first-run",
        "--no-default-browser-check",
        "--disable-extensions",
        "--hide-scrollbars",
        "--mute-audio",
        "--disable-crash-reporter",
        "--window-size=1280,800",
    ]
    .iter()
    .map(|s| s.to_string())
    .collect();
    args.push(format!("--user-data-dir={}", dir.display()));
    args.push(format!("--crash-dumps-dir={}", dir.join("crashes").display()));
    if no_sandbox {
        args.push("--no-sandbox".to_string());
    }
    args.push("about:blank".to_string());
    args
}

fn without_sandbox() -> bool {
    #[cfg(target_os = "linux")]
    let root = unsafe { libc::geteuid() == 0 };
    #[cfg(not(target_os = "linux"))]
    let root = false;
    root || std::env::var("SALAM_AGENT_BROWSER_NO_SANDBOX").is_ok_and(|v| v == "1")
}

static PROCS: LazyLock<Mutex<HashMap<u32, PathBuf>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static SLOTS: LazyLock<Mutex<HashMap<String, Arc<Slot>>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static REAPER: Once = Once::new();
static PROFILES: AtomicU64 = AtomicU64::new(0);

struct Slot {
    session: tokio::sync::Mutex<Option<Session>>,
    used: Mutex<Instant>,
}

fn kill_strays(dir: &Path) {
    #[cfg(target_os = "linux")]
    {
        let needle = dir.to_string_lossy().into_owned();
        let me = std::process::id();
        for e in std::fs::read_dir("/proc").into_iter().flatten().flatten() {
            let Some(pid) = e.file_name().to_str().and_then(|n| n.parse::<i32>().ok()) else {
                continue;
            };
            if pid as u32 == me {
                continue;
            }
            if let Ok(cmd) = std::fs::read(e.path().join("cmdline")) {
                if String::from_utf8_lossy(&cmd).contains(&needle) {
                    unsafe { libc::kill(pid, libc::SIGKILL) };
                }
            }
        }
    }
    #[cfg(target_os = "macos")]
    {
        let _ = std::process::Command::new("pkill")
            .arg("-KILL")
            .arg("-f")
            .arg(dir)
            .stdout(std::process::Stdio::null())
            .stderr(std::process::Stdio::null())
            .status();
    }
    #[cfg(windows)]
    let _ = dir;
}

fn remove_profile(dir: &Path) {
    for _ in 0..20 {
        if std::fs::remove_dir_all(dir).is_ok() || !dir.exists() {
            return;
        }
        std::thread::sleep(Duration::from_millis(250));
    }
}

struct Proc {
    child: tokio::process::Child,
    pid: Option<u32>,
    dir: PathBuf,
}

impl Drop for Proc {
    fn drop(&mut self) {
        exec::kill_tree(self.pid);
        let _ = self.child.start_kill();
        if let Some(pid) = self.pid {
            PROCS.lock().unwrap().remove(&pid);
        }
        let dir = self.dir.clone();
        std::thread::spawn(move || {
            kill_strays(&dir);
            remove_profile(&dir);
        });
    }
}

pub fn close_all() {
    let procs: Vec<(u32, PathBuf)> = PROCS.lock().unwrap().drain().collect();
    for (pid, _) in &procs {
        exec::kill_tree(Some(*pid));
    }
    for (_, dir) in &procs {
        kill_strays(dir);
        for _ in 0..8 {
            if std::fs::remove_dir_all(dir).is_ok() || !dir.exists() {
                break;
            }
            std::thread::sleep(Duration::from_millis(100));
        }
    }
}

fn start_reaper() {
    REAPER.call_once(|| {
        tokio::spawn(async {
            loop {
                tokio::time::sleep(Duration::from_secs(60)).await;
                let slots: Vec<Arc<Slot>> = SLOTS.lock().unwrap().values().cloned().collect();
                for slot in slots {
                    if slot.used.lock().unwrap().elapsed() < IDLE {
                        continue;
                    }
                    if let Ok(mut s) = slot.session.try_lock() {
                        s.take();
                    }
                }
            }
        });
    });
}

fn slot(key: &str) -> Arc<Slot> {
    SLOTS
        .lock()
        .unwrap()
        .entry(key.to_string())
        .or_insert_with(|| {
            Arc::new(Slot {
                session: tokio::sync::Mutex::new(None),
                used: Mutex::new(Instant::now()),
            })
        })
        .clone()
}

struct Shot {
    data: String,
    width: u32,
    height: u32,
}

struct Target {
    x: f64,
    y: f64,
    left: f64,
    top: f64,
    w: f64,
    h: f64,
    label: String,
}

struct Session {
    proc: Proc,
    cdp: Cdp,
    page: String,
    state: Arc<Mutex<PageState>>,
    width: u32,
    height: u32,
    user_agent: String,
    version: String,
}

fn selector_js(selector: &str) -> String {
    serde_json::to_string(selector).unwrap_or_else(|_| "\"\"".to_string())
}

impl Session {
    async fn launch() -> Result<Session, String> {
        let exe = find_browser()?;
        let dir = std::env::temp_dir().join(format!(
            "salam-agent-browser-{}-{}",
            std::process::id(),
            PROFILES.fetch_add(1, Ordering::Relaxed)
        ));
        let _ = std::fs::remove_dir_all(&dir);
        std::fs::create_dir_all(&dir).map_err(|e| format!("Could not create a browser profile folder: {e}"))?;
        let log = dir.join("browser-errors.log");
        let stderr = std::fs::File::create(&log)
            .map(std::process::Stdio::from)
            .unwrap_or_else(|_| std::process::Stdio::null());
        let mut c = exec::command(&exe.to_string_lossy());
        c.args(launch_args(&dir, without_sandbox()))
            .stdin(std::process::Stdio::null())
            .stdout(std::process::Stdio::null())
            .stderr(stderr);
        let mut c = tokio::process::Command::from(c);
        c.kill_on_drop(true);
        #[cfg(unix)]
        c.process_group(0);
        let child = match c.spawn() {
            Ok(child) => child,
            Err(e) => {
                remove_profile(&dir);
                return Err(format!("Could not start the browser {}: {e}", exe.display()));
            }
        };
        let pid = child.id();
        if let Some(pid) = pid {
            PROCS.lock().unwrap().insert(pid, dir.clone());
        }
        let mut proc = Proc { child, pid, dir };
        let port_file = proc.dir.join("DevToolsActivePort");
        let started = Instant::now();
        let (port, path) = loop {
            if let Some(found) = std::fs::read_to_string(&port_file).ok().and_then(|t| cdp::parse_active_port(&t)) {
                break found;
            }
            if let Ok(Some(status)) = proc.child.try_wait() {
                let errors = std::fs::read_to_string(&log).unwrap_or_default();
                if errors.contains("No usable sandbox") {
                    return Err(format!(
                        "The browser {} cannot start its sandbox on this computer (Ubuntu and some other Linux systems block it for browsers that are not installed as a package). \
                         Install Google Chrome or Chromium from a package, or set SALAM_AGENT_BROWSER_NO_SANDBOX=1 to run it without the sandbox.",
                        exe.display()
                    ));
                }
                return Err(format!("The browser {} stopped right after starting ({status}).", exe.display()));
            }
            if started.elapsed() > Duration::from_secs(20) {
                return Err(format!("The browser {} did not start within 20 seconds.", exe.display()));
            }
            tokio::time::sleep(Duration::from_millis(100)).await;
        };
        let state = Arc::new(Mutex::new(PageState::default()));
        let events = state.clone();
        let handler: cdp::Handler = Arc::new(move |cdp: &Cdp, session: Option<&str>, method: &str, params: &Value| {
            let mut s = events.lock().unwrap();
            if session.is_none() || session != Some(s.page.as_str()) {
                return;
            }
            s.on_event(method, params);
            if method == "Page.javascriptDialogOpening" {
                cdp.send(
                    session,
                    "Page.handleJavaScriptDialog",
                    json!({ "accept": true, "promptText": params["defaultPrompt"].as_str().unwrap_or("") }),
                );
            }
        });
        let cdp = Cdp::connect(&format!("ws://127.0.0.1:{port}{path}"), handler).await?;
        let target = cdp.call(None, "Target.createTarget", json!({ "url": "about:blank" })).await?;
        let target = target["targetId"].as_str().unwrap_or("").to_string();
        let attached = cdp
            .call(None, "Target.attachToTarget", json!({ "targetId": target, "flatten": true }))
            .await?;
        let page = attached["sessionId"].as_str().unwrap_or("").to_string();
        if page.is_empty() {
            return Err("The browser did not open a page.".to_string());
        }
        state.lock().unwrap().page = page.clone();
        let p = Some(page.as_str());
        let tree = cdp.call(p, "Page.getFrameTree", json!({})).await?;
        state.lock().unwrap().main_frame = tree["frameTree"]["frame"]["id"].as_str().unwrap_or("").to_string();
        for m in ["Page.enable", "Runtime.enable", "Log.enable", "Network.enable"] {
            cdp.call(p, m, json!({})).await?;
        }
        cdp.call(p, "Page.addScriptToEvaluateOnNewDocument", json!({ "source": ONE_TAB }))
            .await?;
        let version = cdp.call(None, "Browser.getVersion", json!({})).await?;
        let user_agent = version["userAgent"].as_str().unwrap_or("").replace("HeadlessChrome", "Chrome");
        let version = version["product"]
            .as_str()
            .and_then(|p| p.split('/').nth(1))
            .and_then(|v| v.split('.').next())
            .unwrap_or("130")
            .to_string();
        let mut s = Session {
            proc,
            cdp,
            page,
            state,
            width: 1280,
            height: 800,
            user_agent,
            version,
        };
        s.apply_viewport().await?;
        Ok(s)
    }

    fn dead(&mut self) -> bool {
        self.cdp.closed() || !matches!(self.proc.child.try_wait(), Ok(None))
    }

    async fn call(&self, method: &str, params: Value) -> Result<Value, String> {
        self.cdp.call(Some(&self.page), method, params).await
    }

    async fn apply_viewport(&mut self) -> Result<(), String> {
        let mobile = self.width <= 500;
        self.call(
            "Emulation.setDeviceMetricsOverride",
            json!({ "width": self.width, "height": self.height, "deviceScaleFactor": 1, "mobile": mobile }),
        )
        .await?;
        let touch = if mobile {
            json!({ "enabled": true, "maxTouchPoints": 5 })
        } else {
            json!({ "enabled": false })
        };
        self.call("Emulation.setTouchEmulationEnabled", touch).await?;
        let ua = if mobile {
            format!(
                "Mozilla/5.0 (Linux; Android 14; Pixel 8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/{}.0.0.0 Mobile Safari/537.36",
                self.version
            )
        } else {
            self.user_agent.clone()
        };
        if !ua.is_empty() {
            self.call("Emulation.setUserAgentOverride", json!({ "userAgent": ua })).await?;
        }
        Ok(())
    }

    async fn evaluate(&self, expression: &str, repl: bool) -> Result<Value, String> {
        let r = self
            .call(
                "Runtime.evaluate",
                json!({ "expression": expression, "awaitPromise": true, "returnByValue": true, "userGesture": true, "replMode": repl }),
            )
            .await?;
        if let Some(d) = r.get("exceptionDetails") {
            return Err(exception_text(d));
        }
        Ok(r["result"].clone())
    }

    async fn value(&self, expression: &str) -> Result<Value, String> {
        Ok(self.evaluate(expression, false).await?.get("value").cloned().unwrap_or(Value::Null))
    }

    fn reset_page(&self) {
        let mut s = self.state.lock().unwrap();
        s.unseen.clear();
        s.loaded = false;
        s.status = None;
        s.doc_error.clear();
    }

    fn navs(&self) -> u64 {
        self.state.lock().unwrap().navs
    }

    async fn wait_load(&self, max: Duration) {
        let started = Instant::now();
        while !self.state.lock().unwrap().loaded && started.elapsed() < max {
            tokio::time::sleep(Duration::from_millis(50)).await;
        }
    }

    async fn settle(&self, max: Duration) {
        let started = Instant::now();
        loop {
            tokio::time::sleep(Duration::from_millis(100)).await;
            let quiet = {
                let s = self.state.lock().unwrap();
                s.inflight.is_empty() && s.activity.is_none_or(|a| a.elapsed() >= Duration::from_millis(500))
            };
            if (quiet && started.elapsed() >= Duration::from_millis(300)) || started.elapsed() >= max {
                return;
            }
        }
    }

    async fn after(&self, navs: u64, wait: Duration) {
        tokio::time::sleep(Duration::from_millis(250)).await;
        if self.navs() != navs {
            self.wait_load(Duration::from_secs(10)).await;
        }
        self.settle(wait).await;
    }

    async fn locate(&self, selector: &str) -> Result<Target, String> {
        let script = format!(
            "(() => {{ const e = document.querySelector({}); if (!e) return null; \
             e.scrollIntoView({{ block: 'center', inline: 'center', behavior: 'instant' }}); const r = e.getBoundingClientRect(); \
             return {{ x: r.left + r.width / 2, y: r.top + r.height / 2, left: r.left + scrollX, top: r.top + scrollY, w: r.width, h: r.height, \
             label: e.tagName.toLowerCase() + ' \"' + (e.innerText || e.value || e.getAttribute('aria-label') || '').replace(/\\s+/g, ' ').trim().slice(0, 60) + '\"' }}; }})()",
            selector_js(selector)
        );
        let v = self
            .value(&script)
            .await
            .map_err(|e| format!("The selector {selector} did not work: {e}"))?;
        if v.is_null() {
            return Err(format!("No element matches the selector {selector}."));
        }
        let f = |k: &str| v[k].as_f64().unwrap_or(0.0);
        if f("w") <= 0.0 || f("h") <= 0.0 {
            return Err(format!("The element {selector} is not visible (it has no size)."));
        }
        Ok(Target {
            x: f("x"),
            y: f("y"),
            left: f("left"),
            top: f("top"),
            w: f("w"),
            h: f("h"),
            label: v["label"].as_str().unwrap_or("").to_string(),
        })
    }

    async fn click_at(&self, x: f64, y: f64) -> Result<(), String> {
        self.call("Input.dispatchMouseEvent", json!({ "type": "mouseMoved", "x": x, "y": y }))
            .await?;
        for kind in ["mousePressed", "mouseReleased"] {
            let buttons = if kind == "mousePressed" { 1 } else { 0 };
            self.call(
                "Input.dispatchMouseEvent",
                json!({ "type": kind, "x": x, "y": y, "button": "left", "buttons": buttons, "clickCount": 1 }),
            )
            .await?;
        }
        Ok(())
    }

    async fn press(&self, spec: &str) -> Result<(), String> {
        let k = parse_key(spec)?;
        let mut down = json!({
            "type": if k.text.is_empty() { "rawKeyDown" } else { "keyDown" },
            "modifiers": k.modifiers,
            "key": k.key,
            "code": k.code,
            "windowsVirtualKeyCode": k.key_code,
        });
        if !k.text.is_empty() {
            down["text"] = json!(k.text);
            down["unmodifiedText"] = json!(k.text);
        }
        self.call("Input.dispatchKeyEvent", down).await?;
        self.call(
            "Input.dispatchKeyEvent",
            json!({ "type": "keyUp", "modifiers": k.modifiers, "key": k.key, "code": k.code, "windowsVirtualKeyCode": k.key_code }),
        )
        .await?;
        Ok(())
    }

    async fn capture(&self, full: bool, selector: &str) -> Result<Shot, String> {
        let (w, h, clip) = if !selector.is_empty() {
            let t = self.locate(selector).await?;
            let (w, h) = (t.w.ceil().min(4000.0), t.h.ceil().min(MAX_PAGE_HEIGHT));
            (w, h, Some((t.left, t.top)))
        } else if full {
            let m = self.call("Page.getLayoutMetrics", json!({})).await?;
            let size = if m["cssContentSize"].is_object() {
                &m["cssContentSize"]
            } else {
                &m["contentSize"]
            };
            let h = size["height"].as_f64().unwrap_or(self.height as f64).ceil();
            (self.width as f64, h.clamp(1.0, MAX_PAGE_HEIGHT), Some((0.0, 0.0)))
        } else {
            (self.width as f64, self.height as f64, None)
        };
        for (quality, scale) in [(70, 1.0), (50, 1.0), (35, 1.0), (35, 0.5), (25, 0.35)] {
            let mut params = json!({ "format": "jpeg", "quality": quality, "fromSurface": true });
            if clip.is_some() || scale < 1.0 {
                let (x, y) = clip.unwrap_or((0.0, 0.0));
                params["clip"] = json!({ "x": x, "y": y, "width": w, "height": h, "scale": scale });
                params["captureBeyondViewport"] = json!(clip.is_some());
            }
            let r = self.call("Page.captureScreenshot", params).await?;
            let data = r["data"].as_str().unwrap_or("").to_string();
            if data.is_empty() {
                return Err("The browser returned an empty screenshot.".to_string());
            }
            if data.len() <= MAX_IMAGE {
                return Ok(Shot {
                    data,
                    width: (w * scale).round() as u32,
                    height: (h * scale).round() as u32,
                });
            }
        }
        Err("The screenshot is too large to send even at low quality; try a smaller viewport or a selector.".to_string())
    }

    async fn finish(&self, body: &str, shot: Option<Shot>) -> Outcome {
        let here = tokio::time::timeout(Duration::from_secs(3), self.value("[location.href, document.title]"))
            .await
            .ok()
            .and_then(Result::ok);
        let (line, problems) = {
            let mut s = self.state.lock().unwrap();
            let url = here.as_ref().and_then(|v| v[0].as_str()).unwrap_or(&s.url).to_string();
            let title = here.as_ref().and_then(|v| v[1].as_str()).unwrap_or("");
            let line = state_line(&url, s.status, &s.doc_error, title, !s.loaded);
            (line, s.unseen.drain(..).collect::<Vec<_>>())
        };
        let mut text = line;
        if !body.is_empty() {
            text.push('\n');
            text.push_str(body);
        }
        if let Some(s) = &shot {
            text.push_str(&format!("\nScreenshot attached ({}x{} px, JPEG).", s.width, s.height));
        }
        if !problems.is_empty() {
            text.push_str("\n\nProblems seen:\n");
            text.push_str(&format_problems(&problems, 30));
        }
        Outcome {
            text,
            image: shot.map(|s| Image {
                data: s.data,
                mime: "image/jpeg",
            }),
        }
    }

    async fn run(&mut self, r: &Request) -> Result<Outcome, String> {
        if r.width.is_some() || r.height.is_some() {
            self.width = r.width.unwrap_or(self.width);
            self.height = r.height.unwrap_or(self.height);
            self.apply_viewport().await?;
        }
        let wait = |default: u64| Duration::from_millis(r.wait_ms.unwrap_or(default));
        let navs = self.navs();
        match r.act {
            Act::Open => {
                self.reset_page();
                let nav = self.call("Page.navigate", json!({ "url": r.url })).await?;
                if let Some(err) = nav["errorText"].as_str().filter(|e| !e.is_empty()) {
                    return Err(format!("Could not open {}: {err}.", r.url));
                }
                if nav["loaderId"].is_string() {
                    self.wait_load(Duration::from_secs(20)).await;
                } else {
                    self.state.lock().unwrap().loaded = true;
                }
                self.settle(wait(DEFAULT_WAIT)).await;
                let shot = self.capture(false, "").await?;
                Ok(self.finish("", Some(shot)).await)
            }
            Act::Screenshot => {
                let shot = self.capture(r.full_page, &r.selector).await?;
                Ok(self.finish("", Some(shot)).await)
            }
            Act::Click => {
                let (x, y, what) = if r.selector.is_empty() {
                    let (x, y) = (r.x.unwrap_or(0.0), r.y.unwrap_or(0.0));
                    (x, y, format!("Clicked at ({x}, {y})."))
                } else {
                    let t = self.locate(&r.selector).await?;
                    (t.x, t.y, format!("Clicked {} at ({}, {}).", t.label, t.x.round(), t.y.round()))
                };
                self.click_at(x, y).await?;
                self.after(navs, wait(1000)).await;
                let shot = self.capture(false, "").await?;
                Ok(self.finish(&what, Some(shot)).await)
            }
            Act::Type => {
                let mut what = String::new();
                if !r.selector.is_empty() {
                    let script = format!(
                        "(() => {{ const e = document.querySelector({}); if (!e) return null; e.scrollIntoView({{ block: 'center', behavior: 'instant' }}); e.focus(); \
                         if (typeof e.value === 'string' && typeof e.setSelectionRange === 'function') {{ try {{ e.setSelectionRange(e.value.length, e.value.length); }} catch (_) {{}} }} \
                         return document.activeElement === e || e.contains(document.activeElement); }})()",
                        selector_js(&r.selector)
                    );
                    match self.value(&script).await {
                        Ok(Value::Bool(true)) => {}
                        Ok(Value::Null) => return Err(format!("No element matches the selector {}.", r.selector)),
                        Ok(_) => return Err(format!("{} could not take the keyboard focus.", r.selector)),
                        Err(e) => return Err(format!("The selector {} did not work: {e}", r.selector)),
                    }
                }
                if !r.text.is_empty() {
                    self.call("Input.insertText", json!({ "text": r.text })).await?;
                    let count = r.text.chars().count();
                    what = format!("Typed {count} character{}", if count == 1 { "" } else { "s" });
                    if !r.selector.is_empty() {
                        what.push_str(&format!(" into {}", r.selector));
                    }
                    if let Ok(Value::String(v)) = self.value("document.activeElement && document.activeElement.value").await {
                        what.push_str(&format!("; its value is now \"{}\"", clip(&v, 200)));
                    }
                    what.push('.');
                }
                if !r.key_name.is_empty() {
                    self.press(&r.key_name).await?;
                    what.push_str(&format!(" Pressed {}.", r.key_name));
                }
                self.after(navs, wait(600)).await;
                let shot = self.capture(false, "").await?;
                Ok(self.finish(what.trim(), Some(shot)).await)
            }
            Act::Press => {
                self.press(&r.key_name).await?;
                self.after(navs, wait(600)).await;
                Ok(self.finish(&format!("Pressed {}.", r.key_name), None).await)
            }
            Act::Scroll => {
                if !r.selector.is_empty() {
                    self.locate(&r.selector).await?;
                } else {
                    let dy = r.y.map(|y| y.to_string()).unwrap_or_else(|| "innerHeight".to_string());
                    self.value(&format!("window.scrollBy({}, {dy})", r.x.unwrap_or(0.0))).await?;
                }
                self.settle(wait(400)).await;
                let pos = self
                    .value("[Math.round(scrollY), innerHeight, document.documentElement.scrollHeight]")
                    .await
                    .unwrap_or(Value::Null);
                let what = format!(
                    "Scrolled to y={} (viewport {} px, page {} px tall).",
                    pos[0].as_i64().unwrap_or(0),
                    pos[1].as_i64().unwrap_or(0),
                    pos[2].as_i64().unwrap_or(0)
                );
                let shot = self.capture(false, "").await?;
                Ok(self.finish(&what, Some(shot)).await)
            }
            Act::Eval => {
                let compiled = self
                    .call(
                        "Runtime.compileScript",
                        json!({ "expression": r.script, "sourceURL": "", "persistScript": false }),
                    )
                    .await?;
                let bare_return = compiled["exceptionDetails"]["exception"]["description"]
                    .as_str()
                    .or(compiled["exceptionDetails"]["text"].as_str())
                    .is_some_and(|e| e.contains("Illegal return statement"));
                let result = if bare_return {
                    self.evaluate(&format!("(async () => {{\n{}\n}})()", r.script), false).await
                } else {
                    self.evaluate(&r.script, true).await
                }
                .map_err(|e| format!("The script failed: {e}"))?;
                let shown = match result.get("value") {
                    Some(v) => serde_json::to_string_pretty(v).unwrap_or_default(),
                    None if result["type"] == "undefined" => "undefined".to_string(),
                    None => remote_text(&result),
                };
                Ok(self.finish(&format!("Result:\n{}", clip(&shown, MAX_TEXT)), None).await)
            }
            Act::Text => {
                let v = self.value(OUTLINE).await?;
                Ok(self.finish(&format_page_text(&v), None).await)
            }
            Act::Console => {
                let all: Vec<Entry> = {
                    let mut s = self.state.lock().unwrap();
                    s.unseen.clear();
                    s.log.drain(..).collect()
                };
                let body = if all.is_empty() {
                    "No console messages since the last check.".to_string()
                } else {
                    format!("Console since the last check (every level):\n{}", format_entries(&all, BUFFER))
                };
                Ok(self.finish(&body, None).await)
            }
            Act::Network => {
                let all: Vec<NetEntry> = {
                    let mut s = self.state.lock().unwrap();
                    let mut pending: Vec<Req> = s.requests.values().cloned().collect();
                    pending.sort_by(|a, b| a.start.total_cmp(&b.start));
                    let mut all: Vec<NetEntry> = s.net.drain(..).collect();
                    all.extend(pending.into_iter().map(|r| r.done(None, "")));
                    all
                };
                let body = if all.is_empty() {
                    "No requests since the last check.".to_string()
                } else {
                    format!(
                        "Requests since the last check (method, status, type, url, time):\n{}",
                        format_requests(&all)
                    )
                };
                Ok(self.finish(&body, None).await)
            }
            Act::Hover => {
                let (x, y, what) = if r.selector.is_empty() {
                    let (x, y) = (r.x.unwrap_or(0.0), r.y.unwrap_or(0.0));
                    (x, y, format!("Hovered at ({x}, {y})."))
                } else {
                    let t = self.locate(&r.selector).await?;
                    (t.x, t.y, format!("Hovered over {} at ({}, {}).", t.label, t.x.round(), t.y.round()))
                };
                self.call("Input.dispatchMouseEvent", json!({ "type": "mouseMoved", "x": x, "y": y }))
                    .await?;
                self.after(navs, wait(300)).await;
                let shot = self.capture(false, "").await?;
                Ok(self.finish(&what, Some(shot)).await)
            }
            Act::Select => {
                let script = format!("{SELECT}({}, {})", selector_js(&r.selector), selector_js(r.text.trim()));
                let v = self
                    .value(&script)
                    .await
                    .map_err(|e| format!("The selector {} did not work: {e}", r.selector))?;
                if v.is_null() {
                    return Err(format!("No element matches the selector {}.", r.selector));
                }
                if let Some(tag) = v["tag"].as_str() {
                    return Err(format!("{} is a <{tag}> element, not a <select>.", r.selector));
                }
                if let Some(options) = v["options"].as_array() {
                    let names: Vec<&str> = options.iter().filter_map(Value::as_str).collect();
                    return Err(format!(
                        "No option of {} matches \"{}\". The options are: {}.",
                        r.selector,
                        r.text.trim(),
                        names.join("; ")
                    ));
                }
                self.after(navs, wait(600)).await;
                let shot = self.capture(false, "").await?;
                let what = format!("selected {}", v["selected"].as_str().unwrap_or(""));
                Ok(self.finish(&what, Some(shot)).await)
            }
            Act::Wait => {
                let max = Duration::from_millis(r.wait_ms.unwrap_or(10_000));
                let (check, shown) = if r.selector.is_empty() {
                    (
                        format!("!!document.body && document.body.innerText.includes({})", selector_js(&r.text)),
                        format!("\"{}\"", clip(&r.text, 100)),
                    )
                } else {
                    (format!("{VISIBLE}({})", selector_js(&r.selector)), r.selector.clone())
                };
                let started = Instant::now();
                loop {
                    match self.value(&check).await {
                        Ok(Value::Bool(true)) => {
                            let what = format!("Found {shown} after {} ms.", started.elapsed().as_millis());
                            return Ok(self.finish(&what, None).await);
                        }
                        Err(e) if e.contains("is not a valid selector") => {
                            return Err(format!("The selector {} did not work: {e}", r.selector));
                        }
                        _ => {}
                    }
                    if started.elapsed() >= max {
                        break;
                    }
                    tokio::time::sleep(Duration::from_millis(200)).await;
                }
                let shot = self.capture(false, "").await?;
                let what = format!("Still not there after {} ms: {shown}.", started.elapsed().as_millis());
                Ok(self.finish(&what, Some(shot)).await)
            }
            Act::Reload => {
                self.reset_page();
                self.call("Page.reload", json!({})).await?;
                self.wait_load(Duration::from_secs(20)).await;
                self.settle(wait(DEFAULT_WAIT)).await;
                let shot = self.capture(false, "").await?;
                Ok(self.finish("Reloaded.", Some(shot)).await)
            }
            Act::Back => {
                let h = self.call("Page.getNavigationHistory", json!({})).await?;
                let index = h["currentIndex"].as_u64().unwrap_or(0) as usize;
                if index == 0 {
                    return Err("There is no earlier page in this tab's history.".to_string());
                }
                let entry = h["entries"][index - 1]["id"].clone();
                self.call("Page.navigateToHistoryEntry", json!({ "entryId": entry })).await?;
                self.after(navs, wait(DEFAULT_WAIT)).await;
                Ok(self.finish("Went back one page.", None).await)
            }
            Act::Close => Err("close is handled before the browser starts.".to_string()),
        }
    }
}

pub fn format_page_text(v: &Value) -> String {
    let mut outline = Vec::new();
    let list = |k: &str| v[k].as_array().cloned().unwrap_or_default();
    let headings = list("headings");
    if !headings.is_empty() {
        outline.push("Headings:".to_string());
        outline.extend(headings.iter().filter_map(|h| h.as_str()).map(|h| format!("  {h}")));
    }
    let links = list("links");
    if !links.is_empty() {
        outline.push(format!("Links ({}):", links.len()));
        outline.extend(links.iter().map(|l| {
            let label = l[0].as_str().unwrap_or("");
            format!(
                "  {} -> {}",
                if label.is_empty() { "(no text)" } else { label },
                l[1].as_str().unwrap_or("")
            )
        }));
    }
    let buttons = list("buttons");
    if !buttons.is_empty() {
        outline.push("Buttons:".to_string());
        outline.extend(buttons.iter().filter_map(|b| b.as_str()).map(|b| {
            if b.is_empty() {
                "  (no text)".to_string()
            } else {
                format!("  {b}")
            }
        }));
    }
    let inputs = list("inputs");
    if !inputs.is_empty() {
        outline.push("Inputs:".to_string());
        outline.extend(inputs.iter().map(|i| {
            let mut s = format!("  {}", i["tag"].as_str().unwrap_or("input"));
            for k in ["type", "name", "placeholder", "label"] {
                if let Some(val) = i[k].as_str().filter(|x| !x.is_empty()) {
                    s.push_str(&format!(" {k}=\"{val}\""));
                }
            }
            s
        }));
    }
    let images = list("images");
    if !images.is_empty() {
        outline.push("Images with problems:".to_string());
        outline.extend(images.iter().map(|i| {
            let mut why = Vec::new();
            if i["broken"].as_bool() == Some(true) {
                why.push("failed to load");
            }
            if i["noalt"].as_bool() == Some(true) {
                why.push("no alt text");
            }
            format!("  {} ({})", i["src"].as_str().unwrap_or(""), why.join(", "))
        }));
    }
    let outline = clip(&outline.join("\n"), 8000);
    let mut body = String::new();
    let mut blank = 0;
    for line in v["text"].as_str().unwrap_or("").lines() {
        let line = line.trim_end();
        blank = if line.trim().is_empty() { blank + 1 } else { 0 };
        if blank < 2 {
            body.push_str(line);
            body.push('\n');
        }
    }
    let budget = MAX_TEXT.saturating_sub(outline.chars().count()).max(4000);
    let body = clip(body.trim(), budget);
    let mut out = format!("Visible text:\n{}", if body.is_empty() { "(none)" } else { &body });
    if !outline.is_empty() {
        out.push_str("\n\n");
        out.push_str(&outline);
    }
    out
}

async fn close(key: &str) -> String {
    let existing = SLOTS.lock().unwrap().get(key).cloned();
    let Some(slot) = existing else {
        return "No browser was open for this chat.".to_string();
    };
    match slot.session.lock().await.take() {
        Some(_) => "Closed the browser for this chat.".to_string(),
        None => "No browser was open for this chat.".to_string(),
    }
}

pub async fn execute(r: Request, cancel: Arc<Notify>) -> Result<Outcome, String> {
    start_reaper();
    if r.act == Act::Close {
        return Ok(Outcome {
            text: close(&r.key).await,
            image: None,
        });
    }
    let slot = slot(&r.key);
    let budget = r.timeout + Duration::from_millis(r.wait_ms.unwrap_or(0));
    let work = async {
        let mut guard = slot.session.lock().await;
        if guard.as_mut().is_some_and(|s| s.dead()) {
            guard.take();
        }
        if guard.is_none() {
            let started = tokio::time::timeout(Duration::from_secs(45), Session::launch())
                .await
                .map_err(|_| "The browser did not start within 45 seconds.".to_string())??;
            *guard = Some(started);
        }
        let out = tokio::time::timeout(budget, guard.as_mut().unwrap().run(&r)).await;
        *slot.used.lock().unwrap() = Instant::now();
        match out {
            Ok(result) => result,
            Err(_) => {
                guard.take();
                Err(format!(
                    "The browser did not finish within {} seconds, so Salam Agent closed it; the next browser call starts a fresh one.",
                    budget.as_secs()
                ))
            }
        }
    };
    tokio::select! {
        out = work => out,
        _ = cancel.notified() => Err("[stopped by the user]".to_string()),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn ws() -> (tempfile::TempDir, Workspace) {
        let d = tempfile::tempdir().unwrap();
        std::fs::write(d.path().join("index.html"), "<h1>hi</h1>").unwrap();
        let w = Workspace::open(d.path().to_str().unwrap()).unwrap();
        (d, w)
    }

    #[test]
    fn parses_and_validates_arguments() {
        let (_d, w) = ws();
        let p = |v: Value| parse(&v, "chat", &w);
        let r = p(json!({"action": "open", "url": "localhost:5173/app", "wait_ms": 99999, "width": 390, "height": "844"})).unwrap();
        assert_eq!(r.url, "http://localhost:5173/app");
        assert_eq!(r.wait_ms, Some(MAX_WAIT));
        assert_eq!((r.width, r.height), (Some(390), Some(844)));
        assert_eq!(r.key, "chat");
        assert_eq!(r.timeout, Duration::from_secs(30));
        assert_eq!(
            p(json!({"action": "open", "url": "example.com"})).unwrap().url,
            "https://example.com"
        );
        assert_eq!(
            p(json!({"action": "open", "url": "127.0.0.1:8080"})).unwrap().url,
            "http://127.0.0.1:8080"
        );
        assert!(
            p(json!({"action": "open", "url": "index.html"}))
                .unwrap()
                .url
                .starts_with("file://")
        );
        assert!(
            p(json!({"action": "open", "url": "data:text/html,<p>x"}))
                .unwrap()
                .url
                .starts_with("data:")
        );
        assert!(p(json!({"action": "open", "url": "javascript:alert(1)"})).is_err());
        assert!(p(json!({"action": "open", "url": "chrome://settings"})).is_err());
        assert!(p(json!({"action": "open"})).is_err());
        assert!(p(json!({"action": "fly"})).is_err());
        assert!(p(json!({"action": "click"})).is_err());
        assert!(p(json!({"action": "click", "x": 10})).is_err());
        assert!(p(json!({"action": "click", "x": 10, "y": 20.5})).is_ok());
        assert!(p(json!({"action": "click", "selector": "#go"})).is_ok());
        assert!(p(json!({"action": "type", "selector": "input"})).is_err());
        assert!(p(json!({"action": "type", "text": "hi", "key": "Enter"})).is_ok());
        assert!(p(json!({"action": "press"})).is_err());
        assert!(p(json!({"action": "press", "key": "Hyper+Q"})).is_err());
        assert!(p(json!({"action": "eval", "script": " "})).is_err());
        assert!(p(json!({"action": "screenshot", "width": 50})).is_err());
        assert_eq!(p(json!({"action": "text", "session": "admin"})).unwrap().key, "chat#admin");
        assert!(p(json!({"action": "hover"})).is_err());
        assert!(p(json!({"action": "hover", "y": 3})).is_err());
        assert!(p(json!({"action": "hover", "x": 1, "y": 3})).is_ok());
        assert!(p(json!({"action": "hover", "selector": "nav a"})).is_ok());
        assert!(p(json!({"action": "select", "text": "Red"})).is_err());
        assert!(p(json!({"action": "select", "selector": "#color"})).is_err());
        assert!(p(json!({"action": "select", "selector": "#color", "text": "Red"})).is_ok());
        assert!(p(json!({"action": "wait"})).is_err());
        assert!(p(json!({"action": "wait", "text": "  "})).is_err());
        assert!(p(json!({"action": "wait", "selector": ".done"})).is_ok());
        let w8 = p(json!({"action": "wait", "text": "Saved", "wait_ms": 99999})).unwrap();
        assert_eq!(w8.wait_ms, Some(MAX_WAIT_FOR));
        assert_eq!(p(json!({"action": "refresh"})).unwrap().act, Act::Reload);
        assert_eq!(p(json!({"action": "network"})).unwrap().act, Act::Network);
    }

    #[test]
    fn eval_is_exec_and_the_rest_read() {
        let (_d, w) = ws();
        for (args, kind, summary) in [
            (
                json!({"action": "eval", "script": "document.title"}),
                Kind::Exec,
                "browser eval document.title",
            ),
            (
                json!({"action": "open", "url": "https://example.com/"}),
                Kind::Read,
                "browser open https://example.com/",
            ),
            (
                json!({"action": "click", "selector": "#submit"}),
                Kind::Read,
                "browser click #submit",
            ),
            (
                json!({"action": "type", "selector": "input[name=q]", "text": "x"}),
                Kind::Read,
                "browser type into input[name=q]",
            ),
            (json!({"action": "screenshot"}), Kind::Read, "browser screenshot"),
            (json!({"action": "console"}), Kind::Read, "browser console"),
            (json!({"action": "network"}), Kind::Read, "browser network"),
            (json!({"action": "hover", "selector": "#menu"}), Kind::Read, "browser hover #menu"),
            (
                json!({"action": "select", "selector": "#color", "text": "Red"}),
                Kind::Read,
                "browser select Red in #color",
            ),
            (json!({"action": "wait", "selector": ".done"}), Kind::Read, "browser wait for .done"),
            (
                json!({"action": "wait", "text": "Saved"}),
                Kind::Read,
                "browser wait for text \"Saved\"",
            ),
            (json!({"action": "reload"}), Kind::Read, "browser reload"),
            (json!({"action": "close"}), Kind::Read, "browser close"),
        ] {
            let r = parse(&args, "c", &w).unwrap();
            assert_eq!(r.act.kind(), kind);
            let (s, detail, lang) = describe(&r);
            assert_eq!(s, summary);
            if kind == Kind::Exec {
                assert_eq!((detail.as_str(), lang.as_str()), ("document.title", "javascript"));
            }
        }
    }

    #[test]
    fn keys_map_to_key_events() {
        let enter = parse_key("Enter").unwrap();
        assert_eq!((enter.key_code, enter.text.as_str()), (13, "\r"));
        assert_eq!(parse_key("esc").unwrap().key, "Escape");
        assert_eq!(parse_key("down").unwrap().key, "ArrowDown");
        let a = parse_key("Control+a").unwrap();
        assert_eq!((a.modifiers, a.code.as_str(), a.text.as_str()), (2, "KeyA", ""));
        assert_eq!(parse_key("Shift+a").unwrap().text, "A");
        assert_eq!(parse_key("F5").unwrap().key_code, 116);
        assert_eq!(parse_key("Control++").unwrap().key, "+");
        assert!(parse_key("NotAKey").is_err());
    }

    #[test]
    fn finds_the_browser_in_order() {
        let none = |_: &Path| false;
        let fake = |_: &str| None;
        assert!(find_with(&fake, &none).unwrap_err().contains("Google Chrome or Microsoft Edge"));
        let custom = Path::new("/opt/custom/chrome-bin");
        let env = |k: &str| (k == "SALAM_AGENT_BROWSER").then(|| "/opt/custom/chrome-bin".to_string());
        assert_eq!(find_with(&env, &|p: &Path| p == custom).unwrap(), custom);
        assert!(find_with(&env, &none).unwrap_err().contains("SALAM_AGENT_BROWSER"));
    }

    #[cfg(all(unix, not(target_os = "macos")))]
    #[test]
    fn prefers_chrome_over_chromium_on_path() {
        let env = |k: &str| (k == "PATH").then(|| "/a:/b".to_string());
        let found = find_with(&env, &|p: &Path| {
            p == Path::new("/a/chromium") || p == Path::new("/b/google-chrome")
        })
        .unwrap();
        assert_eq!(found, Path::new("/b/google-chrome"));
        let named = |k: &str| match k {
            "PATH" => Some("/a:/b".to_string()),
            "SALAM_AGENT_BROWSER" => Some("brave-browser".to_string()),
            _ => None,
        };
        let found = find_with(&named, &|p: &Path| {
            p == Path::new("/b/brave-browser") || p == Path::new("/a/chromium")
        })
        .unwrap();
        assert_eq!(found, Path::new("/b/brave-browser"));
    }

    #[test]
    fn launch_args_use_a_fresh_profile() {
        let args = launch_args(Path::new("/tmp/p"), false);
        for a in [
            "--headless=new",
            "--remote-debugging-port=0",
            "--no-first-run",
            "--disable-extensions",
        ] {
            assert!(args.iter().any(|x| x == a), "{a}");
        }
        assert!(args.iter().any(|x| x.starts_with("--user-data-dir=") && x.ends_with('p')));
        assert!(args.iter().any(|x| x.starts_with("--crash-dumps-dir=/tmp/p")));
        assert!(!args.iter().any(|x| x == "--no-sandbox"));
        assert!(launch_args(Path::new("/tmp/p"), true).iter().any(|x| x == "--no-sandbox"));
    }

    #[test]
    fn collects_console_and_network_problems() {
        let mut s = PageState {
            page: "p".into(),
            main_frame: "F".into(),
            ..Default::default()
        };
        s.on_event(
            "Runtime.consoleAPICalled",
            &json!({"type": "error", "args": [{"type": "string", "value": "boom"}, {"type": "number", "value": 2}],
                    "stackTrace": {"callFrames": [{"url": "http://x/app.js", "lineNumber": 9}]}}),
        );
        s.on_event(
            "Runtime.consoleAPICalled",
            &json!({"type": "log", "args": [{"type": "string", "value": "hello"}]}),
        );
        s.on_event(
            "Runtime.exceptionThrown",
            &json!({"exceptionDetails": {"text": "Uncaught", "exception": {"description": "TypeError: x is null\n    at go (http://x/app.js:3:5)\n    at main (http://x/app.js:9:1)"}}}),
        );
        s.on_event(
            "Network.requestWillBeSent",
            &json!({"requestId": "D", "loaderId": "D", "frameId": "F", "type": "Document", "timestamp": 100.0, "request": {"method": "GET", "url": "http://x/"}}),
        );
        s.on_event(
            "Log.entryAdded",
            &json!({"entry": {"source": "security", "level": "warning", "text": "Mixed content", "url": "http://x/"}}),
        );
        s.on_event(
            "Runtime.consoleAPICalled",
            &json!({"type": "debug", "args": [{"type": "object", "subtype": "array", "preview": {"properties": [{"name": "0", "value": "1"}]}}]}),
        );
        s.on_event(
            "Runtime.consoleAPICalled",
            &json!({"type": "info", "args": [{"type": "string", "value": "i"}]}),
        );
        s.on_event("Network.responseReceived", &json!({"requestId": "D", "response": {"status": 200}}));
        s.on_event("Network.loadingFinished", &json!({"requestId": "D", "timestamp": 100.25}));
        s.on_event(
            "Network.requestWillBeSent",
            &json!({"requestId": "1", "type": "Script", "request": {"method": "GET", "url": "http://x/missing.js"}}),
        );
        s.on_event("Network.responseReceived", &json!({"requestId": "1", "response": {"status": 404}}));
        s.on_event(
            "Network.requestWillBeSent",
            &json!({"requestId": "2", "type": "Fetch", "timestamp": 101.0, "request": {"method": "POST", "url": "http://api/"}}),
        );
        s.on_event(
            "Network.loadingFailed",
            &json!({"requestId": "2", "timestamp": 101.004, "errorText": "net::ERR_CONNECTION_REFUSED"}),
        );
        s.on_event(
            "Log.entryAdded",
            &json!({"entry": {"source": "network", "level": "error", "text": "dup"}}),
        );
        s.on_event(
            "Runtime.consoleAPICalled",
            &json!({"type": "warning", "args": [{"type": "string", "value": "w"}]}),
        );
        s.on_event(
            "Runtime.consoleAPICalled",
            &json!({"type": "warning", "args": [{"type": "string", "value": "w"}]}),
        );
        assert_eq!(s.status, Some(200));
        let all: Vec<Entry> = s.log.iter().cloned().collect();
        assert_eq!(
            format_entries(&all, BUFFER),
            "error: boom 2 (http://x/app.js:10)\n\
             log: hello\n\
             exception: Uncaught TypeError: x is null at go (http://x/app.js:3:5)\n\
             warn: Mixed content (http://x/)\n\
             debug: [0: 1]\n\
             info: i\n\
             failed: 404 GET http://x/missing.js\n\
             failed: POST http://api/ net::ERR_CONNECTION_REFUSED\n\
             warn: w (x2)"
        );
        let problems: Vec<Entry> = s.unseen.iter().cloned().collect();
        assert_eq!(
            format_problems(&problems, 30),
            "- error: boom 2 (http://x/app.js:10)\n\
             - exception: Uncaught TypeError: x is null at go (http://x/app.js:3:5)\n\
             - warn: Mixed content (http://x/)\n\
             - failed: 404 GET http://x/missing.js\n\
             - failed: POST http://api/ net::ERR_CONNECTION_REFUSED\n\
             - warn: w (x2)"
        );
        assert!(format_problems(&problems, 2).starts_with("- [... 4 earlier entries ...]\n- failed: POST"));
        let net: Vec<NetEntry> = s.net.iter().cloned().collect();
        assert_eq!(
            format_requests(&net),
            "GET 200 document http://x/ 250ms\n\
             POST failed fetch http://api/ 4ms net::ERR_CONNECTION_REFUSED"
        );
        assert_eq!(s.requests.len(), 1);
        for i in 0..250 {
            s.on_event(
                "Runtime.consoleAPICalled",
                &json!({"type": "log", "args": [{"type": "number", "value": i}]}),
            );
        }
        assert_eq!(s.log.len(), BUFFER);
    }

    #[test]
    fn lists_requests_with_redirects_and_long_urls() {
        let mut s = PageState::default();
        let long = format!("http://host/{}", "a".repeat(300));
        s.on_event(
            "Network.requestWillBeSent",
            &json!({"requestId": "1", "type": "Fetch", "timestamp": 5.0, "request": {"method": "GET", "url": "http://host/old"}}),
        );
        s.on_event(
            "Network.requestWillBeSent",
            &json!({"requestId": "1", "type": "Fetch", "timestamp": 5.002, "redirectResponse": {"status": 301},
                    "request": {"method": "GET", "url": "http://host/api"}}),
        );
        s.on_event("Network.responseReceived", &json!({"requestId": "1", "response": {"status": 500}}));
        s.on_event("Network.loadingFinished", &json!({"requestId": "1", "timestamp": 5.012}));
        s.on_event(
            "Network.requestWillBeSent",
            &json!({"requestId": "2", "type": "Image", "timestamp": 6.0, "request": {"method": "GET", "url": long}}),
        );
        let mut all: Vec<NetEntry> = s.net.iter().cloned().collect();
        all.extend(s.requests.values().cloned().map(|r| r.done(None, "")));
        let out = format_requests(&all);
        let lines: Vec<&str> = out.lines().collect();
        assert_eq!(lines[0], "GET 301 fetch http://host/old 2ms");
        assert_eq!(lines[1], "GET 500 fetch http://host/api 10ms");
        assert!(lines[2].starts_with("GET pending image http://host/aaa") && lines[2].ends_with(" …"));
        assert_eq!(lines[2].chars().count(), "GET pending image ".len() + 200 + 2);
        assert!(s.unseen.iter().any(|e| e.level == "failed" && e.text == "500 GET http://host/api"));
    }

    #[test]
    fn state_line_and_outline() {
        assert_eq!(
            state_line("https://example.com/", Some(200), "", "Example Domain", false),
            "https://example.com/ (200) \"Example Domain\""
        );
        assert_eq!(
            state_line("http://localhost:3000/", None, "net::ERR_CONNECTION_REFUSED", "", true),
            "http://localhost:3000/ (net::ERR_CONNECTION_REFUSED) \"\" (still loading)"
        );
        let out = format_page_text(&json!({
            "text": "Hello\n\n\n\nWorld",
            "headings": ["h1 Hello"],
            "links": [["Docs", "https://x/docs"]],
            "buttons": ["Send"],
            "inputs": [{"tag": "input", "type": "email", "name": "mail", "placeholder": "you@x", "label": ""}],
            "images": [{"src": "a.png", "broken": true, "noalt": true}]
        }));
        assert!(out.starts_with("Visible text:\nHello\n\nWorld\n\nHeadings:\n  h1 Hello"));
        assert!(out.contains("  Docs -> https://x/docs"));
        assert!(out.contains("  input type=\"email\" name=\"mail\" placeholder=\"you@x\""));
        assert!(out.contains("  a.png (failed to load, no alt text)"));
    }

    #[tokio::test]
    #[ignore]
    async fn drives_a_real_browser() {
        struct Cleanup;
        impl Drop for Cleanup {
            fn drop(&mut self) {
                close_all();
            }
        }
        let _cleanup = Cleanup;
        let (d, w) = ws();
        std::fs::write(
            d.path().join("page.html"),
            r#"<!doctype html><html><head><title>Test Page</title></head><body>
<h1>Hello</h1><input name="q" placeholder="search">
<button id="b" onclick="document.getElementById('out').textContent = 'clicked ' + (window.n = (window.n || 0) + 1); console.error('boom from click')">Press me</button>
<p id="out">waiting</p><div id="h" onmouseover="this.textContent = 'hovered'">hover me</div>
<select id="s" onchange="window.changed = this.value"><option value="a">Apple</option><option value="b">Banana</option></select>
<img src="missing.png" alt="gone">
<div style="height:3000px"></div><p id="end">bottom</p>
<script>console.error('early failure'); console.warn('careful'); console.log('plain'); console.info('fyi'); console.debug('dbg');
setTimeout(() => { const p = document.createElement('p'); p.id = 'late'; p.textContent = 'arrived late'; document.body.appendChild(p); }, 4000);</script>
</body></html>"#,
        )
        .unwrap();
        let run = |args: Value| {
            let r = parse(&args, "integration", &w).unwrap();
            execute(r, Arc::new(Notify::new()))
        };
        let open = run(json!({"action": "open", "url": "page.html"})).await.unwrap();
        println!("{}", open.text);
        assert!(open.text.lines().next().unwrap().contains("\"Test Page\""));
        assert!(open.text.contains("Problems seen:\n"));
        assert!(open.text.contains("- error: early failure") && open.text.contains("- warn: careful"));
        assert!(open.text.contains("missing.png") && open.text.contains("ERR_FILE_NOT_FOUND"));
        assert!(!open.text.contains("plain") && !open.text.contains("fyi"));
        let image = open.image.unwrap();
        assert_eq!(image.mime, "image/jpeg");
        assert!(image.data.starts_with("/9j/"));
        assert!(open.text.contains("Screenshot attached (1280x800 px, JPEG)."));

        let click = run(json!({"action": "click", "selector": "#b"})).await.unwrap();
        println!("{}", click.text);
        assert!(click.text.contains("boom from click") && click.image.is_some());
        let text = run(json!({"action": "text"})).await.unwrap();
        assert!(text.text.contains("clicked 1") && text.text.contains("Press me"));
        assert!(text.text.contains("placeholder=\"search\""));

        let typed = run(json!({"action": "type", "selector": "input[name=q]", "text": "salam"}))
            .await
            .unwrap();
        assert!(typed.text.contains("its value is now \"salam\""), "{}", typed.text);

        let console = run(json!({"action": "console"})).await.unwrap();
        println!("{}", console.text);
        assert!(console.text.contains("Console since the last check (every level):"));
        for line in ["log: plain", "warn: careful", "info: fyi", "debug: dbg", "error: boom from click"] {
            assert!(console.text.contains(line), "{line}");
        }
        let again = run(json!({"action": "console"})).await.unwrap();
        assert!(again.text.contains("No console messages"));

        let net = run(json!({"action": "network"})).await.unwrap();
        println!("{}", net.text);
        assert!(
            net.text
                .contains("Requests since the last check (method, status, type, url, time):")
        );
        assert!(net.text.contains("GET 200 document file://") || net.text.contains("GET - document file://"));
        assert!(net.text.contains("image") && net.text.contains("net::ERR_FILE_NOT_FOUND"));
        let net = run(json!({"action": "network"})).await.unwrap();
        assert!(net.text.contains("No requests since the last check."), "{}", net.text);

        let hover = run(json!({"action": "hover", "selector": "#h"})).await.unwrap();
        assert!(hover.text.contains("Hovered over div") && hover.image.is_some());
        let hovered = run(json!({"action": "eval", "script": "document.getElementById('h').textContent"}))
            .await
            .unwrap();
        assert!(hovered.text.ends_with("\"hovered\""), "{}", hovered.text);
        let picked = run(json!({"action": "select", "selector": "#s", "text": "Banana"})).await.unwrap();
        assert!(
            picked.text.contains("\nselected Banana") && picked.image.is_some(),
            "{}",
            picked.text
        );
        let changed = run(json!({"action": "eval", "script": "window.changed"})).await.unwrap();
        assert!(changed.text.ends_with("\"b\""), "{}", changed.text);
        let missing = run(json!({"action": "select", "selector": "#s", "text": "Cherry"}))
            .await
            .err()
            .unwrap();
        assert!(missing.contains("Apple (a); Banana (b)"), "{missing}");
        let found = run(json!({"action": "wait", "selector": "#late"})).await.unwrap();
        assert!(found.text.contains("Found #late after") && found.image.is_none(), "{}", found.text);
        let text_found = run(json!({"action": "wait", "text": "arrived late"})).await.unwrap();
        assert!(text_found.text.contains("Found \"arrived late\" after"));
        let never = run(json!({"action": "wait", "text": "never shown", "wait_ms": 500})).await.unwrap();
        assert!(
            never.text.contains("Still not there after") && never.image.is_some(),
            "{}",
            never.text
        );

        let ev = run(json!({"action": "eval", "script": "return document.title + '!'"}))
            .await
            .unwrap();
        assert!(ev.text.contains("\"Test Page!\""), "{}", ev.text);
        let awaited = run(json!({"action": "eval", "script": "const v = await Promise.resolve(41); v + 1"}))
            .await
            .unwrap();
        assert!(awaited.text.ends_with("42"), "{}", awaited.text);
        let again = run(json!({"action": "eval", "script": "let k = 1; k"})).await.unwrap();
        assert!(again.text.ends_with('1'));
        let redeclared = run(json!({"action": "eval", "script": "let k = 2; k"})).await.unwrap();
        assert!(redeclared.text.ends_with('2'), "{}", redeclared.text);
        let bad = run(json!({"action": "eval", "script": "nope()"})).await.err().unwrap();
        assert!(bad.contains("ReferenceError"), "{bad}");

        let scroll = run(json!({"action": "scroll", "selector": "#end"})).await.unwrap();
        assert!(!scroll.text.contains("y=0 "), "{}", scroll.text);
        let full = run(json!({"action": "screenshot", "full_page": true})).await.unwrap();
        assert!(full.text.contains("Screenshot attached (1280x"), "{}", full.text);
        let part = run(json!({"action": "screenshot", "selector": "#b"})).await.unwrap();
        assert!(part.image.is_some());

        let mobile = run(json!({"action": "eval", "script": "[screen.width, navigator.userAgent]", "width": 390, "height": 844}))
            .await
            .unwrap();
        assert!(mobile.text.contains("390") && mobile.text.contains("Mobile"), "{}", mobile.text);

        let reload = run(json!({"action": "reload", "width": 1280, "height": 800})).await.unwrap();
        assert!(reload.text.contains("Reloaded.") && reload.image.is_some(), "{}", reload.text);
        assert!(reload.text.contains("- error: early failure"), "{}", reload.text);

        let data = run(json!({"action": "open", "url": "data:text/html,<title>D</title><p>data page</p>"}))
            .await
            .unwrap();
        assert!(data.text.contains("\"D\""));
        let back = run(json!({"action": "back"})).await.unwrap();
        assert!(back.text.contains("Test Page"), "{}", back.text);

        assert_eq!(PROCS.lock().unwrap().len(), 1);
        let closed = run(json!({"action": "close"})).await.unwrap();
        assert_eq!(closed.text, "Closed the browser for this chat.");
        assert!(PROCS.lock().unwrap().is_empty());
        #[cfg(target_os = "linux")]
        {
            std::thread::sleep(Duration::from_secs(2));
            let ours = format!("salam-agent-browser-{}-", std::process::id());
            let leftover = std::fs::read_dir("/proc")
                .unwrap()
                .flatten()
                .filter_map(|e| std::fs::read(e.path().join("cmdline")).ok())
                .any(|c| String::from_utf8_lossy(&c).contains(&ours));
            assert!(!leftover, "a browser process outlived close");
        }
    }
}
