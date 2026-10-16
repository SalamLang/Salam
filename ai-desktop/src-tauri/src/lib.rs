mod approve;
mod bridge;
mod commands;
mod config;
mod device;
mod tools;
mod workspace;

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
        .title("دستیار سلام")
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
            commands::agent_retry,
        ])
        .setup(|app| {
            #[cfg(unix)]
            config::import_login_path();
            let handle = app.handle().clone();
            app.manage(device::DeviceStore::load(&handle));
            allow_dev_origin(app)?;
            open_main(&handle)?;
            tauri::async_runtime::spawn(bridge::run(handle));
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running the Salam desktop app");
}
