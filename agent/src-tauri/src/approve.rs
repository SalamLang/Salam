use crate::tools::{Kind, Preview};
use serde::Deserialize;
use std::collections::{HashMap, HashSet};
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tauri::{AppHandle, Manager, UserAttentionType};
use tokio::sync::{Notify, oneshot};

const ANSWER_WAIT: Duration = Duration::from_secs(9 * 60);

#[derive(Clone, Copy, Debug, Default, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Mode {
    #[default]
    Ask,
    Auto,
    Readonly,
}

#[derive(Clone, Copy, Debug, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Answer {
    Allow,
    Always,
    Deny,
}

#[derive(Debug, PartialEq, Eq)]
pub enum Policy {
    Allow,
    Ask,
    Deny(&'static str),
}

pub enum Decision {
    Allow,
    Deny(String),
}

pub fn policy(mode: Mode, kind: Kind, outside: bool, always: bool) -> Policy {
    match mode {
        Mode::Readonly => match (kind, outside) {
            (Kind::Read, false) => Policy::Allow,
            (Kind::Read, true) => Policy::Ask,
            _ => Policy::Deny(
                "The user set this conversation to read-only, so nothing may be changed or run. Explain what you would do instead.",
            ),
        },
        Mode::Ask => {
            if !outside && (kind == Kind::Read || always) {
                Policy::Allow
            } else {
                Policy::Ask
            }
        }
        Mode::Auto => {
            if outside {
                Policy::Ask
            } else {
                Policy::Allow
            }
        }
    }
}

#[derive(Default)]
pub struct Approvals {
    modes: Mutex<HashMap<String, Mode>>,
    always: Mutex<HashSet<(String, String)>>,
    pending: Mutex<HashMap<String, (Preview, oneshot::Sender<Answer>)>>,
    running: Mutex<HashMap<String, Arc<Notify>>>,
}

impl Approvals {
    pub fn set_mode(&self, chat: String, mode: Mode) {
        self.modes.lock().unwrap().insert(chat, mode);
    }

    pub fn needs(&self, p: &Preview) -> Policy {
        let mode = self.modes.lock().unwrap().get(&p.chat).copied().unwrap_or_default();
        let always = self.always.lock().unwrap().contains(&(p.chat.clone(), p.tool.clone()));
        policy(mode, p.kind, p.outside, always)
    }

    pub async fn ask(&self, app: &AppHandle, p: &Preview) -> Decision {
        let (tx, rx) = oneshot::channel();
        self.pending.lock().unwrap().insert(p.call_id.clone(), (p.clone(), tx));
        if let Some(w) = app.get_webview_window("main") {
            let _ = w.request_user_attention(Some(UserAttentionType::Informational));
        }
        let answer = tokio::time::timeout(ANSWER_WAIT, rx).await;
        self.pending.lock().unwrap().remove(&p.call_id);
        match answer {
            Ok(Ok(Answer::Allow)) => Decision::Allow,
            Ok(Ok(Answer::Always)) => {
                self.always.lock().unwrap().insert((p.chat.clone(), p.tool.clone()));
                Decision::Allow
            }
            Ok(Ok(Answer::Deny)) => Decision::Deny(String::new()),
            _ => Decision::Deny("The user did not answer the approval request in time.".to_string()),
        }
    }

    pub fn answer(&self, call_id: &str, a: Answer) -> bool {
        match self.pending.lock().unwrap().remove(call_id) {
            Some((_, tx)) => tx.send(a).is_ok(),
            None => false,
        }
    }

    pub fn pending(&self) -> Vec<Preview> {
        self.pending.lock().unwrap().values().map(|(p, _)| p.clone()).collect()
    }

    pub fn start(&self, call_id: &str) -> Arc<Notify> {
        let n = Arc::new(Notify::new());
        self.running.lock().unwrap().insert(call_id.to_string(), n.clone());
        n
    }

    pub fn finish(&self, call_id: &str) {
        self.running.lock().unwrap().remove(call_id);
    }

    pub fn stop(&self, call_id: &str) -> bool {
        if self.answer(call_id, Answer::Deny) {
            return true;
        }
        match self.running.lock().unwrap().get(call_id) {
            Some(n) => {
                n.notify_one();
                true
            }
            None => false,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ask_mode_runs_reads_and_asks_for_the_rest() {
        assert_eq!(policy(Mode::Ask, Kind::Read, false, false), Policy::Allow);
        assert_eq!(policy(Mode::Ask, Kind::Read, true, false), Policy::Ask);
        assert_eq!(policy(Mode::Ask, Kind::Write, false, false), Policy::Ask);
        assert_eq!(policy(Mode::Ask, Kind::Exec, false, false), Policy::Ask);
        assert_eq!(policy(Mode::Ask, Kind::Exec, false, true), Policy::Allow);
        assert_eq!(policy(Mode::Ask, Kind::Exec, true, true), Policy::Ask);
    }

    #[test]
    fn auto_mode_still_asks_outside_the_workspace() {
        assert_eq!(policy(Mode::Auto, Kind::Exec, false, false), Policy::Allow);
        assert_eq!(policy(Mode::Auto, Kind::Write, true, false), Policy::Ask);
    }

    #[test]
    fn read_only_refuses_changes() {
        assert_eq!(policy(Mode::Readonly, Kind::Read, false, false), Policy::Allow);
        assert!(matches!(policy(Mode::Readonly, Kind::Write, false, true), Policy::Deny(_)));
        assert!(matches!(policy(Mode::Readonly, Kind::Exec, false, false), Policy::Deny(_)));
    }

    #[tokio::test]
    async fn stop_denies_a_pending_call_and_wakes_a_running_one() {
        let a = Approvals::default();
        let (tx, rx) = oneshot::channel();
        let p = Preview {
            call_id: "c".into(),
            chat: "x".into(),
            tool: "shell".into(),
            kind: Kind::Exec,
            summary: String::new(),
            detail: String::new(),
            lang: String::new(),
            outside: false,
            cwd: String::new(),
        };
        a.pending.lock().unwrap().insert("c".into(), (p, tx));
        assert_eq!(a.pending().len(), 1);
        assert!(a.stop("c"));
        assert_eq!(rx.await.unwrap(), Answer::Deny);
        let n = a.start("r");
        assert!(a.stop("r"));
        tokio::time::timeout(Duration::from_secs(1), n.notified()).await.unwrap();
        a.finish("r");
        assert!(!a.stop("r"));
    }
}
