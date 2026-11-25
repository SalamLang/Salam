# ruff: noqa: UP030, UP032
# pylint: disable=import-error,too-few-public-methods,consider-using-f-string

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


STATE_FILE = "Salam.state.sublime-settings"
STATUS_KEY = "salam"
ALL_PLATFORMS = ["web", "js", "android", "ios", "linux", "macos", "windows", "all"]
INIT_PLATFORMS = ["web", "android", "ios", "linux", "macos", "windows"]
LABEL_LIMIT = 32


def project_root(path):
    if not path:
        return None
    folder = os.path.dirname(os.path.abspath(path))
    while True:
        if os.path.isfile(os.path.join(folder, "salam.toml")):
            return folder
        parent = os.path.dirname(folder)
        if parent == folder:
            return None
        folder = parent


def project_platforms(root):
    if not root:
        return []
    try:
        with open(os.path.join(root, "salam.toml"), "r", encoding="utf-8") as handle:
            lines = handle.read().splitlines()
    except OSError:
        return []
    in_project = False
    collecting = False
    text = ""
    for line in lines:
        if not collecting:
            stripped = line.strip()
            if stripped.startswith("["):
                in_project = stripped == "[project]"
                continue
            if not in_project or not stripped.startswith("platforms"):
                continue
            if "=" not in stripped:
                continue
            collecting = True
            line = line.split("=", 1)[1]
        text += line
        if "]" in text:
            break
    return re.findall(r'"([^"]*)"', text)


def platform_choices(root):
    listed = project_platforms(root)
    if not listed:
        return list(ALL_PLATFORMS)
    return listed + (["all"] if len(listed) > 1 else [])


def platform_key(root):
    return root or "*"


def chosen_platform(root):
    state = sublime.load_settings(STATE_FILE).get("platforms", {})
    if not isinstance(state, dict):
        return None
    return state.get(platform_key(root))


def remember_platform(root, platform):
    store = sublime.load_settings(STATE_FILE)
    state = store.get("platforms", {})
    if not isinstance(state, dict):
        state = {}
    if platform:
        state[platform_key(root)] = platform
    else:
        state.pop(platform_key(root), None)
    store.set("platforms", state)
    sublime.save_settings(STATE_FILE)


def indent_width(text, tab_size):
    width = 0
    for char in text:
        if char == " ":
            width += 1
        elif char == "\t":
            width += tab_size - (width % tab_size)
        else:
            break
    return width


BLANK_LINE = re.compile(r"^\s*(?:$|//|/\*|\*)")
END_LINE = re.compile(r"^\s*(?:end|\u067e\u0627\u06cc\u0627\u0646)\s*(?://.*)?$")
HEADER_LINE = re.compile(r"^\s*(.*?)\s*:\s*(?://.*)?$")


def header_label(text):
    match = HEADER_LINE.match(text)
    if not match:
        return None
    label = match.group(1).strip()
    cut = label.find("(")
    if cut > 0:
        label = label[:cut].rstrip()
    if len(label) > LABEL_LIMIT:
        label = label[: LABEL_LIMIT - 1] + "..."
    return label or None


def blocks_at(lines, row, tab_size=4):
    """Return [(row, label)] of the blocks around lines[row], outermost first."""
    if not lines:
        return []
    row = min(row, len(lines) - 1)
    from_blank = False
    if BLANK_LINE.match(lines[row]):
        from_blank = True
        while row >= 0 and BLANK_LINE.match(lines[row]):
            row -= 1
        if row < 0:
            return []
    chain = []
    text = lines[row]
    here = indent_width(text, tab_size)
    if END_LINE.match(text):
        limit = here if from_blank else here + 1
    else:
        limit = here + 1
        label = header_label(text)
        if label:
            chain.append((row, label))
            limit = here
    index = row - 1
    while index >= 0 and limit > 0:
        line = lines[index]
        if not BLANK_LINE.match(line):
            width = indent_width(line, tab_size)
            if width < limit:
                label = header_label(line)
                if label:
                    chain.append((index, label))
                    limit = width
        index -= 1
    chain.reverse()
    return chain


