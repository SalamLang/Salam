const COMMANDS: &[&str] = &[
    "agent_info",
    "agent_link",
    "agent_unlink",
    "agent_pick_folder",
    "agent_set_mode",
    "agent_decide",
    "agent_stop",
    "agent_pending",
    "agent_project_context",
    "agent_alive",
    "agent_notify",
    "agent_notify_prefs",
    "agent_set_notify_prefs",
    "agent_watch_reply",
    "agent_unwatch_reply",
];

fn main() {
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(tauri_build::AppManifest::new().commands(COMMANDS)))
        .expect("failed to run tauri-build");
}
