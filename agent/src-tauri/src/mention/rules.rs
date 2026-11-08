use std::path::Path;

pub enum When {
    Always,
    SiblingExt(&'static [&'static str]),
    SiblingFile(&'static str),
    InsideFile(&'static str),
}

pub struct DirRule {
    pub name: &'static str,
    pub when: When,
}

const fn always(name: &'static str) -> DirRule {
    DirRule { name, when: When::Always }
}

const DOTNET: &[&str] = &["csproj", "fsproj", "vbproj", "sln"];

pub const DIR_RULES: &[DirRule] = &[
    always("vendor"),
    always("node_modules"),
    always("bower_components"),
    always("jspm_packages"),
    always(".git"),
    always(".hg"),
    always(".svn"),
    always("target"),
    always("build"),
    always("dist"),
    always("out"),
    DirRule {
        name: "bin",
        when: When::SiblingExt(DOTNET),
    },
    DirRule {
        name: "obj",
        when: When::SiblingExt(DOTNET),
    },
    always(".gradle"),
    always(".idea"),
    always(".vscode-test"),
    always(".next"),
    always(".nuxt"),
    always(".svelte-kit"),
    always(".turbo"),
    always(".parcel-cache"),
    always(".cache"),
    always(".angular"),
    always("coverage"),
    always(".nyc_output"),
    always("__pycache__"),
    always(".venv"),
    always("venv"),
    DirRule {
        name: "env",
        when: When::InsideFile("pyvenv.cfg"),
    },
    always(".tox"),
    always(".mypy_cache"),
    always(".pytest_cache"),
    always(".ruff_cache"),
    always(".eggs"),
    always("site-packages"),
    always(".terraform"),
    always(".serverless"),
    always("Pods"),
    always("Carthage"),
    always("DerivedData"),
    always(".dart_tool"),
    always(".pub-cache"),
    always(".stack-work"),
    always("dist-newstyle"),
    always("_build"),
    DirRule {
        name: "deps",
        when: When::SiblingFile("mix.exs"),
    },
    always(".elixir_ls"),
    always(".bundle"),
    always(".sass-cache"),
];

pub const DIR_SUFFIXES: &[&str] = &[".egg-info"];

pub const DIR_PATHS: &[&str] = &["storage/framework", "bootstrap/cache", "var/cache", "var/log", "wp-content/cache"];

pub const FILE_NAMES: &[&str] = &[".DS_Store", "Thumbs.db"];

pub const FILE_EXTS: &[&str] = &[
    "zip", "tar", "gz", "tgz", "bz2", "xz", "7z", "rar", "iso", "dmg", "exe", "dll", "so", "dylib", "class", "jar", "war", "pyc", "o", "a",
];

fn has_ext(dir: &Path, exts: &[&str]) -> bool {
    let Ok(read) = std::fs::read_dir(dir) else { return false };
    read.flatten().any(|e| {
        let name = e.file_name();
        let name = name.to_string_lossy();
        name.rsplit_once('.')
            .is_some_and(|(_, ext)| exts.iter().any(|x| ext.eq_ignore_ascii_case(x)))
    })
}

pub fn skip_dir(rel: &str, name: &str, abs: &Path) -> bool {
    for rule in DIR_RULES {
        if !rule.name.eq_ignore_ascii_case(name) {
            continue;
        }
        return match &rule.when {
            When::Always => true,
            When::SiblingExt(exts) => abs.parent().is_some_and(|p| has_ext(p, exts)),
            When::SiblingFile(file) => abs.parent().is_some_and(|p| p.join(file).is_file()),
            When::InsideFile(file) => abs.join(file).is_file(),
        };
    }
    let lower = name.to_ascii_lowercase();
    if DIR_SUFFIXES.iter().any(|s| lower.ends_with(s)) {
        return true;
    }
    DIR_PATHS.iter().any(|p| {
        rel.len() >= p.len()
            && rel.is_char_boundary(rel.len() - p.len())
            && rel[rel.len() - p.len()..].eq_ignore_ascii_case(p)
            && (rel.len() == p.len() || rel.as_bytes()[rel.len() - p.len() - 1] == b'/')
    })
}

pub fn skip_file(name: &str) -> bool {
    if FILE_NAMES.iter().any(|n| n.eq_ignore_ascii_case(name)) {
        return true;
    }
    match name.rsplit_once('.') {
        Some((stem, ext)) if !stem.is_empty() => FILE_EXTS.iter().any(|x| ext.eq_ignore_ascii_case(x)),
        _ => false,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    fn dir(rel: &str, abs: &Path) -> bool {
        let name = rel.rsplit('/').next().unwrap();
        skip_dir(rel, name, abs)
    }

    #[test]
    fn ecosystem_dirs_are_skipped_at_any_depth() {
        let tmp = tempfile::tempdir().unwrap();
        for rel in [
            "vendor",
            "packages/app/vendor",
            "node_modules",
            "apps/web/node_modules",
            "a/b/.git",
            "crates/x/target",
            "web/.next",
            "py/__pycache__",
            "py/.venv",
            "pkg/foo.egg-info",
            "app/storage/framework",
            "laravel/bootstrap/cache",
            "sym/var/cache",
            "sym/var/log",
            "site/wp-content/cache",
            "ios/Pods",
            "ios/DerivedData",
            "x/.dart_tool",
            "x/dist-newstyle",
            "x/_build",
            "x/coverage",
            "x/dist",
        ] {
            assert!(dir(rel, &tmp.path().join(rel)), "{rel} should be skipped");
        }
    }

    #[test]
    fn normal_dirs_are_kept() {
        let tmp = tempfile::tempdir().unwrap();
        for rel in [
            "src",
            "src/vendor-utils",
            "vendors",
            "my_vendor",
            "bin",
            "obj",
            "env",
            "deps",
            "lib/cache",
            "storage/app",
            "docs",
        ] {
            assert!(!dir(rel, &tmp.path().join(rel)), "{rel} should be kept");
        }
    }

    #[test]
    fn conditional_dirs_need_their_marker() {
        let tmp = tempfile::tempdir().unwrap();
        let p = tmp.path();
        fs::create_dir_all(p.join("svc/bin")).unwrap();
        assert!(!dir("svc/bin", &p.join("svc/bin")));
        fs::write(p.join("svc/App.csproj"), "").unwrap();
        assert!(dir("svc/bin", &p.join("svc/bin")));
        assert!(dir("svc/obj", &p.join("svc/obj")));
        fs::create_dir_all(p.join("py/env")).unwrap();
        assert!(!dir("py/env", &p.join("py/env")));
        fs::write(p.join("py/env/pyvenv.cfg"), "").unwrap();
        assert!(dir("py/env", &p.join("py/env")));
        fs::create_dir_all(p.join("ex/deps")).unwrap();
        assert!(!dir("ex/deps", &p.join("ex/deps")));
        fs::write(p.join("ex/mix.exs"), "").unwrap();
        assert!(dir("ex/deps", &p.join("ex/deps")));
    }

    #[test]
    fn junk_files_are_skipped_and_source_is_kept() {
        for name in [
            ".DS_Store",
            "Thumbs.db",
            "a.zip",
            "x.tar",
            "lib.so",
            "App.dll",
            "Main.class",
            "m.pyc",
            "x.o",
            "l.a",
            "setup.EXE",
            "x.jar",
        ] {
            assert!(skip_file(name), "{name}");
        }
        for name in [
            "main.rs",
            "Cargo.lock",
            "package-lock.json",
            "a.zipper",
            "zip",
            "notes.md",
            ".gitignore",
            "فایل.txt",
        ] {
            assert!(!skip_file(name), "{name}");
        }
    }
}
