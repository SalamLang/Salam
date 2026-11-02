use tauri::Url;

const APP_URL: &str = "https://ai.salamlang.ir/";
const API_URL: &str = "https://ai-api.salamlang.ir";

fn debug_override(name: &str) -> Option<String> {
    if cfg!(debug_assertions) {
        std::env::var(name).ok().filter(|v| !v.is_empty())
    } else {
        None
    }
}

pub fn app_url() -> Url {
    let raw = debug_override("SALAM_APP_URL").unwrap_or_else(|| APP_URL.to_string());
    Url::parse(&raw).unwrap_or_else(|_| Url::parse(APP_URL).expect("valid app url"))
}

pub fn api_url() -> String {
    debug_override("SALAM_API_URL")
        .unwrap_or_else(|| API_URL.to_string())
        .trim_end_matches('/')
        .to_string()
}

pub fn bridge_url() -> String {
    debug_override("SALAM_BRIDGE_URL")
        .map(|v| v.trim_end_matches('/').to_string())
        .unwrap_or_else(api_url)
}

pub fn same_origin(a: &Url, b: &Url) -> bool {
    a.scheme() == b.scheme() && a.host_str() == b.host_str() && a.port_or_known_default() == b.port_or_known_default()
}

pub fn is_local_asset(url: &Url) -> bool {
    url.scheme() == "tauri" || url.host_str() == Some("tauri.localhost")
}

pub fn local_url(path: &str) -> Url {
    let base = if cfg!(windows) || cfg!(target_os = "android") {
        "http://tauri.localhost/"
    } else {
        "tauri://localhost/"
    };
    Url::parse(base)
        .and_then(|url| url.join(path))
        .unwrap_or_else(|_| Url::parse(base).expect("valid asset base url"))
}

#[cfg(target_os = "macos")]
pub fn bundled() -> bool {
    std::env::current_exe().is_ok_and(|p| p.to_string_lossy().contains(".app/Contents/MacOS/"))
}

pub fn os_name() -> &'static str {
    if cfg!(target_os = "windows") {
        "windows"
    } else if cfg!(target_os = "macos") {
        "macos"
    } else if cfg!(target_os = "linux") {
        "linux"
    } else {
        "other"
    }
}

pub fn arch() -> &'static str {
    std::env::consts::ARCH
}

pub fn os_version() -> &'static str {
    static VERSION: std::sync::OnceLock<String> = std::sync::OnceLock::new();
    VERSION.get_or_init(|| header_safe(&detect_os_version()))
}

fn header_safe(text: &str) -> String {
    text.chars()
        .filter(|c| c.is_ascii_alphanumeric() || " .-+_()".contains(*c))
        .take(64)
        .collect::<String>()
        .trim()
        .to_string()
}

#[cfg(target_os = "linux")]
fn detect_os_version() -> String {
    std::fs::read_to_string("/etc/os-release")
        .or_else(|_| std::fs::read_to_string("/usr/lib/os-release"))
        .map(|t| parse_os_release(&t))
        .unwrap_or_default()
}

#[cfg(target_os = "macos")]
fn detect_os_version() -> String {
    match std::process::Command::new("sw_vers").arg("-productVersion").output() {
        Ok(out) if out.status.success() => format!("macOS {}", String::from_utf8_lossy(&out.stdout).trim()),
        _ => String::new(),
    }
}

#[cfg(target_os = "windows")]
fn detect_os_version() -> String {
    use std::os::windows::process::CommandExt;
    const CREATE_NO_WINDOW: u32 = 0x0800_0000;
    match std::process::Command::new("cmd")
        .args(["/C", "ver"])
        .creation_flags(CREATE_NO_WINDOW)
        .output()
    {
        Ok(out) if out.status.success() => parse_windows_ver(&String::from_utf8_lossy(&out.stdout)),
        _ => String::new(),
    }
}

#[cfg(not(any(target_os = "linux", target_os = "macos", target_os = "windows")))]
fn detect_os_version() -> String {
    String::new()
}

