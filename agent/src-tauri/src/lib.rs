mod approve;
mod bridge;
mod commands;
mod config;
mod device;
mod memory;
mod notify;
mod tools;
mod watch;
mod watchdog;
mod workspace;

use std::time::Duration;
use tauri::webview::{NewWindowResponse, PageLoadEvent};
use tauri::{AppHandle, Manager, WebviewUrl, WebviewWindowBuilder};
use tauri_plugin_opener::OpenerExt;

const LOCK_SCRIPT: &str = include_str!("lock.js");

const REMOTE_COMMANDS: &[&str] = &[
    "core:event:allow-listen",
    "core:event:allow-unlisten",
    "allow-agent-info",
    "allow-agent-link",
    "allow-agent-unlink",
    "allow-agent-pick-folder",
    "allow-agent-set-mode",
    "allow-agent-decide",
    "allow-agent-stop",
    "allow-agent-pending",
    "allow-agent-project-context",
    "allow-agent-alive",
    "allow-agent-notify",
    "allow-agent-notify-prefs",
    "allow-agent-set-notify-prefs",
    "allow-agent-watch-reply",
    "allow-agent-unwatch-reply",
];

#[cfg(target_os = "macos")]
fn set_dock_icon() {
    use objc2::{AllocAnyThread, MainThreadMarker};
    use objc2_app_kit::{NSApplication, NSImage};
    use objc2_foundation::NSData;
    if crate::config::bundled() {
        return;
    }
    let Some(mtm) = MainThreadMarker::new() else { return };
    let data = NSData::with_bytes(include_bytes!("../icons/icon-macos.png"));
    if let Some(image) = NSImage::initWithData(NSImage::alloc(), &data) {
        unsafe { NSApplication::sharedApplication(mtm).setApplicationIconImage(Some(&image)) };
    }
}

fn is_external(url: &tauri::Url) -> bool {
    matches!(url.scheme(), "http" | "https" | "mailto")
}

fn open_external(app: &AppHandle, url: &tauri::Url) -> bool {
    if !is_external(url) {
        return false;
    }
    app.opener().open_url(url.as_str(), None::<&str>).is_ok()
}

fn locked() -> bool {
    !(cfg!(debug_assertions) && std::env::var_os("SALAM_DEVTOOLS").is_some())
}

fn open_main(app: &AppHandle) -> tauri::Result<()> {
    let target = config::app_url();
    let loaded = target.clone();
    let navigation = app.clone();
    let popup = app.clone();
    let mut builder = WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()));
    if let Some(icon) = app.default_window_icon() {
        builder = builder.icon(icon.clone())?;
    }
    if locked() {
        builder = builder.initialization_script_for_all_frames(LOCK_SCRIPT);
    }
    builder
        .title("Salam Agent")
        .inner_size(1180.0, 800.0)
        .min_inner_size(720.0, 520.0)
        .disable_drag_drop_handler()
        .devtools(!locked())
        .on_page_load(move |window, payload| {
            let app = window.app_handle();
            match payload.event() {
                PageLoadEvent::Started => watchdog::disarm(app),
                PageLoadEvent::Finished if config::same_origin(payload.url(), &loaded) => watchdog::arm(app),
                PageLoadEvent::Finished => {}
            }
        })
        .on_navigation(move |url| {
            if config::is_local_asset(url) || url.scheme() == "about" || config::same_origin(url, &target) {
                return true;
            }
            open_external(&navigation, url);
            false
        })
        .on_new_window(move |url, _features| {
            open_external(&popup, &url);
            NewWindowResponse::Deny
        })
        .build()?;
    Ok(())
}

