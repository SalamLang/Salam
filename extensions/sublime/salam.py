# Sublime Text runs package code in a Python 3.3 host unless the package opts
# into 3.8, so this file stays on str.format and cannot use f-strings.
# ruff: noqa: UP030, UP032
# pylint: disable=import-error,too-few-public-methods,consider-using-f-string
"""Salam support for Sublime Text.

Syntax highlighting, indentation and completions come from the data files
next to this one; this module adds the parts that have to shell out to the
compiler: formatting a buffer and checking it without running it.

Sublime Text 3 and 4 are both supported, so no f-strings.
"""

import os
import re
import subprocess
import tempfile

import sublime
import sublime_plugin

SETTINGS_FILE = "Salam.sublime-settings"
SYNTAX_SCOPE = "source.salam"
OUTPUT_PANEL = "salam"

GCC_DIAGNOSTIC = r"^(.+?):([0-9]+):([0-9]+): (?:error|warning): (.*)$"

PERSIAN_ONLY = "پچژگکی"
ARABIC_RANGES = (
    (0x0600, 0x06FF),
    (0x0750, 0x077F),
    (0x08A0, 0x08FF),
    (0xFB50, 0xFDFF),
    (0xFE70, 0xFEFF),
)


def settings():
    return sublime.load_settings(SETTINGS_FILE)


def setting(key, fallback):
    value = settings().get(key, fallback)
    return fallback if value is None else value


def is_salam(view):
    return view is not None and view.match_selector(0, SYNTAX_SCOPE)


def startup_info():
    """Keep Windows from flashing a console window for each compiler run."""
    if os.name != "nt":
        return None
    info = subprocess.STARTUPINFO()
    info.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    info.wShowWindow = subprocess.SW_HIDE
    return info


def run_compiler(args, cwd=None):
    """Run the compiler and return (returncode, stdout, stderr).

    Returns (None, "", message) when the executable cannot be started, which
    is almost always a compiler_path that is not on PATH, or when it outlived
    its timeout. Formatting runs on the UI thread because the edit token only
    exists there, so an unbounded wait would freeze the editor, and freeze a
    save along with it.
    """
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


def is_arabic_script(codepoint):
    for low, high in ARABIC_RANGES:
        if low <= codepoint <= high:
            return True
    return False


def directive_lang(head):
    """The two-letter code from a '//! lang: xx' header, or ''."""
    match = re.search(r"(?:lang|LANG|زبان)\s*:\s*([A-Za-z]{2})", head)
    return match.group(1).lower() if match else ""


def detect_lang(text):
    """Which keyword pack a buffer is written in: 'en', 'fa' or 'ar'."""
    choice = setting("language_pack", "auto")
    if choice in ("en", "fa", "ar"):
        return choice

    head = text[:4000]
    from_directive = directive_lang(head)
    if from_directive:
        return from_directive

    arabic = 0
    for char in head:
        if char in PERSIAN_ONLY:
            return "fa"
        if is_arabic_script(ord(char)):
            arabic += 1
    return "ar" if arabic >= 3 else "en"


def common_args(text):
    args = ["--lang=" + detect_lang(text)]
    stdlib = setting("stdlib_path", "")
    if stdlib:
        args.append("--stdlib-path=" + stdlib)
    return args


def format_source(text):
    """Return (formatted_text, error). `salam format` rewrites files in
    place, so the buffer goes through a scratch file."""
    workdir = tempfile.mkdtemp(prefix="salam-fmt-")
    path = os.path.join(workdir, "document.salam")
    try:
        with open(path, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
        args = ["format", path, "--indent=" + str(setting("format_indent", "4"))]
        code, out, err = run_compiler(args + common_args(text), cwd=workdir)
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
    """Reformat the whole buffer with `salam format`."""

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
    """Type-check the file without running it, into the output panel.

    Nothing here needs the edit token, so the compiler runs off the UI thread
    and only the panel update comes back to it.
    """

    def is_enabled(self):
        return is_salam(self.view)

    def run(self, _edit):
        path = self.view.file_name()
        if not path:
            sublime.status_message("Salam: save the file first")
            return
        if self.view.is_dirty():
            self.view.run_command("save")

        # Read the buffer and the settings here, where the UI thread owns them.
        text = self.view.substr(sublime.Region(0, self.view.size()))
        null_device = "NUL" if os.name == "nt" else "/dev/null"
        args = [
            "inspect",
            path,
            "--emit-symbol-xml",
            "--xml-out=" + null_device,
            "--error-style=gcc",
            "--log-level=error",
        ] + common_args(text)
        window = self.view.window()
        working_dir = os.path.dirname(path)

        sublime.status_message("Salam: checking...")
        sublime.set_timeout_async(
            lambda: check_in_background(window, args, working_dir), 0
        )


def check_in_background(window, args, working_dir):
    """Runs off the UI thread; touches no buffer."""
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
    """Turn the format-on-save setting on or off."""

    def run(self):
        config = settings()
        now = not config.get("format_on_save", False)
        config.set("format_on_save", now)
        sublime.save_settings(SETTINGS_FILE)
        sublime.status_message(
            "Salam: format on save {0}".format("on" if now else "off")
        )


class SalamFormatOnSave(sublime_plugin.EventListener):
    """Reformat a Salam buffer before it is written, when enabled."""

    def on_pre_save(self, view):
        if is_salam(view) and setting("format_on_save", False):
            view.run_command("salam_format")
