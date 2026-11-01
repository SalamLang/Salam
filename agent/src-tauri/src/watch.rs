use crate::config;
use crate::notify::{self, Kind, Notice};
use serde_json::Value;
use std::collections::HashMap;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};
use tauri::{AppHandle, Manager};

const STEP: Duration = Duration::from_secs(2);
const LIMIT: Duration = Duration::from_secs(2 * 60 * 60);
const MAX_JOBS: usize = 8;

struct Job {
    mid: i64,
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
            for job in jobs.values() {
                job.halt();
            }
            jobs.clear();
        }
        let job = Arc::new(Job {
            mid,
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
                tokio::time::sleep(STEP).await;
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
            match next_pending(&http, &base, &token, mid).await {
                Some(next) => mid = next,
                None => break,
            }
        }
        app.state::<Watchers>().done(&chat, &job);
    });
}

async fn next_pending(http: &reqwest::Client, base: &str, token: &str, after: i64) -> Option<i64> {
    for _ in 0..2 {
        tokio::time::sleep(STEP).await;
        let Ok(res) = http.get(base).bearer_auth(token).send().await else {
            continue;
        };
        if !res.status().is_success() {
            return None;
        }
        let Ok(value) = res.json::<Value>().await else { continue };
        if let Some(mid) = value["messages"]
            .as_array()
            .and_then(|items| {
                items
                    .iter()
                    .find(|m| m["role"] == "assistant" && m["status"] == "pending" && m["id"].as_i64().unwrap_or(0) > after)
            })
            .and_then(|m| m["id"].as_i64())
        {
            return Some(mid);
        }
    }
    None
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
    fn too_many_watchers_are_dropped() {
        let w = Watchers::default();
        for i in 0..MAX_JOBS {
            assert!(w.start(&format!("c{i}"), 1).is_some());
        }
        let last = w.start("extra", 1).expect("a fresh job replaces the crowd");
        assert!(!last.halted());
        assert_eq!(w.jobs.lock().unwrap().len(), 1);
    }
}
