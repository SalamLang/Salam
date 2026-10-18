use std::collections::VecDeque;
use std::process::Stdio;
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tokio::io::{AsyncRead, AsyncReadExt};
use tokio::process::Command;
use tokio::sync::Notify;

pub const OUTPUT_CAP: usize = 30_000;

pub type Sink = Arc<dyn Fn(String) + Send + Sync>;

pub struct Capped {
    head: Vec<u8>,
    tail: VecDeque<u8>,
    total: usize,
    half: usize,
}

impl Capped {
    pub fn new(cap: usize) -> Self {
        Capped {
            head: Vec::new(),
            tail: VecDeque::new(),
            total: 0,
            half: cap / 2,
        }
    }

    pub fn push(&mut self, bytes: &[u8]) {
        self.total += bytes.len();
        for &b in bytes {
            if self.head.len() < self.half {
                self.head.push(b);
            } else {
                if self.tail.len() == self.half {
                    self.tail.pop_front();
                }
                self.tail.push_back(b);
            }
        }
    }

    pub fn finish(&self) -> String {
        let kept = self.head.len() + self.tail.len();
        let head = String::from_utf8_lossy(&self.head).to_string();
        let tail: Vec<u8> = self.tail.iter().copied().collect();
        if kept < self.total {
            format!(
                "{head}\n[... {} bytes cut ...]\n{}",
                self.total - kept,
                String::from_utf8_lossy(&tail)
            )
        } else {
            head + &String::from_utf8_lossy(&tail)
        }
    }
}

pub fn cap_text(text: &str) -> String {
    let mut c = Capped::new(OUTPUT_CAP);
    c.push(text.as_bytes());
    c.finish()
}

async fn pump<R: AsyncRead + Unpin>(mut r: R, buf: Arc<Mutex<Capped>>, sink: Sink) {
    let mut chunk = vec![0u8; 8192];
    loop {
        match r.read(&mut chunk).await {
            Ok(0) | Err(_) => break,
            Ok(n) => {
                buf.lock().unwrap().push(&chunk[..n]);
                sink(String::from_utf8_lossy(&chunk[..n]).to_string());
            }
        }
    }
}

fn kill_tree(pid: Option<u32>) {
    let Some(pid) = pid else { return };
    #[cfg(unix)]
    unsafe {
        libc::killpg(pid as i32, libc::SIGKILL);
    }
    #[cfg(windows)]
    {
        use std::os::windows::process::CommandExt;
        let _ = std::process::Command::new("taskkill")
            .args(["/PID", &pid.to_string(), "/T", "/F"])
            .creation_flags(0x0800_0000)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status();
    }
}

pub fn command(program: &str) -> std::process::Command {
    #[allow(unused_mut)]
    let mut c = std::process::Command::new(program);
    #[cfg(windows)]
    {
        use std::os::windows::process::CommandExt;
        c.creation_flags(0x0800_0000);
    }
    c
}

pub async fn run(cmd: std::process::Command, timeout: Duration, sink: Sink, cancel: Arc<Notify>) -> Result<String, String> {
    let program = cmd.get_program().to_string_lossy().to_string();
    let mut cmd = Command::from(cmd);
    cmd.stdin(Stdio::null())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .kill_on_drop(true);
    #[cfg(unix)]
    cmd.process_group(0);
    let mut child = match cmd.spawn() {
        Ok(c) => c,
        Err(e) if e.kind() == std::io::ErrorKind::NotFound => return Err(format!("{program} is not installed or not on PATH.")),
        Err(e) => return Err(format!("Could not start {program}: {e}")),
    };
    let pid = child.id();
    let buf = Arc::new(Mutex::new(Capped::new(OUTPUT_CAP)));
    let out = child.stdout.take().map(|o| tokio::spawn(pump(o, buf.clone(), sink.clone())));
    let err = child.stderr.take().map(|e| tokio::spawn(pump(e, buf.clone(), sink.clone())));
    let mut note = String::new();
    let status = tokio::select! {
        s = child.wait() => s.ok(),
        _ = tokio::time::sleep(timeout) => {
            kill_tree(pid);
            note = format!("\n[timed out after {} seconds and was stopped]", timeout.as_secs());
            child.wait().await.ok()
        }
        _ = cancel.notified() => {
            kill_tree(pid);
            note = "\n[stopped by the user]".to_string();
            child.wait().await.ok()
        }
    };
    for h in [out, err].into_iter().flatten() {
        let _ = tokio::time::timeout(Duration::from_secs(2), h).await;
    }
    let mut text = buf.lock().unwrap().finish();
    if text.trim().is_empty() {
        text = "(no output)".to_string();
    }
    text.push_str(&note);
    match status.and_then(|s| s.code()) {
        Some(0) if note.is_empty() => Ok(text),
        Some(code) => Err(format!("{text}\n[exit code {code}]")),
        None => Err(text),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn quiet() -> Sink {
        Arc::new(|_| {})
    }

    #[test]
    fn capped_keeps_head_and_tail() {
        let mut c = Capped::new(10);
        c.push(b"0123456789abcdefghij");
        let out = c.finish();
        assert!(out.starts_with("01234"));
        assert!(out.ends_with("fghij"));
        assert!(out.contains("10 bytes cut"));
        let mut small = Capped::new(10);
        small.push(b"abc");
        assert_eq!(small.finish(), "abc");
    }

    #[cfg(unix)]
    #[tokio::test]
    async fn runs_and_reports_exit_codes() {
        let mut ok = command("sh");
        ok.args(["-c", "echo hi; echo err >&2"]);
        let out = run(ok, Duration::from_secs(10), quiet(), Arc::new(Notify::new())).await.unwrap();
        assert!(out.contains("hi") && out.contains("err"));
        let mut bad = command("sh");
        bad.args(["-c", "exit 3"]);
        let e = run(bad, Duration::from_secs(10), quiet(), Arc::new(Notify::new()))
            .await
            .unwrap_err();
        assert!(e.contains("exit code 3"));
    }

    #[cfg(unix)]
    #[tokio::test]
    async fn timeouts_kill_the_whole_tree() {
        let mut c = command("sh");
        c.args(["-c", "sleep 30 & sleep 30; echo never"]);
        let started = std::time::Instant::now();
        let e = run(c, Duration::from_secs(1), quiet(), Arc::new(Notify::new())).await.unwrap_err();
        assert!(e.contains("timed out"));
        assert!(started.elapsed() < Duration::from_secs(10));
    }

    #[cfg(unix)]
    #[tokio::test]
    async fn stop_cancels_a_running_command() {
        let cancel = Arc::new(Notify::new());
        let c2 = cancel.clone();
        tokio::spawn(async move {
            tokio::time::sleep(Duration::from_millis(300)).await;
            c2.notify_one();
        });
        let mut c = command("sh");
        c.args(["-c", "sleep 30"]);
        let e = run(c, Duration::from_secs(20), quiet(), cancel).await.unwrap_err();
        assert!(e.contains("stopped by the user"));
    }

    #[tokio::test]
    async fn missing_programs_are_explained() {
        let c = command("salam-no-such-program-xyz");
        let e = run(c, Duration::from_secs(5), quiet(), Arc::new(Notify::new())).await.unwrap_err();
        assert!(e.contains("not installed"));
    }
}
