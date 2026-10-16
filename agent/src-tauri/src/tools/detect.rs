use std::path::Path;

fn has(dir: &Path, name: &str) -> bool {
    dir.join(name).exists()
}

fn package_has_test(dir: &Path) -> bool {
    std::fs::read_to_string(dir.join("package.json"))
        .ok()
        .and_then(|t| serde_json::from_str::<serde_json::Value>(&t).ok())
        .map(|v| v["scripts"]["test"].is_string())
        .unwrap_or(false)
}

fn makefile_has_test(dir: &Path) -> bool {
    ["Makefile", "makefile", "GNUmakefile"]
        .iter()
        .filter_map(|n| std::fs::read_to_string(dir.join(n)).ok())
        .any(|t| t.lines().any(|l| l.starts_with("test:")))
}

pub fn test_command(dir: &Path) -> Option<String> {
    if has(dir, "Cargo.toml") {
        return Some("cargo test".to_string());
    }
    if package_has_test(dir) {
        let runner = if has(dir, "bun.lock") || has(dir, "bun.lockb") {
            "bun run test"
        } else if has(dir, "pnpm-lock.yaml") {
            "pnpm test"
        } else if has(dir, "yarn.lock") {
            "yarn test"
        } else {
            "npm test"
        };
        return Some(runner.to_string());
    }
    if has(dir, "go.mod") {
        return Some("go test ./...".to_string());
    }
    if has(dir, "pytest.ini") || has(dir, "pyproject.toml") || has(dir, "setup.py") || has(dir, "tox.ini") {
        return Some("python -m pytest -q".to_string());
    }
    if makefile_has_test(dir) {
        return Some("make test".to_string());
    }
    None
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn detects_common_runners() {
        let d = tempfile::tempdir().unwrap();
        assert_eq!(test_command(d.path()), None);
        std::fs::write(d.path().join("Makefile"), "build:\n\ttrue\ntest:\n\ttrue\n").unwrap();
        assert_eq!(test_command(d.path()).unwrap(), "make test");
        std::fs::write(d.path().join("package.json"), r#"{"scripts":{"test":"vitest"}}"#).unwrap();
        std::fs::write(d.path().join("bun.lock"), "").unwrap();
        assert_eq!(test_command(d.path()).unwrap(), "bun run test");
        std::fs::write(d.path().join("Cargo.toml"), "").unwrap();
        assert_eq!(test_command(d.path()).unwrap(), "cargo test");
    }
}
