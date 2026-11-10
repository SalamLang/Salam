use crate::approve::{Approvals, Decision, Policy};
use crate::config;
use crate::device::DeviceStore;
use crate::notify::{self, Kind, Notice};
use crate::tools::{self, Call, Image, Preview};
use crate::workspace::Workspace;
use serde::Serialize;
use serde_json::json;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::time::Duration;
use tauri::{AppHandle, Emitter, Manager};

#[derive(Default)]
pub struct Status {
    connected: AtomicBool,
    told_offline: AtomicBool,
    flips: AtomicU64,
}

const OFFLINE_GRACE: Duration = Duration::from_secs(20);

impl Status {
    pub fn connected(&self) -> bool {
        self.connected.load(Ordering::Relaxed)
    }
}

#[derive(Clone, Serialize)]
struct CallEvent<'a> {
    #[serde(flatten)]
    preview: &'a Preview,
    state: &'a str,
    output: &'a str,
    image: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    image_data: Option<&'a str>,
    #[serde(skip_serializing_if = "Option::is_none")]
    image_mime: Option<&'a str>,
}

#[derive(Clone, Serialize)]
struct OutputEvent {
    call_id: String,
    chunk: String,
}

fn set_connected(app: &AppHandle, value: bool) {
    let status = app.state::<Status>();
    if status.connected.swap(value, Ordering::Relaxed) != value {
        let _ = app.emit("agent://connection", value);
        let flip = status.flips.fetch_add(1, Ordering::Relaxed) + 1;
        if value {
            if status.told_offline.swap(false, Ordering::Relaxed) {
                notify::post(app, Notice::new(Kind::Online));
            }
            return;
        }
        let app = app.clone();
        tauri::async_runtime::spawn(async move {
            tokio::time::sleep(OFFLINE_GRACE).await;
            let status = app.state::<Status>();
            let still_down = !status.connected() && status.flips.load(Ordering::Relaxed) == flip;
            if still_down && app.state::<DeviceStore>().secret().is_some() {
                status.told_offline.store(true, Ordering::Relaxed);
                notify::post(&app, Notice::new(Kind::Offline));
            }
        });
    }
}

pub fn client() -> reqwest::Client {
    reqwest::Client::builder()
        .timeout(Duration::from_secs(40))
        .user_agent(concat!("salam-agent/", env!("CARGO_PKG_VERSION")))
        .build()
        .expect("http client")
}

pub async fn run(app: AppHandle) {
    let http = client();
    let mut backoff = 1u64;
    loop {
        let Some(secret) = app.state::<DeviceStore>().secret() else {
            set_connected(&app, false);
            tokio::time::sleep(Duration::from_secs(2)).await;
            continue;
        };
        let poll = http
            .get(format!("{}/bridge/poll", config::bridge_url()))
            .bearer_auth(&secret)
            .send();
        tokio::pin!(poll);
        let res = tokio::select! {
            r = &mut poll => r,
            _ = tokio::time::sleep(Duration::from_millis(1500)) => {
                set_connected(&app, true);
                poll.await
            }
        };
        match res {
            Ok(r) if r.status().as_u16() == 200 => {
                set_connected(&app, true);
                backoff = 1;
                if let Ok(v) = r.json::<serde_json::Value>().await {
                    if let Ok(call) = serde_json::from_value::<Call>(v["call"].clone()) {
                        let app = app.clone();
                        let http = http.clone();
                        tauri::async_runtime::spawn(async move { handle(app, http, secret, call).await });
                    }
                }
            }
            Ok(r) if r.status().as_u16() == 204 => {
                set_connected(&app, true);
                backoff = 1;
            }
            Ok(r) if r.status().as_u16() == 401 => {
                app.state::<DeviceStore>().clear();
                set_connected(&app, false);
                notify::post(&app, Notice::new(Kind::Unlinked));
                let _ = app.emit("agent://device", ());
            }
            _ => {
                set_connected(&app, false);
                tokio::time::sleep(Duration::from_secs(backoff)).await;
                backoff = (backoff * 2).min(30);
            }
        }
    }
}

fn emit_call(app: &AppHandle, preview: &Preview, state: &str, output: &str, image: Option<&Image>) {
    let _ = app.emit(
        "agent://call",
        CallEvent {
            preview,
            state,
            output,
            image: image.is_some(),
            image_data: image.map(|i| i.data.as_str()),
            image_mime: image.map(|i| i.mime),
        },
    );
}

async fn report(
    http: &reqwest::Client,
    secret: &str,
    call_id: &str,
    status: &str,
    output: &str,
    preview: Option<&Preview>,
    image: Option<&Image>,
) {
    let mut body = json!({ "call_id": call_id, "status": status, "output": output });
    if let Some(i) = image {
        body["image"] = json!(i.data);
        body["image_mime"] = json!(i.mime);
    }
    if let Some(p) = preview {
        body["tool"] = json!(p.tool);
        body["kind"] = json!(p.kind);
        body["summary"] = json!(p.summary);
        body["detail"] = json!(p.detail);
        body["lang"] = json!(p.lang);
        body["outside"] = json!(p.outside);
        body["cwd"] = json!(p.cwd);
    }
    let mut wait = 1u64;
    let mut waited = 0u64;
    loop {
        let sent = http
            .post(format!("{}/bridge/result", config::bridge_url()))
            .bearer_auth(secret)
            .json(&body)
            .send()
            .await;
        if matches!(&sent, Ok(r) if r.status().is_success() || r.status().as_u16() == 404) {
            return;
        }
        if waited >= 120 {
            return;
        }
        tokio::time::sleep(Duration::from_secs(wait)).await;
        waited += wait;
        wait = (wait * 2).min(15);
    }
}

