use crate::approve::{Approvals, Decision, Policy};
use crate::config;
use crate::device::DeviceStore;
use crate::notify::{self, Kind, Notice};
use crate::tools::{self, Call, Preview};
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

fn emit_call(app: &AppHandle, preview: &Preview, state: &str, output: &str) {
    let _ = app.emit("agent://call", CallEvent { preview, state, output });
}

async fn report(http: &reqwest::Client, secret: &str, call_id: &str, status: &str, output: &str, preview: Option<&Preview>) {
    let mut body = json!({ "call_id": call_id, "status": status, "output": output });
    if let Some(p) = preview {
        body["tool"] = json!(p.tool);
        body["kind"] = json!(p.kind);
        body["summary"] = json!(p.summary);
        body["detail"] = json!(p.detail);
        body["lang"] = json!(p.lang);
        body["outside"] = json!(p.outside);
        body["cwd"] = json!(p.cwd);
    }
    for _ in 0..3 {
        let sent = http
            .post(format!("{}/bridge/result", config::bridge_url()))
            .bearer_auth(secret)
            .json(&body)
            .send()
            .await;
        if matches!(&sent, Ok(r) if r.status().is_success() || r.status().as_u16() == 404) {
            return;
        }
        tokio::time::sleep(Duration::from_secs(1)).await;
    }
}

async fn handle(app: AppHandle, http: reqwest::Client, secret: String, call: Call) {
    let ws = match Workspace::open(&call.workspace) {
        Ok(ws) => ws,
        Err(e) => return report(&http, &secret, &call.id, "error", &e, None).await,
    };
    let plan = match tools::plan(&call, &ws) {
        Ok(p) => p,
        Err(e) => return report(&http, &secret, &call.id, "error", &e, None).await,
    };
    let approvals = app.state::<Approvals>();
    let preview = plan.preview;
    let decision = match approvals.needs(&preview) {
        Policy::Allow => Decision::Allow,
        Policy::Deny(reason) => Decision::Deny(reason.to_string()),
        Policy::Ask => {
            emit_call(&app, &preview, "pending", "");
            approvals.ask(&app, &preview).await
        }
    };
    if let Decision::Deny(reason) = decision {
        emit_call(&app, &preview, "denied", &reason);
        return report(&http, &secret, &call.id, "denied", &reason, Some(&preview)).await;
    }
    emit_call(&app, &preview, "running", "");
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
    let result = tools::execute(plan.action, ws.root().to_path_buf(), sink, cancel).await;
    approvals.finish(&call.id);
    let (status, output) = match &result {
        Ok(o) => ("ok", o.as_str()),
        Err(e) => ("error", e.as_str()),
    };
    emit_call(&app, &preview, if status == "ok" { "done" } else { "error" }, output);
    report(&http, &secret, &call.id, status, output, Some(&preview)).await;
}
