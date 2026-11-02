use crate::config;
use crate::notify::{self, Kind, Notice};
use serde_json::Value;
use std::collections::HashMap;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};
use tauri::{AppHandle, Manager};

const STEP: Duration = Duration::from_secs(2);
const LIMIT: Duration = Duration::from_secs(2 * 60 * 60);
const MAX_JOBS: usize = 8;

struct Job {
    mid: i64,
    started: u64,
    stop: AtomicBool,
}

impl Job {
    fn halt(&self) {
        self.stop.store(true, Ordering::Relaxed);
    }

    fn halted(&self) -> bool {
        self.stop.load(Ordering::Relaxed)
    }
}

#[derive(Default)]
pub struct Watchers {
    jobs: Mutex<HashMap<String, Arc<Job>>>,
    seq: AtomicU64,
}

impl Watchers {
    fn start(&self, chat: &str, mid: i64) -> Option<Arc<Job>> {
        let mut jobs = self.jobs.lock().unwrap();
        if let Some(old) = jobs.get(chat) {
            if old.mid == mid {
                return None;
            }
            old.halt();
        }
        if jobs.len() >= MAX_JOBS {
            if let Some(oldest) = jobs.iter().min_by_key(|(_, job)| job.started).map(|(chat, _)| chat.clone()) {
                if let Some(job) = jobs.remove(&oldest) {
                    job.halt();
                }
            }
        }
        let job = Arc::new(Job {
            mid,
            started: self.seq.fetch_add(1, Ordering::Relaxed),
            stop: AtomicBool::new(false),
        });
        jobs.insert(chat.to_string(), job.clone());
        Some(job)
    }

    pub fn stop(&self, chat: &str) {
        if let Some(job) = self.jobs.lock().unwrap().remove(chat) {
            job.halt();
        }
    }

    pub fn stop_all(&self) {
        let mut jobs = self.jobs.lock().unwrap();
        for job in jobs.values() {
            job.halt();
        }
        jobs.clear();
    }

    fn done(&self, chat: &str, job: &Arc<Job>) {
        let mut jobs = self.jobs.lock().unwrap();
        if jobs.get(chat).is_some_and(|cur| Arc::ptr_eq(cur, job)) {
            jobs.remove(chat);
        }
    }
}

pub fn watch(app: &AppHandle, chat: String, mid: i64, token: String, title: String) {
    let Some(job) = app.state::<Watchers>().start(&chat, mid) else {
        return;
    };
    let app = app.clone();
    tauri::async_runtime::spawn(async move {
        let Ok(http) = reqwest::Client::builder()
            .timeout(Duration::from_secs(12))
            .user_agent(concat!("salam-agent/", env!("CARGO_PKG_VERSION")))
            .build()
        else {
            return;
        };
        let base = format!("{}/chats/{}", config::api_url(), chat);
        let start = Instant::now();
        let mut mid = job.mid;
        'watch: loop {
            let url = format!("{base}/messages/{mid}");
            let mut finished: Option<(Kind, String)> = None;
            while !job.halted() && start.elapsed() < LIMIT {
                tokio::time::sleep(step(start.elapsed())).await;
                if job.halted() {
                    break 'watch;
                }
                if !notify::away(&app) || !app.state::<notify::Notifier>().prefs().enabled {
                    continue;
                }
                let answer = http.get(&url).bearer_auth(&token).send().await;
                let value = match answer {
                    Ok(res) if res.status().as_u16() == 401 => break 'watch,
                    Ok(res) if res.status().is_success() => match res.json::<Value>().await {
                        Ok(value) => value,
                        Err(_) => continue,
                    },
                    _ => continue,
                };
                let status = value["status"].as_str().unwrap_or("pending");
                if status == "pending" {
                    continue;
                }
                let body = value["body"].as_str().unwrap_or("").to_string();
                let kind = if status == "error" { Kind::Failed } else { Kind::Done };
                finished = Some((kind, body));
                break;
            }
            let Some((kind, body)) = finished else { break };
            let mut notice = Notice::new(kind).text(body).mid(mid);
            notice.chat = title.clone();
            notify::post(&app, notice);
            match next_reply(&http, &base, &token, mid).await {
                Some(next) => mid = next,
                None => break,
            }
        }
        app.state::<Watchers>().done(&chat, &job);
    });
}

fn step(elapsed: Duration) -> Duration {
    if elapsed > Duration::from_secs(5 * 60) {
        Duration::from_secs(10)
    } else {
        STEP
    }
}

async fn next_reply(http: &reqwest::Client, base: &str, token: &str, after: i64) -> Option<i64> {
    for attempt in 0..2 {
        if attempt > 0 {
            tokio::time::sleep(STEP).await;
        }
        let Ok(res) = http.get(base).bearer_auth(token).send().await else {
            continue;
        };
        if !res.status().is_success() {
            return None;
        }
        let Ok(value) = res.json::<Value>().await else { continue };
        if let Some(mid) = pick_next(&value["messages"], after) {
            return Some(mid);
        }
    }
    None
}

fn pick_next(messages: &Value, after: i64) -> Option<i64> {
    let items = messages.as_array()?;
    items
        .iter()
        .filter(|m| m["role"] == "assistant")
        .filter_map(|m| m["id"].as_i64())
        .filter(|id| *id > after)
        .min()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn watching_the_same_reply_twice_keeps_one_job() {
        let w = Watchers::default();
        let first = w.start("c", 4).expect("first job");
        assert!(w.start("c", 4).is_none());
        let second = w.start("c", 5).expect("new reply");
        assert!(first.halted());
        assert!(!second.halted());
        w.done("c", &first);
        assert!(w.start("c", 6).is_some());
    }

    #[test]
    fn stopping_a_chat_halts_its_job() {
        let w = Watchers::default();
        let job = w.start("c", 1).unwrap();
        w.stop("c");
        assert!(job.halted());
        assert!(w.start("c", 2).is_some());
    }

    #[test]
    fn a_ninth_watcher_evicts_the_oldest_only() {
        let w = Watchers::default();
        let oldest = w.start("c0", 1).expect("the first job");
        for i in 1..MAX_JOBS {
            assert!(w.start(&format!("c{i}"), 1).is_some());
        }
        let fresh = w.start("extra", 1).expect("a fresh job fits in");
        assert!(oldest.halted());
        assert!(!fresh.halted());
        let jobs = w.jobs.lock().unwrap();
        assert_eq!(jobs.len(), MAX_JOBS);
        assert!(!jobs.contains_key("c0"));
        assert!(jobs.contains_key("c1"));
    }

    #[test]
    fn the_next_reply_is_the_oldest_assistant_message_after_the_announced_one() {
        let messages = serde_json::json!([
            {"id": 4, "role": "user"},
            {"id": 5, "role": "assistant", "status": "done"},
            {"id": 6, "role": "assistant", "status": "done"},
            {"id": 7, "role": "assistant", "status": "pending"}
        ]);
        assert_eq!(pick_next(&messages, 5), Some(6));
        assert_eq!(pick_next(&messages, 7), None);
        assert_eq!(pick_next(&serde_json::json!([]), 1), None);
        assert_eq!(pick_next(&serde_json::json!({}), 1), None);
    }

    #[test]
    fn a_long_wait_is_polled_less_often() {
        assert_eq!(step(Duration::from_secs(10)), STEP);
        assert_eq!(step(Duration::from_secs(10 * 60)), Duration::from_secs(10));
    }
}
