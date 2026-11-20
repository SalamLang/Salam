# ruff: noqa: UP030, UP032
# pylint: disable=import-error,too-few-public-methods,consider-using-f-string

import os
import subprocess
import tempfile

import sublime
import sublime_plugin

SETTINGS_FILE = "Salam.sublime-settings"
SYNTAX_SCOPE = "source.salam"
OUTPUT_PANEL = "salam"

GCC_DIAGNOSTIC = r"^(.+?):([0-9]+):([0-9]+): (?:error|warning): (.*)$"


def settings():
    return sublime.load_settings(SETTINGS_FILE)


def setting(key, fallback):
    value = settings().get(key, fallback)
    return fallback if value is None else value


def is_salam(view):
    return view is not None and view.match_selector(0, SYNTAX_SCOPE)


def startup_info():
    if os.name != "nt":
        return None
    info = subprocess.STARTUPINFO()
    info.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    info.wShowWindow = subprocess.SW_HIDE
    return info


def run_compiler(args, cwd=None):
    command = [setting("compiler_path", "salam")] + args
    seconds = setting("timeout_seconds", 60)
    try:
        with subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=cwd,
            startupinfo=startup_info(),
            universal_newlines=True,
        ) as proc:
            try:
                out, err = proc.communicate(timeout=seconds)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.communicate()
                return (
                    None,
                    "",
                    "{0} did not finish within {1}s".format(command[0], seconds),
                )
            return proc.returncode, out or "", err or ""
    except OSError as exc:
        return None, "", "cannot run {0!r}: {1}".format(command[0], exc)


def lang_args():
    choice = setting("language_pack", "auto")
    if choice in ("en", "fa"):
        return ["--lang=" + choice]
    return []


def common_args():
    args = lang_args()
    stdlib = setting("stdlib_path", "")
    if stdlib:
        args.append("--stdlib-path=" + stdlib)
    return args


def switch_lint_args():
    mode = setting("switch_lint", "off")
    if mode in ("warn", "error", "fix"):
        args = ["--switch-lint=" + mode]
        minimum = setting("switch_lint_min", 4)
        try:
            args.append("--switch-lint-min=" + str(int(minimum)))
        except (TypeError, ValueError):
            args.append("--switch-lint-min=4")
        return args
    return []


def format_source(text):
    workdir = tempfile.mkdtemp(prefix="salam-fmt-")
    path = os.path.join(workdir, "document.salam")
    try:
        with open(path, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
        args = ["format", path, "--indent=" + str(setting("format_indent", "4"))]
        code, out, err = run_compiler(args + common_args(), cwd=workdir)
        if code != 0:
            return None, (err or out).strip() or "salam format exited {0}".format(code)
        with open(path, "r", encoding="utf-8") as handle:
            return handle.read(), None
    except OSError as exc:
        return None, str(exc)
    finally:
        try:
            if os.path.exists(path):
                os.remove(path)
            os.rmdir(workdir)
        except OSError:
            pass


def show_panel(window, text):
    panel = window.create_output_panel(OUTPUT_PANEL)
    panel.settings().set("result_file_regex", GCC_DIAGNOSTIC)
    panel.settings().set("word_wrap", True)
    panel.set_read_only(False)
    panel.run_command("append", {"characters": text})
    panel.set_read_only(True)
    window.run_command("show_panel", {"panel": "output." + OUTPUT_PANEL})


class SalamFormatCommand(sublime_plugin.TextCommand):
    def is_enabled(self):
        return is_salam(self.view)

    def run(self, edit):
        whole = sublime.Region(0, self.view.size())
        original = self.view.substr(whole)
        formatted, error = format_source(original)
        if error is not None:
            sublime.status_message("Salam: formatting failed")
            show_panel(self.view.window(), error + "\n")
            return
        if formatted is None or formatted == original:
            sublime.status_message("Salam: already formatted")
            return

        viewport = self.view.viewport_position()
        selections = [(r.a, r.b) for r in self.view.sel()]
        self.view.replace(edit, whole, formatted)
        size = self.view.size()
        self.view.sel().clear()
        for start, end in selections:
            self.view.sel().add(sublime.Region(min(start, size), min(end, size)))
        self.view.set_viewport_position(viewport, False)
        sublime.status_message("Salam: formatted")


class SalamCheckCommand(sublime_plugin.TextCommand):
    def is_enabled(self):
        return is_salam(self.view)

    def run(self, _edit):
        path = self.view.file_name()
        if not path:
            sublime.status_message("Salam: save the file first")
            return
        if self.view.is_dirty():
            self.view.run_command("save")

        null_device = "NUL" if os.name == "nt" else "/dev/null"
        args = [
            "inspect",
            path,
            "--emit-symbol",
            "--xml-out=" + null_device,
            "--error-style=gcc",
            "--log-level=error",
        ] + common_args() + switch_lint_args()
        window = self.view.window()
        working_dir = os.path.dirname(path)

        sublime.status_message("Salam: checking...")
        sublime.set_timeout_async(
            lambda: check_in_background(window, args, working_dir), 0
        )


def check_in_background(window, args, working_dir):
    code, out, err = run_compiler(args, cwd=working_dir)
    report = (out + err).strip()
    sublime.set_timeout(lambda: report_check(window, code, report), 0)


def report_check(window, code, report):
    if code == 0 and not report:
        sublime.status_message("Salam: no problems found")
        window.run_command("hide_panel", {"panel": "output." + OUTPUT_PANEL})
        return
    show_panel(window, report + "\n")


class SalamToggleFormatOnSaveCommand(sublime_plugin.ApplicationCommand):
    def run(self):
        config = settings()
        now = not config.get("format_on_save", False)
        config.set("format_on_save", now)
        sublime.save_settings(SETTINGS_FILE)
        sublime.status_message(
            "Salam: format on save {0}".format("on" if now else "off")
        )


class SalamFormatOnSave(sublime_plugin.EventListener):
    def on_pre_save(self, view):
        if is_salam(view) and setting("format_on_save", False):
            view.run_command("salam_format")