class ViewLines:
    def __init__(self, view):
        self.view = view
        self.count = view.rowcol(view.size())[0] + 1

    def __len__(self):
        return self.count

    def __getitem__(self, row):
        point = self.view.text_point(row, 0)
        return self.view.substr(self.view.line(point))


def view_blocks(view):
    selection = view.sel()
    if not selection:
        return []
    row = view.rowcol(selection[0].b)[0]
    tab_size = view.settings().get("tab_size", 4)
    return blocks_at(ViewLines(view), row, tab_size)


def block_chain(view, separator=" > "):
    return separator.join(label for _row, label in view_blocks(view))


def update_status(view):
    if view is None or not is_salam(view):
        return
    parts = []
    if setting("status_block_chain", True):
        chain = block_chain(view)
        if chain:
            parts.append(chain)
    if setting("status_platform", True):
        platform = chosen_platform(project_root(view.file_name()))
        if platform:
            parts.append("platform: " + platform)
    if parts:
        view.set_status(STATUS_KEY, "Salam: " + "  |  ".join(parts))
    else:
        view.erase_status(STATUS_KEY)


class SalamStatus(sublime_plugin.EventListener):
    def on_activated_async(self, view):
        update_status(view)

    def on_load_async(self, view):
        update_status(view)

    def on_modified_async(self, view):
        update_status(view)

    def on_selection_modified_async(self, view):
        update_status(view)

    def on_post_save_async(self, view):
        update_status(view)


def refresh_all_statuses():
    for window in sublime.windows():
        for view in window.views():
            update_status(view)


class SalamGotoEnclosingBlockCommand(sublime_plugin.TextCommand):
    def is_enabled(self):
        return is_salam(self.view)

    def run(self, _edit, count=1):
        selection = self.view.sel()
        if not selection:
            return
        here = self.view.rowcol(selection[0].b)[0]
        blocks = [b for b in view_blocks(self.view) if b[0] != here]
        if not blocks:
            sublime.status_message("Salam: not inside a block")
            return
        target = blocks[max(len(blocks) - max(int(count), 1), 0)]
        self.jump(target[0])

    def jump(self, row):
        point = self.view.text_point(row, 0)
        line = self.view.substr(self.view.line(point))
        point += len(line) - len(line.lstrip())
        self.view.sel().clear()
        self.view.sel().add(sublime.Region(point))
        self.view.show_at_center(point)


class SalamPickBlockCommand(sublime_plugin.TextCommand):
    def is_enabled(self):
        return is_salam(self.view)

    def run(self, _edit):
        blocks = view_blocks(self.view)
        if not blocks:
            sublime.status_message("Salam: not inside a block")
            return
        items = ["{0}  (line {1})".format(label, row + 1) for row, label in blocks]

        def picked(index):
            if index >= 0:
                SalamGotoEnclosingBlockCommand.jump(self, blocks[index][0])

        self.view.window().show_quick_panel(items, picked)


def active_salam_view(window):
    view = window.active_view()
    return view if is_salam(view) else None


def exec_compiler(window, args, working_dir):
    command = [setting("compiler_path", "salam")] + args
    window.run_command(
        "exec",
        {
            "cmd": command,
            "working_dir": working_dir,
            "file_regex": GCC_DIAGNOSTIC,
            "quiet": True,
        },
    )


def invocation(window, verb):
    view = active_salam_view(window)
    path = view.file_name() if view else None
    root = project_root(path)
    platform = chosen_platform(root)
    tail = ["--error-style=gcc"] + common_args()
    flag = []
    if platform and not (verb == "run" and platform in ("all", "js")):
        flag = ["--platform=" + platform]
    if root:
        return [verb] + flag + tail, root
    if not path:
        return None, None
    folder = os.path.dirname(path)
    if verb == "run" and not flag:
        return [setting("run_command", "exec"), path] + tail, folder
    return [verb, path] + flag + tail, folder


