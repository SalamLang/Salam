use crate::approve::{Answer, Approvals, Mode};
use crate::bridge::{self, Status};
use crate::config;
use crate::device::{Device, DeviceStore};
use crate::memory;
use crate::notify::{self, Delivery, Kind, Notice, Notifier, Prefs};
use crate::tools::Preview;
use crate::workspace::Workspace;
use serde::Serialize;
use serde_json::json;
use tauri::{AppHandle, Manager};
use tauri_plugin_dialog::DialogExt;
use tauri_plugin_opener::OpenerExt;

#[derive(Serialize)]
pub struct Info {
    version: &'static str,
    os: &'static str,
    os_version: &'static str,
    arch: &'static str,
    device: Option<Device>,
    connected: bool,
}

fn info(app: &AppHandle) -> Info {
    Info {
        version: env!("CARGO_PKG_VERSION"),
        os: config::os_name(),
        os_version: config::os_version(),
        arch: config::arch(),
        device: app.state::<DeviceStore>().device(),
        connected: app.state::<Status>().connected(),
    }
}

fn device_name() -> String {
    let host = gethostname::gethostname().to_string_lossy().trim().to_string();
    let name = if host.is_empty() { config::os_name().to_string() } else { host };
    name.chars().take(80).collect()
}

#[tauri::command]
pub fn agent_info(app: AppHandle) -> Info {
    info(&app)
}

#[tauri::command]
pub async fn agent_link(app: AppHandle, token: String) -> Result<Info, String> {
    let name = device_name();
    let res = bridge::client()
        .post(format!("{}/bridge/devices", config::api_url()))
        .bearer_auth(token.trim())
        .json(&json!({ "name": name, "os": config::os_name() }))
        .send()
        .await
        .map_err(|e| format!("network: {e}"))?;
    let status = res.status();
    let body: serde_json::Value = res.json().await.unwrap_or_default();
    if !status.is_success() {
        return Err(body["message"].as_str().unwrap_or("could not register this computer").to_string());
    }
    let id = match &body["device"]["id"] {
        serde_json::Value::Number(n) => n.to_string(),
        serde_json::Value::String(s) => s.clone(),
        _ => return Err("the server answered without a device id".to_string()),
    };
    let secret = body["secret"].as_str().ok_or("the server answered without a secret")?.to_string();
    app.state::<DeviceStore>().save(Device { id, name }, secret)?;
    Ok(info(&app))
}

#[tauri::command]
pub async fn agent_unlink(app: AppHandle, token: Option<String>) -> Result<Info, String> {
    let store = app.state::<DeviceStore>();
    if let (Some(token), Some(device)) = (token, store.device()) {
        let _ = bridge::client()
            .delete(format!("{}/bridge/devices/{}", config::api_url(), device.id))
            .bearer_auth(token.trim())
            .send()
            .await;
    }
    store.clear();
    app.state::<crate::watch::Watchers>().stop_all();
    app.state::<Notifier>().forget_seen();
    Ok(info(&app))
}

#[tauri::command]
pub async fn agent_pick_folder(app: AppHandle) -> Option<String> {
    let picked =
        tauri::async_runtime::spawn_blocking(move || app.dialog().file().set_title("پوشه‌ی کاری را انتخاب کنید").blocking_pick_folder())
            .await
            .ok()
            .flatten()?;
    picked.into_path().ok().map(|p| p.to_string_lossy().to_string())
}

#[tauri::command]
pub fn agent_set_mode(app: AppHandle, chat: String, mode: Mode) {
    app.state::<Approvals>().set_mode(chat, mode);
}

#[tauri::command]
pub fn agent_decide(app: AppHandle, call_id: String, answer: Answer) -> bool {
    app.state::<Approvals>().answer(&call_id, answer)
}

#[tauri::command]
pub fn agent_stop(app: AppHandle, call_id: String) -> bool {
    app.state::<Approvals>().stop(&call_id)
}