static PINS: std::sync::Mutex<Vec<(String, std::path::PathBuf, usize)>> = std::sync::Mutex::new(Vec::new());

struct Pin(String);

impl Drop for Pin {
    fn drop(&mut self) {
        if let Ok(mut pins) = PINS.lock() {
            if let Some(i) = pins.iter().position(|p| p.0 == self.0) {
                pins[i].2 -= 1;
                if pins[i].2 == 0 {
                    pins.remove(i);
                }
            }
        }
    }
}

fn pin(chat: &str, root: &std::path::Path) -> Result<Pin, String> {
    let mut pins = PINS.lock().map_err(|_| "Salam Agent is busy, try again.".to_string())?;
    let pos = pins.iter().position(|p| p.0 == chat);
    if let Some(i) = pos {
        if pins[i].1 != root {
            return Err(
                "The project folder of this chat changed while it is running. Wait for the running steps to finish, then run it again."
                    .to_string(),
            );
        }
        pins[i].2 += 1;
    } else {
        pins.push((chat.to_string(), root.to_path_buf(), 1));
    }
    Ok(Pin(chat.to_string()))
}

async fn handle(app: AppHandle, http: reqwest::Client, secret: String, call: Call) {
    let opened = if call.project && call.workspace.trim().is_empty() {
        Err("This project has no folder yet. Open the project edit screen and choose an existing folder.".to_string())
    } else {
        Workspace::open(&call.workspace).map_err(|e| {
            if call.project && e.contains("does not exist") {
                format!("The project folder {} does not exist on this computer. Open the project edit screen and choose an existing folder, then run the chat again.", call.workspace)
            } else {
                e
            }
        })
    };
    let ws = match opened.and_then(|ws| ws.with_extra(&call.extra)) {
        Ok(ws) => ws,
        Err(e) => return report(&http, &secret, &call.id, "error", &e, None, None).await,
    };
    let _pin = match pin(&call.chat, ws.root()) {
        Ok(p) => p,
        Err(e) => return report(&http, &secret, &call.id, "error", &e, None, None).await,
    };
    let plan = match tools::plan(&call, &ws) {
        Ok(p) => p,
        Err(e) => return report(&http, &secret, &call.id, "error", &e, None, None).await,
    };
    let approvals = app.state::<Approvals>();
    let preview = plan.preview;
    let decision = match approvals.needs(&preview) {
        Policy::Allow => Decision::Allow,
        Policy::Deny(reason) => Decision::Deny(reason.to_string()),
        Policy::Ask => {
            emit_call(&app, &preview, "pending", "", None);
            approvals.ask(&app, &preview).await
        }
    };
    if let Decision::Deny(reason) = decision {
        emit_call(&app, &preview, "denied", &reason, None);
        return report(&http, &secret, &call.id, "denied", &reason, Some(&preview), None).await;
    }
    emit_call(&app, &preview, "running", "", None);
    let cancel = approvals.start(&call.id);
    let out_app = app.clone();
    let id = call.id.clone();
    let sink: tools::exec::Sink = Arc::new(move |chunk| {
        let _ = out_app.emit(
            "agent://output",
            OutputEvent {
                call_id: id.clone(),
                chunk,
            },
        );
    });
    let result = tools::run(plan.action, ws.root().to_path_buf(), sink, cancel).await;
    approvals.finish(&call.id);
    let (status, output, image) = match &result {
        Ok((o, image)) => ("ok", o.as_str(), image.as_ref()),
        Err(e) => ("error", e.as_str(), None),
    };
    emit_call(&app, &preview, if status == "ok" { "done" } else { "error" }, output, image);
    report(&http, &secret, &call.id, status, output, Some(&preview), image).await;
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::tools::Kind;

    fn preview() -> Preview {
        Preview {
            call_id: "call-1".into(),
            chat: "chat-1".into(),
            tool: "browser".into(),
            kind: Kind::Read,
            summary: "browser open http://localhost:3000/".into(),
            detail: String::new(),
            lang: String::new(),
            outside: false,
            cwd: "/work".into(),
            memory: false,
        }
    }

    #[test]
    fn a_chat_keeps_one_folder_while_calls_run() {
        let root = std::path::Path::new("/work/a");
        let first = pin("chat-pin", root).unwrap();
        assert!(pin("chat-pin", std::path::Path::new("/work/b")).is_err());
        let second = pin("chat-pin", root).unwrap();
        drop(first);
        assert!(pin("chat-pin", std::path::Path::new("/work/b")).is_err());
        drop(second);
        assert!(pin("chat-pin", std::path::Path::new("/work/b")).is_ok());
    }

    #[test]
    fn a_screenshot_travels_with_the_finished_call() {
        let p = preview();
        let shot = Image {
            data: "QUJD".into(),
            mime: "image/jpeg",
        };
        let event = serde_json::to_value(CallEvent {
            preview: &p,
            state: "done",
            output: "http://localhost:3000/ (200) \"Salam AI\"\nScreenshot attached (1440x1000 px, JPEG).",
            image: true,
            image_data: Some(shot.data.as_str()),
            image_mime: Some(shot.mime),
        })
        .unwrap();
        assert_eq!(event["call_id"], "call-1");
        assert_eq!(event["tool"], "browser");
        assert_eq!(event["image"], true);
        assert_eq!(event["image_data"], "QUJD");
        assert_eq!(event["image_mime"], "image/jpeg");
    }

    #[test]
    fn a_call_without_a_screenshot_carries_no_image_data() {
        let p = preview();
        let event = serde_json::to_value(CallEvent {
            preview: &p,
            state: "running",
            output: "",
            image: false,
            image_data: None,
            image_mime: None,
        })
        .unwrap();
        assert_eq!(event["image"], false);
        assert!(event.get("image_data").is_none());
        assert!(event.get("image_mime").is_none());
    }
}