class SalamRunCommand(sublime_plugin.WindowCommand):
    verb = "run"

    def is_enabled(self, **_kwargs):
        return active_salam_view(self.window) is not None

    def run(self, **_kwargs):
        view = active_salam_view(self.window)
        if view is not None and view.is_dirty() and view.file_name():
            view.run_command("save")
        args, folder = invocation(self.window, self.verb)
        if args is None:
            sublime.status_message("Salam: save the file first")
            return
        exec_compiler(self.window, args, folder)


class SalamBuildCommand(SalamRunCommand):
    verb = "build"


class SalamSelectPlatformCommand(sublime_plugin.WindowCommand):
    def run(self):
        view = self.window.active_view()
        root = project_root(view.file_name() if view else None)
        choices = ["auto"] + platform_choices(root)
        current = chosen_platform(root) or "auto"
        items = [
            [name, "current" if name == current else
             ("follow salam.toml" if name == "auto" else "")]
            for name in choices
        ]

        def picked(index):
            if index < 0:
                return
            name = choices[index]
            remember_platform(root, None if name == "auto" else name)
            sublime.status_message("Salam: platform " + name)
            refresh_all_statuses()

        selected = choices.index(current) if current in choices else 0
        self.window.show_quick_panel(items, picked, selected_index=selected)


class SalamNewProjectCommand(sublime_plugin.WindowCommand):
    def run(self):
        folders = self.window.folders()
        start = folders[0] if folders else os.path.expanduser("~")
        self.window.show_input_panel(
            "New Salam project in:", os.path.join(start, "app"), self.got_dir, None, None
        )

    def got_dir(self, text):
        target = os.path.abspath(os.path.expanduser(text.strip()))
        if not text.strip():
            return
        self.answers = {"dir": target, "name": os.path.basename(target)}
        self.choose("type", ["cli", "http", "dsl"], "Project type", self.got_type)

    def choose(self, key, options, title, then):
        def picked(index):
            if index >= 0:
                self.answers[key] = options[index]
                then()

        self.window.show_quick_panel(options, picked, placeholder=title)

    def got_type(self):
        default = "web,linux" if self.answers["type"] == "dsl" else "linux"
        self.window.show_input_panel(
            "Platforms (" + ", ".join(INIT_PLATFORMS) + ", or all):",
            default, self.got_platforms, None, None,
        )

    def got_platforms(self, text):
        names = [n.strip() for n in text.split(",") if n.strip()]
        if not names:
            return
        self.answers["platforms"] = ",".join(names)
        if self.answers["type"] == "dsl":
            self.choose("design", ["shadcn", "none"], "Design system", self.got_design)
        else:
            self.finish()

    def got_design(self):
        if "web" in self.answers["platforms"].split(",") or self.answers["platforms"] == "all":
            self.choose("web", ["page", "spa", "ssr"], "Web mode", self.finish)
        else:
            self.finish()

    def finish(self):
        answers = self.answers
        args = [
            "init", answers["dir"],
            "--name=" + answers["name"],
            "--type=" + answers["type"],
            "--platforms=" + answers["platforms"],
            "-y",
        ]
        if "design" in answers:
            args.append("--design=" + answers["design"])
        if "web" in answers:
            args.append("--web=" + answers["web"])
        window = self.window
        sublime.status_message("Salam: creating " + answers["name"] + "...")

        def work():
            parent = os.path.dirname(answers["dir"])
            os.makedirs(parent, exist_ok=True)
            code, out, err = run_compiler(args + lang_args(), cwd=parent)
            sublime.set_timeout(lambda: created(code, (out + err).strip()), 0)

        def created(code, report):
            if code != 0:
                show_panel(window, (report or "salam init failed") + "\n")
                return
            data = window.project_data() or {}
            folders = data.setdefault("folders", [])
            if not any(f.get("path") == answers["dir"] for f in folders):
                folders.append({"path": answers["dir"]})
            window.set_project_data(data)
            sublime.status_message("Salam: created " + answers["name"])

        sublime.set_timeout_async(work, 0)