#[cfg_attr(not(target_os = "linux"), allow(dead_code))]
fn parse_os_release(text: &str) -> String {
    let field = |key: &str| {
        text.lines()
            .find_map(|l| l.strip_prefix(key).and_then(|r| r.strip_prefix('=')))
            .map(|v| v.trim().trim_matches('"').to_string())
            .filter(|v| !v.is_empty())
    };
    match (field("NAME"), field("VERSION_ID")) {
        (Some(n), Some(v)) => format!("{n} {v}"),
        (Some(n), None) => n,
        _ => String::new(),
    }
}

#[cfg_attr(not(target_os = "windows"), allow(dead_code))]
fn parse_windows_ver(text: &str) -> String {
    let Some(start) = text.find("Version ") else {
        return String::new();
    };
    let ver: String = text[start + 8..].chars().take_while(|c| c.is_ascii_digit() || *c == '.').collect();
    let mut parts = ver.split('.');
    let (major, _minor, build) = (parts.next(), parts.next(), parts.next());
    match (major, build.and_then(|b| b.parse::<u32>().ok())) {
        (Some("10"), Some(b)) if b >= 22000 => format!("Windows 11 (build {b})"),
        (Some("10"), Some(b)) => format!("Windows 10 (build {b})"),
        _ => format!("Windows {ver}"),
    }
}

#[cfg(unix)]
pub fn import_login_path() {
    use std::process::{Command, Stdio};
    use std::time::Duration;
    let shell = std::env::var("SHELL").unwrap_or_else(|_| "/bin/sh".to_string());
    let (tx, rx) = std::sync::mpsc::channel();
    std::thread::spawn(move || {
        let out = Command::new(shell)
            .args(["-lc", "printf %s \"$PATH\""])
            .stdin(Stdio::null())
            .stderr(Stdio::null())
            .output();
        let _ = tx.send(out);
    });
    if let Ok(Ok(out)) = rx.recv_timeout(Duration::from_secs(4)) {
        let path = String::from_utf8_lossy(&out.stdout).trim().to_string();
        if out.status.success() && path.contains('/') {
            // SAFETY: called first thing in run(), before Tauri, GTK or the async runtime start any thread that reads the environment.
            unsafe { std::env::set_var("PATH", path) };
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn os_release_gives_name_and_version() {
        assert_eq!(
            parse_os_release("NAME=\"Ubuntu\"\nVERSION_ID=\"24.04\"\nID=ubuntu\n"),
            "Ubuntu 24.04"
        );
        assert_eq!(parse_os_release("PRETTY_NAME=x\nNAME=Arch Linux\n"), "Arch Linux");
        assert_eq!(parse_os_release(""), "");
    }

    #[test]
    fn windows_ver_names_the_release() {
        assert_eq!(
            parse_windows_ver("\r\nMicrosoft Windows [Version 10.0.22631.4602]\r\n"),
            "Windows 11 (build 22631)"
        );
        assert_eq!(
            parse_windows_ver("Microsoft Windows [Version 10.0.19045.5011]"),
            "Windows 10 (build 19045)"
        );
        assert_eq!(parse_windows_ver("nothing here"), "");
    }

    #[test]
    fn header_text_stays_plain_ascii() {
        assert_eq!(header_safe("  Fedora Linux 40 (Workstation)\n"), "Fedora Linux 40 (Workstation)");
        assert_eq!(header_safe("ویندوز 11"), "11");
    }

    #[test]
    fn origins_compare_scheme_host_and_port() {
        let a = Url::parse("https://ai.salamlang.ir/chats/1").unwrap();
        assert!(same_origin(&a, &Url::parse("https://ai.salamlang.ir/").unwrap()));
        assert!(!same_origin(&a, &Url::parse("https://ai.salamlang.ir.evil.com/").unwrap()));
        assert!(!same_origin(&a, &Url::parse("http://ai.salamlang.ir/").unwrap()));
        assert!(!same_origin(&a, &Url::parse("https://ai.salamlang.ir:8443/").unwrap()));
    }
}
