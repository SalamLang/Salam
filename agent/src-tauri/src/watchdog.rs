use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};
use tauri::{AppHandle, Manager, WebviewWindow};

const CHECK: Duration = Duration::from_secs(3);
const SILENCE: Duration = Duration::from_secs(45);
const PROBE: Duration = Duration::from_secs(15);
const RECOVER_GAP: Duration = Duration::from_secs(120);

#[derive(Default)]
pub struct Watchdog {
    last: Mutex<Option<Instant>>,
    recovered: Mutex<Option<Instant>>,
    armed: AtomicBool,
}

impl Watchdog {
    pub fn arm(&self) {
        *self.last.lock().unwrap() = Some(Instant::now());
        self.armed.store(true, Ordering::Relaxed);
    }

    pub fn beat(&self) {
        *self.last.lock().unwrap() = Some(Instant::now());
    }

    pub fn disarm(&self) {
        self.armed.store(false, Ordering::Relaxed);
    }

    fn recover(&self) -> bool {
        let mut last = self.recovered.lock().unwrap();
        if last.is_some_and(|at| at.elapsed() < RECOVER_GAP) {
            return false;
        }
        *last = Some(Instant::now());
        true
    }

    fn silence(&self) -> Option<Duration> {
        self.last.lock().unwrap().map(|last| last.elapsed())
    }
}

pub fn arm(app: &AppHandle) {
    app.state::<Watchdog>().arm();
}

pub fn beat(app: &AppHandle) {
    app.state::<Watchdog>().beat();
}

pub fn disarm(app: &AppHandle) {
    app.state::<Watchdog>().disarm();
}

enum Reach {
    Ok,
    Server,
    Offline,
}

async fn reach() -> Reach {
    let client = match reqwest::Client::builder().timeout(Duration::from_secs(6)).build() {
        Ok(client) => client,
        Err(_) => return Reach::Offline,
    };
    match client.get(crate::config::app_url()).send().await {
        Ok(response) if response.status().as_u16() < 500 => Reach::Ok,
        Ok(_) => Reach::Server,
        Err(_) => Reach::Offline,
    }
}

async fn responds(window: &WebviewWindow) -> bool {
    let answered = Arc::new(AtomicBool::new(false));
    let flag = answered.clone();
    if window.eval_with_callback("1", move |_| flag.store(true, Ordering::Relaxed)).is_err() {
        return false;
    }
    let deadline = Instant::now() + PROBE;
    while Instant::now() < deadline && !answered.load(Ordering::Relaxed) {
        tokio::time::sleep(Duration::from_millis(200)).await;
    }
    answered.load(Ordering::Relaxed)
}

pub async fn run(app: AppHandle) {
    loop {
        tokio::time::sleep(CHECK).await;
        let watchdog = app.state::<Watchdog>();
        if !watchdog.armed.load(Ordering::Relaxed) {
            continue;
        }
        let Some(window) = app.get_webview_window("main") else {
            continue;
        };
        let awake = window.is_visible().unwrap_or(false) && !window.is_minimized().unwrap_or(true) && window.is_focused().unwrap_or(false);
        if !awake {
            watchdog.beat();
            continue;
        }
        let Some(silence) = watchdog.silence() else {
            continue;
        };
        if silence < SILENCE {
            continue;
        }
        watchdog.disarm();
        if responds(&window).await {
            watchdog.arm();
            continue;
        }
        let reason = match reach().await {
            Reach::Ok => "crash",
            Reach::Server => "server",
            Reach::Offline => "offline",
        };
        if reason == "crash" && watchdog.recover() {
            let _ = window.navigate(crate::config::app_url());
            continue;
        }
        let _ = window.navigate(crate::config::local_url(&format!("error.html#{reason}")));
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_starts_disarmed_and_tracks_the_heartbeat() {
        let watchdog = Watchdog::default();
        assert!(!watchdog.armed.load(Ordering::Relaxed));
        watchdog.arm();
        assert!(watchdog.armed.load(Ordering::Relaxed));
        assert!(watchdog.silence().is_some_and(|age| age < Duration::from_secs(2)));
        watchdog.beat();
        assert!(watchdog.armed.load(Ordering::Relaxed));
        watchdog.disarm();
        assert!(!watchdog.armed.load(Ordering::Relaxed));
    }

    #[test]
    fn it_recovers_silently_only_once_in_a_while() {
        let watchdog = Watchdog::default();
        assert!(watchdog.recover());
        assert!(!watchdog.recover());
    }

    #[test]
    fn an_unbeated_watchdog_keeps_its_last_beat() {
        let watchdog = Watchdog::default();
        watchdog.arm();
        let first = watchdog.silence().unwrap();
        std::thread::sleep(Duration::from_millis(5));
        assert!(watchdog.silence().unwrap() >= first);
    }

    #[test]
    fn local_error_pages_stay_on_the_app_origin() {
        let url = crate::config::local_url("error.html#memory");
        assert!(crate::config::is_local_asset(&url));
        assert_eq!(url.fragment(), Some("memory"));
        assert!(url.path().ends_with("error.html"));
    }
}