async fn connect_main(app: AppHandle) {
    let url = config::app_url();
    let http = reqwest::Client::builder()
        .timeout(Duration::from_secs(8))
        .build()
        .unwrap_or_else(|_| bridge::client());
    let mut attempts = 0u32;
    let mut shown = "";
    loop {
        let state = match http.get(url.clone()).send().await {
            Ok(response) if response.status().as_u16() < 500 => "ok",
            Ok(_) => "server",
            Err(_) => "offline",
        };
        if let Some(window) = app.get_webview_window("main") {
            if state == "ok" {
                let _ = window.navigate(url.clone());
                return;
            }
            attempts += 1;
            if attempts >= 2 && shown != state {
                let _ = window.navigate(config::local_url(&format!("error.html#{state}")));
                shown = state;
            } else if shown.is_empty() {
                if let Ok(mut here) = window.url() {
                    here.set_fragment(Some(state));
                    let _ = window.navigate(here);
                }
                shown = "loader";
            }
        }
        tokio::time::sleep(Duration::from_secs(5)).await;
    }
}

fn allow_dev_origin(app: &tauri::App) -> tauri::Result<()> {
    if !cfg!(debug_assertions) || std::env::var("SALAM_APP_URL").is_err() {
        return Ok(());
    }
    let pattern = format!("{}/*", config::app_url().origin().ascii_serialization());
    let mut cap = tauri::ipc::CapabilityBuilder::new("dev-remote").remote(pattern).window("main");
    for p in REMOTE_COMMANDS {
        cap = cap.permission(*p);
    }
    app.add_capability(cap)
}

pub fn run() {
    #[cfg(unix)]
    config::import_login_path();
    let builder = tauri::Builder::default()
        .plugin(tauri_plugin_single_instance::init(|app, _, _| {
            notify::raise(app);
        }))
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_opener::init())
        .manage(approve::Approvals::default())
        .manage(bridge::Status::default())
        .manage(watch::Watchers::default())
        .manage(watchdog::Watchdog::default())
        .invoke_handler(tauri::generate_handler![
            commands::agent_info,
            commands::agent_link,
            commands::agent_unlink,
            commands::agent_pick_folder,
            commands::agent_set_mode,
            commands::agent_decide,
            commands::agent_stop,
            commands::agent_pending,
            commands::agent_project_context,
            commands::agent_alive,
            commands::agent_notify,
            commands::agent_notify_prefs,
            commands::agent_set_notify_prefs,
            commands::agent_watch_reply,
            commands::agent_unwatch_reply,
        ])
        .setup(|app| {
            #[cfg(target_os = "macos")]
            set_dock_icon();
            let handle = app.handle().clone();
            app.manage(device::DeviceStore::load(&handle));
            app.manage(notify::Notifier::load(&handle));
            allow_dev_origin(app)?;
            open_main(&handle)?;
            tauri::async_runtime::spawn(connect_main(handle.clone()));
            tauri::async_runtime::spawn(watchdog::run(handle.clone()));
            tauri::async_runtime::spawn(bridge::run(handle));
            Ok(())
        });
    #[cfg(any(target_os = "macos", target_os = "ios"))]
    let builder = builder.on_web_content_process_terminate(|webview| {
        let app = webview.app_handle().clone();
        watchdog::disarm(&app);
        let url = config::local_url("error.html#memory");
        tauri::async_runtime::spawn(async move {
            tokio::time::sleep(Duration::from_millis(150)).await;
            if let Some(window) = app.get_webview_window("main") {
                let _ = window.navigate(url);
            }
        });
    });
    builder.run(tauri::generate_context!()).expect("error while running Salam Agent");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn only_web_and_mail_links_leave_the_app() {
        for link in [
            "https://www.salamlang.ir/learn/",
            "http://localhost:3000/chat",
            "mailto:hello@salamlang.ir",
        ] {
            assert!(is_external(&tauri::Url::parse(link).unwrap()), "{link} should open externally");
        }
        for link in ["tauri://localhost/index.html", "about:blank", "file:///etc/passwd"] {
            assert!(!is_external(&tauri::Url::parse(link).unwrap()), "{link} should stay in the app");
        }
    }
}
