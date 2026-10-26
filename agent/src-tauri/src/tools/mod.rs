pub mod detect;
pub mod exec;
pub mod fs;
pub mod search;

use crate::memory;
use crate::workspace::{Workspace, in_git_dir};
use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::path::PathBuf;
use std::sync::Arc;
use std::time::Duration;
use tokio::sync::Notify;

const MAX_DIFF: usize = 20_000;

#[derive(Clone, Deserialize)]
pub struct Call {
    pub id: String,
    pub tool: String,
    #[serde(default)]
    pub args: Value,
    #[serde(default)]
    pub chat: String,
    #[serde(default)]
    pub workspace: String,
}

#[derive(Clone, Copy, Debug, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Kind {
    Read,
    Write,
    Exec,
}

#[derive(Clone, Serialize)]
pub struct Preview {
    pub call_id: String,
    pub chat: String,
    pub tool: String,
    pub kind: Kind,
    pub summary: String,
    pub detail: String,
    pub lang: String,
    pub outside: bool,
    pub cwd: String,
    #[serde(default)]
    pub memory: bool,
}

pub enum Action {
    Run {
        program: String,
        args: Vec<String>,
        raw: Option<String>,
        cwd: PathBuf,
        timeout: u64,
    },
    WslList,
    Read {
        path: PathBuf,
        offset: usize,
        limit: usize,
    },
    Write {
        path: PathBuf,
        content: String,
    },
    Edit {
        path: PathBuf,
        before: String,
        after: String,
        count: usize,
    },
    List {
        path: PathBuf,
    },
    Glob {
        base: PathBuf,
        pattern: String,
    },
    Grep {
        base: PathBuf,
        pattern: String,
        glob: String,
        ignore_case: bool,
    },
    SaveMemory {
        slug: String,
        title: String,
        description: String,
        content: String,
    },
    ForgetMemory {
        slug: String,
    },
}

pub struct Plan {
    pub preview: Preview,
    pub action: Action,
}

fn text(args: &Value, key: &str) -> String {
    args.get(key).and_then(|v| v.as_str()).unwrap_or("").to_string()
}

fn number(args: &Value, key: &str) -> Option<u64> {
    args.get(key)
        .and_then(|v| v.as_u64().or_else(|| v.as_str().and_then(|s| s.parse().ok())))
}

fn flag(args: &Value, key: &str) -> bool {
    args.get(key).and_then(|v| v.as_bool()).unwrap_or(false)
}

fn timeout_of(args: &Value) -> u64 {
    number(args, "timeout").unwrap_or(120).clamp(1, 1800)
}

fn quote(arg: &str) -> String {
    if !arg.is_empty() && arg.chars().all(|c| c.is_ascii_alphanumeric() || "-_./:=@,+".contains(c)) {
        arg.to_string()
    } else {
        format!("'{}'", arg.replace('\'', "'\\''"))
    }
}

fn first_line(s: &str, max: usize) -> String {
    let line = s.lines().find(|l| !l.trim().is_empty()).unwrap_or("").trim();
    let mut out: String = line.chars().take(max).collect();
    if line.chars().count() > max || s.trim().lines().count() > 1 {
        out.push_str(" …");
    }
    out
}

fn diff(path: &str, before: &str, after: &str) -> String {
    let d = similar::TextDiff::from_lines(before, after);
    let mut out = d.unified_diff().context_radius(3).header(path, path).to_string();
    if out.len() > MAX_DIFF {
        let mut cut = MAX_DIFF;
        while !out.is_char_boundary(cut) {
            cut -= 1;
        }
        out.truncate(cut);
        out.push_str("\n[... diff cut ...]");
    }
    out
}

#[cfg(unix)]
fn shell_parts(shell: &str, command: &str) -> (String, Vec<String>, Option<String>, String) {
    let c = command.to_string();
    match shell {
        "bash" => ("bash".into(), vec!["-lc".into(), c], None, "bash".into()),
        "sh" => ("sh".into(), vec!["-c".into(), c], None, "bash".into()),
        _ => {
            let login = std::env::var("SHELL")
                .ok()
                .filter(|s| !s.is_empty())
                .unwrap_or_else(|| "/bin/sh".into());
            (login, vec!["-lc".into(), c], None, "bash".into())
        }
    }
}