#[tauri::command]
pub fn agent_pending(app: AppHandle) -> Vec<Preview> {
    app.state::<Approvals>().pending()
}

#[tauri::command]
pub fn agent_alive(app: AppHandle) {
    crate::watchdog::beat(&app);
}

#[tauri::command]
pub async fn agent_project_context(workspace: String) -> Result<memory::ProjectContext, String> {
    if workspace.trim().is_empty() {
        return Err("No workspace folder is selected.".to_string());
    }
    tauri::async_runtime::spawn_blocking(move || {
        let ws = Workspace::open(workspace.trim())?;
        Ok(memory::project_context(ws.root()))
    })
    .await
    .map_err(|e| format!("Reading the project context failed: {e}"))?
}

#[tauri::command]
pub async fn agent_notify(
    app: AppHandle,
    kind: Kind,
    chat: Option<String>,
    text: Option<String>,
    mid: Option<i64>,
) -> Result<Delivery, String> {
    let notice = Notice {
        kind,
        chat: chat.unwrap_or_default(),
        text: text.unwrap_or_default(),
        mid,
    };
    notify::deliver(&app, notice).await
}

fn chat_id(chat: &str) -> bool {
    !chat.is_empty() && chat.len() <= 64 && chat.chars().all(|c| c.is_ascii_alphanumeric() || c == '-' || c == '_')
}

#[tauri::command]
pub fn agent_watch_reply(app: AppHandle, chat: String, mid: i64, token: String, title: Option<String>) -> Result<(), String> {
    if !chat_id(&chat) {
        return Err("invalid chat id".to_string());
    }
    if mid <= 0 {
        return Err("invalid message id".to_string());
    }
    let token = token.trim();
    if token.is_empty() || token.len() > 4096 {
        return Err("invalid token".to_string());
    }
    crate::watch::watch(&app, chat, mid, token.to_string(), title.unwrap_or_default());
    Ok(())
}

#[tauri::command]
pub fn agent_unwatch_reply(app: AppHandle, chat: String) {
    app.state::<crate::watch::Watchers>().stop(&chat);
}

#[tauri::command]
pub fn agent_notify_prefs(app: AppHandle) -> Prefs {
    app.state::<Notifier>().prefs()
}

#[tauri::command]
pub fn agent_set_notify_prefs(app: AppHandle, prefs: Prefs) -> Prefs {
    app.state::<Notifier>().set_prefs(prefs)
}

/// The only URLs a click inside the app may hand to the system browser.
fn external_link(url: &str) -> Option<String> {
    let parsed = tauri::Url::parse(url).ok()?;
    let ok = matches!(parsed.scheme(), "http" | "https" | "mailto");
    if ok { Some(parsed.to_string()) } else { None }
}

/// Opens a link from a message in the user's own browser.
///
/// The site does this itself, because `target="_blank"` in a WKWebView on
/// macOS opens nothing at all (the webview never creates the new window).
#[tauri::command]
pub fn agent_open_url(app: AppHandle, url: String) -> Result<(), String> {
    let link = external_link(&url).ok_or_else(|| "only http, https and mailto links open outside the app".to_string())?;
    app.opener()
        .open_url(link, None::<&str>)
        .map_err(|e| format!("could not open the link: {e}"))
}

#[cfg(test)]
mod tests {
    #[test]
    fn only_web_and_mail_links_leave_the_app() {
        for link in [
            "https://github.com/SalamLang/Salam/pull/2785",
            "http://localhost:3000/chat",
            "mailto:hello@salamlang.ir",
        ] {
            assert!(super::external_link(link).is_some(), "{link} should open externally");
        }
        for link in [
            "tauri://localhost/index.html",
            "file:///etc/passwd",
            "javascript:alert(1)",
            "not a url",
        ] {
            assert!(super::external_link(link).is_none(), "{link} must stay blocked");
        }
    }
}
