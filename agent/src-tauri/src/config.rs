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
    fn origins_compare_scheme_host_and_port() {
        let a = Url::parse("https://ai.salamlang.ir/chats/1").unwrap();
        assert!(same_origin(&a, &Url::parse("https://ai.salamlang.ir/").unwrap()));
        assert!(!same_origin(&a, &Url::parse("https://ai.salamlang.ir.evil.com/").unwrap()));
        assert!(!same_origin(&a, &Url::parse("http://ai.salamlang.ir/").unwrap()));
        assert!(!same_origin(&a, &Url::parse("https://ai.salamlang.ir:8443/").unwrap()));
    }
}