#[cfg(windows)]
fn shell_parts(shell: &str, command: &str) -> (String, Vec<String>, Option<String>, String) {
    match shell {
        "cmd" => (
            "cmd.exe".into(),
            vec![],
            Some(format!("/D /S /C \"chcp 65001>nul & {command}\"")),
            "batch".into(),
        ),
        "bash" | "sh" => ("bash.exe".into(), vec!["-lc".into(), command.to_string()], None, "bash".into()),
        _ => (
            "powershell.exe".into(),
            vec![
                "-NoLogo".into(),
                "-NoProfile".into(),
                "-NonInteractive".into(),
                "-ExecutionPolicy".into(),
                "Bypass".into(),
                "-Command".into(),
                format!("[Console]::OutputEncoding=[Text.Encoding]::UTF8; {command}"),
            ],
            None,
            "powershell".into(),
        ),
    }
}

fn memory_inside(ws: &Workspace, slug: &str) -> Result<String, String> {
    let shown = memory::rel(slug);
    if !ws.resolve(&shown).inside {
        return Err(format!("{shown} points outside the workspace, so Salam Agent will not touch it."));
    }
    Ok(shown)
}

pub fn plan(call: &Call, ws: &Workspace) -> Result<Plan, String> {
    let a = &call.args;
    let mut preview = Preview {
        call_id: call.id.clone(),
        chat: call.chat.clone(),
        tool: call.tool.clone(),
        kind: Kind::Read,
        summary: String::new(),
        detail: String::new(),
        lang: String::new(),
        outside: false,
        cwd: ws.display(ws.root()),
        memory: false,
    };
    let cwd_of = |preview: &mut Preview| {
        let r = ws.resolve(&text(a, "cwd"));
        preview.outside = !r.inside;
        preview.cwd = ws.display(&r.path);
        r.path
    };
    let action = match call.tool.as_str() {
        "shell" | "run_tests" => {
            let cwd = cwd_of(&mut preview);
            if !cwd.is_dir() {
                return Err(format!("The folder {} does not exist.", cwd.display()));
            }
            let mut command = text(a, "command");
            if call.tool == "run_tests" && command.trim().is_empty() {
                command =
                    detect::test_command(&cwd).ok_or_else(|| format!("No test runner found in {}; pass command.", ws.display(&cwd)))?;
            }
            if command.trim().is_empty() {
                return Err("command is empty.".to_string());
            }
            let (program, args, raw, lang) = shell_parts(&text(a, "shell"), &command);
            preview.kind = Kind::Exec;
            preview.summary = first_line(&command, 120);
            preview.detail = command;
            preview.lang = lang;
            Action::Run {
                program,
                args,
                raw,
                cwd,
                timeout: timeout_of(a),
            }
        }
        "docker" => {
            let cwd = cwd_of(&mut preview);
            let args: Vec<String> = a
                .get("args")
                .and_then(|v| v.as_array())
                .map(|v| v.iter().filter_map(|x| x.as_str().map(String::from)).collect())
                .unwrap_or_default();
            if args.is_empty() {
                return Err("args is empty; pass the docker arguments as a list.".to_string());
            }
            let shown = format!("docker {}", args.iter().map(|x| quote(x)).collect::<Vec<_>>().join(" "));
            preview.kind = Kind::Exec;
            preview.summary = first_line(&shown, 120);
            preview.detail = shown;
            preview.lang = "bash".into();
            Action::Run {
                program: "docker".into(),
                args,
                raw: None,
                cwd,
                timeout: timeout_of(a),
            }
        }
        "wsl" => {
            if !cfg!(windows) {
                return Err(format!(
                    "WSL exists only on Windows; this computer runs {}. Use the shell tool.",
                    crate::config::os_name()
                ));
            }
            let command = text(a, "command");
            if command.trim().is_empty() {
                preview.summary = "wsl -l -v".into();
                preview.detail = "wsl -l -v".into();
                preview.lang = "bash".into();
                Action::WslList
            } else {
                let cwd = cwd_of(&mut preview);
                let mut args = Vec::new();
                let distro = text(a, "distro");
                if !distro.is_empty() {
                    args.push("-d".to_string());
                    args.push(distro);
                }
                args.push("--cd".into());
                args.push(cwd.to_string_lossy().to_string());
                args.extend(["-e".to_string(), "bash".into(), "-lc".into(), command.clone()]);
                preview.kind = Kind::Exec;
                preview.summary = first_line(&command, 120);
                preview.detail = command;
                preview.lang = "bash".into();
                Action::Run {
                    program: "wsl.exe".into(),
                    args,
                    raw: None,
                    cwd: ws.root().to_path_buf(),
                    timeout: timeout_of(a),
                }
            }
        }
        "read_file" => {
            let r = ws.resolve(&text(a, "path"));
            preview.outside = !r.inside;
            preview.summary = ws.display(&r.path);
            Action::Read {
                path: r.path,
                offset: number(a, "offset").unwrap_or(1) as usize,
                limit: number(a, "limit").unwrap_or(0) as usize,
            }
        }
        "list_dir" => {
            let r = ws.resolve(&text(a, "path"));
            preview.outside = !r.inside;
            preview.summary = ws.display(&r.path);
            Action::List { path: r.path }
        }
        "glob" | "grep" => {
            let r = ws.resolve(&text(a, "path"));
            preview.outside = !r.inside;
            let pattern = text(a, "pattern");
            if pattern.is_empty() {
                return Err("pattern is empty.".to_string());
            }
            preview.summary = format!("{} in {}", pattern, ws.display(&r.path));
            if call.tool == "glob" {
                Action::Glob { base: r.path, pattern }
            } else {
                Action::Grep {
                    base: r.path,
                    pattern,
                    glob: text(a, "glob"),
                    ignore_case: flag(a, "ignore_case"),
                }
            }
        }
        "write_file" | "edit_file" => {
            let raw = text(a, "path");
            if raw.trim().is_empty() {
                return Err("path is empty.".to_string());
            }
            let r = ws.resolve(&raw);
            if in_git_dir(&r.path) {
                return Err("Writing inside .git is not allowed; use git commands.".to_string());
            }
            let shown = ws.display(&r.path);
            let before = fs::old_content(&r.path);
            preview.kind = Kind::Write;
            preview.outside = !r.inside;
            preview.summary = shown.clone();
            preview.lang = "diff".into();
            if call.tool == "write_file" {
                let content = text(a, "content");
                preview.detail = diff(&shown, &before, &content);
                Action::Write { path: r.path, content }
            } else {
                if !r.path.is_file() {
                    return Err(format!("{shown} does not exist; use write_file to create it."));
                }
                let before = fs::read_text(&r.path)?;
                let (after, count) = fs::apply_edit(&before, &text(a, "old_text"), &text(a, "new_text"), flag(a, "replace_all"))?;
                preview.detail = diff(&shown, &before, &after);
                Action::Edit {
                    path: r.path,
                    before,
                    after,
                    count,
                }
            }
        }
        "save_memory" => {
            let title = text(a, "title");
            let description = text(a, "description");
            let content = text(a, "content");
            let slug = text(a, "slug");
            let draft = memory::draft(ws.root(), Some(slug.as_str()), &title, &description, &content)?;
            let shown = memory_inside(ws, &draft.slug)?;
            preview.kind = Kind::Write;
            preview.memory = true;
            preview.summary = shown.clone();
            preview.detail = diff(&shown, &draft.before, &draft.after);
            preview.lang = "diff".into();
            Action::SaveMemory {
                slug: draft.slug,
                title,
                description,
                content,
            }
        }
        "forget_memory" => {
            let slug = memory::check_slug(&text(a, "slug"))?;
            let shown = memory_inside(ws, &slug)?;
            let path = memory::path_of(ws.root(), &slug);
            if !path.is_file() {
                return Err(format!("{shown} does not exist."));
            }
            preview.kind = Kind::Write;
            preview.memory = true;
            preview.summary = format!("forget {shown}");
            preview.detail = fs::old_content(&path);
            preview.lang = "markdown".into();
            Action::ForgetMemory { slug }
        }
        other => {
            return Err(format!(
                "This version of Salam Agent does not know the tool {other}. Ask the user to update Salam Agent."
            ));
        }
    };
    Ok(Plan { preview, action })
}

