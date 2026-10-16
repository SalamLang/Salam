mod approve;
mod bridge;
mod commands;
mod config;
mod device;
mod tools;
mod workspace;

use std::time::Duration;
use tauri::{AppHandle, Manager, WebviewUrl, WebviewWindowBuilder};
use tauri_plugin_opener::OpenerExt;

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
];

fn open_main(app: &AppHandle) -> tauri::Result<()> {
    let target = config::app_url();
    let handle = app.clone();
    WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()))
        .title("ایجنت سلام")
        .inner_size(1180.0, 800.0)
        .min_inner_size(720.0, 520.0)
        .on_navigation(move |url| {
            if config::is_local_asset(url) || config::same_origin(url, &target) || url.scheme() == "about" {
                return true;
            }
            if matches!(url.scheme(), "http" | "https" | "mailto") {
                let _ = handle.opener().open_url(url.as_str(), None::<&str>);
            }
            false
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
    let mut offline_shown = false;
    loop {
        let reachable = matches!(http.get(url.clone()).send().await, Ok(r) if r.status().as_u16() < 500);
        if let Some(w) = app.get_webview_window("main") {
            if reachable {
                let _ = w.navigate(url);
                return;
            }
            if !offline_shown {
                if let Ok(mut here) = w.url() {
                    here.set_fragment(Some("offline"));
                    let _ = w.navigate(here);
                }
                offline_shown = true;
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
    tauri::Builder::default()
        .plugin(tauri_plugin_single_instance::init(|app, _, _| {
            if let Some(w) = app.get_webview_window("main") {
                let _ = w.unminimize();
                let _ = w.set_focus();
            }
        }))
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_opener::init())
        .manage(approve::Approvals::default())
        .manage(bridge::Status::default())
        .invoke_handler(tauri::generate_handler![
            commands::agent_info,
            commands::agent_link,
            commands::agent_unlink,
            commands::agent_pick_folder,
            commands::agent_set_mode,
            commands::agent_decide,
            commands::agent_stop,
            commands::agent_pending,
        ])
        .setup(|app| {
            #[cfg(unix)]
            config::import_login_path();
            let handle = app.handle().clone();
            app.manage(device::DeviceStore::load(&handle));
            allow_dev_origin(app)?;
            open_main(&handle)?;
            tauri::async_runtime::spawn(connect_main(handle.clone()));
            tauri::async_runtime::spawn(bridge::run(handle));
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running Salam Agent");
}
