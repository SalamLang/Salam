use futures_util::{SinkExt, StreamExt};
use serde_json::{Value, json};
use std::collections::HashMap;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tokio::sync::{mpsc, oneshot};
use tokio_tungstenite::tungstenite::Message;
use tokio_tungstenite::tungstenite::protocol::WebSocketConfig;

pub const CALL_WAIT: Duration = Duration::from_secs(20);

pub type Handler = Arc<dyn Fn(&Cdp, Option<&str>, &str, &Value) + Send + Sync>;

type Pending = Mutex<HashMap<u64, oneshot::Sender<Result<Value, String>>>>;

struct Inner {
    out: mpsc::UnboundedSender<String>,
    pending: Pending,
    next: AtomicU64,
    closed: AtomicBool,
}

#[derive(Clone)]
pub struct Cdp {
    inner: Arc<Inner>,
}

pub fn parse_active_port(text: &str) -> Option<(u16, String)> {
    let mut lines = text.lines().map(str::trim);
    let port = lines.next()?.parse::<u16>().ok().filter(|p| *p != 0)?;
    let path = lines.next().filter(|p| p.starts_with("/devtools/"))?;
    Some((port, path.to_string()))
}

impl Cdp {
    pub async fn connect(url: &str, on_event: Handler) -> Result<Cdp, String> {
        let config = WebSocketConfig::default()
            .max_message_size(Some(256 << 20))
            .max_frame_size(Some(256 << 20));
        let (ws, _) = tokio::time::timeout(
            Duration::from_secs(10),
            tokio_tungstenite::connect_async_with_config(url, Some(config), false),
        )
        .await
        .map_err(|_| "The browser did not accept a DevTools connection in time.".to_string())?
        .map_err(|e| format!("Could not connect to the browser: {e}"))?;
        let (mut sink, mut stream) = ws.split();
        let (out, mut rx) = mpsc::unbounded_channel::<String>();
        let cdp = Cdp {
            inner: Arc::new(Inner {
                out,
                pending: Mutex::new(HashMap::new()),
                next: AtomicU64::new(1),
                closed: AtomicBool::new(false),
            }),
        };
        tokio::spawn(async move {
            while let Some(text) = rx.recv().await {
                if sink.send(Message::text(text)).await.is_err() {
                    break;
                }
            }
            let _ = sink.close().await;
        });
        let reader = cdp.clone();
        tokio::spawn(async move {
            while let Some(Ok(msg)) = stream.next().await {
                let Message::Text(text) = msg else { continue };
                let Ok(v) = serde_json::from_str::<Value>(text.as_str()) else {
                    continue;
                };
                if let Some(id) = v.get("id").and_then(Value::as_u64) {
                    let waiter = reader.inner.pending.lock().unwrap().remove(&id);
                    if let Some(tx) = waiter {
                        let r = match v.get("error") {
                            Some(e) => Err(e.get("message").and_then(Value::as_str).unwrap_or("unknown error").to_string()),
                            None => Ok(v.get("result").cloned().unwrap_or(Value::Null)),
                        };
                        let _ = tx.send(r);
                    }
                } else if let Some(method) = v.get("method").and_then(Value::as_str) {
                    let session = v.get("sessionId").and_then(Value::as_str);
                    on_event(&reader, session, method, v.get("params").unwrap_or(&Value::Null));
                }
            }
            reader.inner.closed.store(true, Ordering::Relaxed);
            for (_, tx) in reader.inner.pending.lock().unwrap().drain() {
                let _ = tx.send(Err("The browser closed the connection.".to_string()));
            }
        });
        Ok(cdp)
    }

    pub fn closed(&self) -> bool {
        self.inner.closed.load(Ordering::Relaxed)
    }

    fn message(&self, session: Option<&str>, method: &str, params: Value) -> (u64, String) {
        let id = self.inner.next.fetch_add(1, Ordering::Relaxed);
        let mut m = json!({ "id": id, "method": method, "params": params });
        if let Some(s) = session {
            m["sessionId"] = json!(s);
        }
        (id, m.to_string())
    }

    pub fn send(&self, session: Option<&str>, method: &str, params: Value) {
        let (_, text) = self.message(session, method, params);
        let _ = self.inner.out.send(text);
    }

    pub async fn call(&self, session: Option<&str>, method: &str, params: Value) -> Result<Value, String> {
        if self.closed() {
            return Err("The browser closed the connection.".to_string());
        }
        let (id, text) = self.message(session, method, params);
        let (tx, rx) = oneshot::channel();
        self.inner.pending.lock().unwrap().insert(id, tx);
        if self.inner.out.send(text).is_err() {
            self.inner.pending.lock().unwrap().remove(&id);
            return Err("The browser closed the connection.".to_string());
        }
        match tokio::time::timeout(CALL_WAIT, rx).await {
            Ok(Ok(r)) => r.map_err(|e| format!("{method} failed: {e}")),
            Ok(Err(_)) => Err("The browser closed the connection.".to_string()),
            Err(_) => {
                self.inner.pending.lock().unwrap().remove(&id);
                Err(format!("The browser did not answer {method} in time."))
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reads_devtools_active_port() {
        assert_eq!(
            parse_active_port("41823\n/devtools/browser/0a1b-2c\n"),
            Some((41823, "/devtools/browser/0a1b-2c".to_string()))
        );
        assert_eq!(
            parse_active_port("9222\r\n/devtools/browser/x\r\n"),
            Some((9222, "/devtools/browser/x".to_string()))
        );
        assert_eq!(parse_active_port(""), None);
        assert_eq!(parse_active_port("41823\n"), None);
        assert_eq!(parse_active_port("0\n/devtools/browser/x"), None);
        assert_eq!(parse_active_port("port\n/devtools/browser/x"), None);
        assert_eq!(parse_active_port("41823\n/other"), None);
    }
}