fn wsl_list() -> Result<String, String> {
    let out = exec::command("wsl.exe")
        .args(["-l", "-v"])
        .output()
        .map_err(|e| format!("WSL is not available: {e}"))?;
    let bytes = out.stdout;
    let text = if bytes.len() > 1 && bytes.iter().skip(1).step_by(2).take(64).all(|&b| b == 0) {
        let units: Vec<u16> = bytes.chunks_exact(2).map(|c| u16::from_le_bytes([c[0], c[1]])).collect();
        String::from_utf16_lossy(&units)
    } else {
        String::from_utf8_lossy(&bytes).to_string()
    };
    let text = text.replace(['\u{feff}', '\0'], "");
    if out.status.success() {
        Ok(text)
    } else {
        Err(format!("{text}\n[WSL is not installed or not set up]"))
    }
}

pub async fn execute(action: Action, ws_root: PathBuf, sink: exec::Sink, cancel: Arc<Notify>) -> Result<String, String> {
    match action {
        Action::Run {
            program,
            args,
            raw,
            cwd,
            timeout,
        } => {
            let mut c = exec::command(&program);
            c.args(&args).current_dir(&cwd);
            #[cfg(windows)]
            if let Some(r) = raw {
                use std::os::windows::process::CommandExt;
                c.raw_arg(r);
            }
            #[cfg(not(windows))]
            let _ = raw;
            exec::run(c, Duration::from_secs(timeout), sink, cancel).await
        }
        other => {
            let task = tokio::task::spawn_blocking(move || {
                let ws = Workspace::open(&ws_root.to_string_lossy())?;
                match other {
                    Action::WslList => wsl_list(),
                    Action::Read { path, offset, limit } => fs::read_file(&path, offset, limit),
                    Action::Write { path, content } => fs::write_file(&path, &content),
                    Action::Edit {
                        path,
                        before,
                        after,
                        count,
                    } => {
                        if fs::read_text(&path)? != before {
                            return Err("The file changed while waiting for approval. Read it again.".to_string());
                        }
                        fs::write_file(&path, &after)?;
                        Ok(format!(
                            "Edited {} ({count} replacement{}).",
                            ws.display(&path),
                            if count == 1 { "" } else { "s" }
                        ))
                    }
                    Action::List { path } => fs::list_dir(&ws, &path),
                    Action::Glob { base, pattern } => search::glob(&ws, &base, &pattern),
                    Action::Grep {
                        base,
                        pattern,
                        glob,
                        ignore_case,
                    } => search::grep(&ws, &base, &pattern, &glob, ignore_case),
                    Action::SaveMemory {
                        slug,
                        title,
                        description,
                        content,
                    } => memory::save(ws.root(), Some(&slug), &title, &description, &content),
                    Action::ForgetMemory { slug } => memory::forget(ws.root(), &slug),
                    Action::Run { .. } => unreachable!(),
                }
            });
            match task.await {
                Ok(r) => r.map(|s| exec::cap_text(&s)),
                Err(e) => Err(format!("The tool crashed: {e}")),
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    fn ws() -> (tempfile::TempDir, Workspace) {
        let d = tempfile::tempdir().unwrap();
        std::fs::write(d.path().join("a.txt"), "hello world\n").unwrap();
        let w = Workspace::open(d.path().to_str().unwrap()).unwrap();
        (d, w)
    }

    fn call(tool: &str, args: Value) -> Call {
        Call {
            id: "c1".into(),
            tool: tool.into(),
            args,
            chat: "chat".into(),
            workspace: String::new(),
        }
    }

    #[test]
    fn writes_preview_a_diff_and_refuse_git() {
        let (_d, w) = ws();
        let p = plan(
            &call("edit_file", json!({"path": "a.txt", "old_text": "world", "new_text": "salam"})),
            &w,
        )
        .unwrap();
        assert_eq!(p.preview.kind, Kind::Write);
        assert!(p.preview.detail.contains("-hello world") && p.preview.detail.contains("+hello salam"));
        assert!(plan(&call("write_file", json!({"path": ".git/config", "content": ""})), &w).is_err());
        assert!(
            plan(
                &call("edit_file", json!({"path": "missing.txt", "old_text": "a", "new_text": "b"})),
                &w
            )
            .is_err()
        );
        assert!(
            plan(&call("write_file", json!({"path": "../x.txt", "content": "x"})), &w)
                .unwrap()
                .preview
                .outside
        );
    }

    #[test]
    fn reads_are_read_kind_and_unknown_tools_fail() {
        let (_d, w) = ws();
        assert_eq!(
            plan(&call("read_file", json!({"path": "a.txt"})), &w).unwrap().preview.kind,
            Kind::Read
        );
        assert!(plan(&call("format_disk", json!({})), &w).is_err());
        assert!(plan(&call("shell", json!({"command": "  "})), &w).is_err());
    }

    #[cfg(unix)]
    #[tokio::test]
    async fn edits_apply_and_detect_changes() {
        let (d, w) = ws();
        let p = plan(
            &call("edit_file", json!({"path": "a.txt", "old_text": "world", "new_text": "salam"})),
            &w,
        )
        .unwrap();
        std::fs::write(d.path().join("a.txt"), "changed\n").unwrap();
        let sink: exec::Sink = Arc::new(|_| {});
        let e = execute(p.action, w.root().to_path_buf(), sink.clone(), Arc::new(Notify::new()))
            .await
            .unwrap_err();
        assert!(e.contains("changed while waiting"));
        std::fs::write(d.path().join("a.txt"), "hello world\n").unwrap();
        let p = plan(
            &call("edit_file", json!({"path": "a.txt", "old_text": "world", "new_text": "salam"})),
            &w,
        )
        .unwrap();
        execute(p.action, w.root().to_path_buf(), sink, Arc::new(Notify::new()))
            .await
            .unwrap();
        assert_eq!(std::fs::read_to_string(d.path().join("a.txt")).unwrap(), "hello salam\n");
    }

    #[cfg(unix)]
    #[tokio::test]
    async fn shell_runs_in_the_workspace() {
        let (_d, w) = ws();
        let p = plan(&call("shell", json!({"command": "cat a.txt", "shell": "sh"})), &w).unwrap();
        assert_eq!(p.preview.kind, Kind::Exec);
        let out = execute(p.action, w.root().to_path_buf(), Arc::new(|_| {}), Arc::new(Notify::new()))
            .await
            .unwrap();
        assert!(out.contains("hello world"));
    }

    #[test]
    fn memory_tools_stay_in_the_workspace() {
        let (d, w) = ws();
        let save = |args: Value| plan(&call("save_memory", args), &w);
        let p = save(json!({"title": "Build steps", "description": "d", "content": "cargo build"})).unwrap();
        assert!(p.preview.memory);
        assert_eq!(p.preview.kind, Kind::Write);
        assert_eq!(p.preview.summary, ".salam-agent/memory/build-steps.md");
        assert!(p.preview.detail.contains("+title: Build steps") && p.preview.lang == "diff");
        assert!(matches!(&p.action, Action::SaveMemory { slug, .. } if slug == "build-steps"));
        assert!(save(json!({"title": "t", "content": "c", "slug": "../x"})).is_err());
        assert!(save(json!({"title": "t", "content": "c", "slug": "a/../../x"})).is_err());
        assert!(save(json!({"title": "", "content": "c"})).is_err());
        assert!(
            !plan(&call("write_file", json!({"path": "x", "content": ""})), &w)
                .unwrap()
                .preview
                .memory
        );
        assert!(plan(&call("forget_memory", json!({"slug": "../a"})), &w).is_err());
        assert!(plan(&call("forget_memory", json!({"slug": "missing"})), &w).is_err());
        memory::save(w.root(), Some("keep"), "Keep", "", "kept text").unwrap();
        let f = plan(&call("forget_memory", json!({"slug": "keep"})), &w).unwrap();
        assert!(f.preview.memory);
        assert_eq!(f.preview.summary, "forget .salam-agent/memory/keep.md");
        assert!(f.preview.detail.contains("kept text"));
        assert!(d.path().join(".salam-agent/memory/keep.md").is_file());
    }

    #[cfg(unix)]
    #[tokio::test]
    async fn memory_tools_save_and_forget() {
        let (d, w) = ws();
        let run = |p: Plan| execute(p.action, w.root().to_path_buf(), Arc::new(|_| {}), Arc::new(Notify::new()));
        let p = plan(&call("save_memory", json!({"title": "Notes", "content": "n"})), &w).unwrap();
        assert!(run(p).await.unwrap().starts_with("Saved .salam-agent/memory/notes.md (1 memory"));
        let p = plan(&call("forget_memory", json!({"slug": "notes"})), &w).unwrap();
        assert!(run(p).await.unwrap().starts_with("Forgot"));
        assert!(!d.path().join(".salam-agent/memory/notes.md").exists());
        assert!(
            std::fs::read_to_string(d.path().join(".salam-agent/index.md"))
                .unwrap()
                .contains("No memories yet.")
        );
    }

    #[test]
    fn wsl_is_windows_only() {
        let (_d, w) = ws();
        let r = plan(&call("wsl", json!({"command": "ls"})), &w);
        assert_eq!(r.is_err(), !cfg!(windows));
    }
}
