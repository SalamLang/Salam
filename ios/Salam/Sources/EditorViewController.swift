import UIKit

final class EditorViewController: UIViewController, UITextViewDelegate {
    private let file: URL
    private var editor: CodeTextView!
    private let console = ConsoleView()
    private var symbols: SymbolBar!
    private let problems = UIButton(type: .system)
    private let findBar = UIStackView()
    private let findField = UITextField()
    private let replaceField = UITextField()
    private var consoleHeight: NSLayoutConstraint!
    private var preferredConsoleHeight: CGFloat = 260
    private var lang = "en"
    private var dirty = false
    private var diagIndex = -1
    private var lastInserted = ""
    private var jobs = [Int: (Int32) -> Void]()
    private var tokensWork: DispatchWorkItem?
    private var checkWork: DispatchWorkItem?
    private var saveWork: DispatchWorkItem?
    private var runItem: UIBarButtonItem!

    init(file: URL) {
        self.file = file
        super.init(nibName: nil, bundle: nil)
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        view.backgroundColor = Palette.background
        navigationItem.largeTitleDisplayMode = .never
        editor = CodeTextView(fontSize: Prefs.fontSize)
        editor.delegate = self
        editor.tabWidth = Prefs.tabWidth
        editor.wrapLines = Prefs.wrap
        editor.showLineNumbers = Prefs.lineNumbers
        editor.onZoom = { Prefs.fontSize = $0 }
        symbols = SymbolBar(editor: editor)
        editor.inputAccessoryView = symbols

        runItem = UIBarButtonItem(image: UIImage(systemName: "play.fill"), style: .plain, target: self, action: #selector(toggleRun))
        runItem.tintColor = Palette.brand
        runItem.accessibilityLabel = L("run")
        let more = UIBarButtonItem(title: nil, image: UIImage(systemName: "ellipsis.circle"), primaryAction: nil, menu: buildMenu())
        more.accessibilityLabel = L("more")
        navigationItem.rightBarButtonItems = [more, runItem]

        buildFindBar()
        problems.contentHorizontalAlignment = .leading
        problems.titleLabel?.font = .systemFont(ofSize: 13)
        problems.titleLabel?.lineBreakMode = .byTruncatingTail
        problems.backgroundColor = Palette.surface
        problems.contentEdgeInsets = UIEdgeInsets(top: 6, left: 12, bottom: 6, right: 12)
        problems.addAction(UIAction { [weak self] _ in self?.nextProblem() }, for: .touchUpInside)

        console.isHidden = true
        console.onInput = { Engine.shared.input($0) }
        console.onStop = { Engine.shared.stop() }
        console.onClose = { [weak self] in
            if Engine.shared.isRunning {
                Engine.shared.stop()
            }
            self?.console.isHidden = true
        }
        console.onResize = { [weak self] dy in
            guard let self = self else { return }
            let limit = max(120, self.view.bounds.height * 0.8)
            self.preferredConsoleHeight = min(limit, max(90, self.preferredConsoleHeight + dy))
            self.consoleHeight.constant = self.preferredConsoleHeight
        }

        let stack = UIStackView(arrangedSubviews: [findBar, editor, problems, console])
        stack.axis = .vertical
        view.addSubview(stack)
        stack.translatesAutoresizingMaskIntoConstraints = false
        consoleHeight = console.heightAnchor.constraint(equalToConstant: preferredConsoleHeight)
        consoleHeight.priority = .defaultHigh
        NSLayoutConstraint.activate([
            stack.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            stack.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            stack.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            stack.bottomAnchor.constraint(equalTo: view.keyboardLayoutGuide.topAnchor),
            consoleHeight,
            console.heightAnchor.constraint(lessThanOrEqualTo: stack.heightAnchor, multiplier: 0.5),
        ])

        let text = (try? String(contentsOf: file, encoding: .utf8)) ?? ""
        editor.load(text)
        updateTitle()
        Engine.shared.detect(text) { [weak self] detected in
            guard let self = self else { return }
            self.setLang(detected == "fa" ? "fa" : "en")
            self.requestTokens()
            if Prefs.liveCheck {
                self.requestCheck()
            }
        }
        NotificationCenter.default.addObserver(self, selector: #selector(saveNow), name: UIApplication.willResignActiveNotification, object: nil)
    }

    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        saveNow()
    }

    private func setLang(_ code: String) {
        lang = code
        editor.lang = code
        symbols.setLang(code)
        editor.textAlignment = .natural
        updateTitle()
    }

    private func updateTitle() {
        let label = UILabel()
        label.numberOfLines = 2
        label.textAlignment = .center
        let title = NSMutableAttributedString(
            string: (dirty ? "• " : "") + file.lastPathComponent,
            attributes: [.font: UIFont.boldSystemFont(ofSize: 16), .foregroundColor: Palette.text]
        )
        title.append(NSAttributedString(
            string: "\n" + (lang == "fa" ? L("lang_fa") : L("lang_en")),
            attributes: [.font: UIFont.systemFont(ofSize: 11), .foregroundColor: Palette.textDim]
        ))
        label.attributedText = title
        navigationItem.titleView = label
    }

    private func buildFindBar() {
        for (field, hint) in [(findField, "find_hint"), (replaceField, "replace_hint")] {
            field.placeholder = L(hint)
            field.borderStyle = .roundedRect
            field.autocorrectionType = .no
            field.autocapitalizationType = .none
            field.font = .systemFont(ofSize: 14)
        }
        func button(_ symbol: String, _ label: String, _ action: @escaping () -> Void) -> UIButton {
            let b = UIButton(type: .system)
            b.setImage(UIImage(systemName: symbol), for: .normal)
            b.accessibilityLabel = L(label)
            b.addAction(UIAction { _ in action() }, for: .touchUpInside)
            b.widthAnchor.constraint(equalToConstant: 36).isActive = true
            return b
        }
        let row1 = UIStackView(arrangedSubviews: [
            findField,
            button("chevron.up", "previous") { [weak self] in self?.find(forward: false) },
            button("chevron.down", "next") { [weak self] in self?.find(forward: true) },
            button("xmark", "close") { [weak self] in self?.findBar.isHidden = true },
        ])
        row1.spacing = 6
        let replaceOne = UIButton(type: .system)
        replaceOne.setTitle(L("replace"), for: .normal)
        replaceOne.addAction(UIAction { [weak self] _ in self?.replaceCurrent() }, for: .touchUpInside)
        let replaceAll = UIButton(type: .system)
        replaceAll.setTitle(L("replace_all"), for: .normal)
        replaceAll.addAction(UIAction { [weak self] _ in self?.replaceEvery() }, for: .touchUpInside)
        let row2 = UIStackView(arrangedSubviews: [replaceField, replaceOne, replaceAll])
        row2.spacing = 8
        findBar.addArrangedSubview(row1)
        findBar.addArrangedSubview(row2)
        findBar.axis = .vertical
        findBar.spacing = 6
        findBar.isLayoutMarginsRelativeArrangement = true
        findBar.directionalLayoutMargins = NSDirectionalEdgeInsets(top: 6, leading: 10, bottom: 6, trailing: 10)
        findBar.backgroundColor = Palette.surface
        findBar.isHidden = true
    }

    private func find(forward: Bool) {
        let query = findField.text ?? ""
        guard !query.isEmpty else { return }
        let ns = editor.textStorage.string as NSString
        let sel = editor.selectedRange
        var found: NSRange
        if forward {
            let start = min(ns.length, NSMaxRange(sel))
            found = ns.range(of: query, options: .caseInsensitive, range: NSRange(location: start, length: ns.length - start))
            if found.location == NSNotFound {
                found = ns.range(of: query, options: .caseInsensitive)
            }
        } else {
            found = ns.range(of: query, options: [.caseInsensitive, .backwards], range: NSRange(location: 0, length: sel.location))
            if found.location == NSNotFound {
                found = ns.range(of: query, options: [.caseInsensitive, .backwards])
            }
        }
        if found.location == NSNotFound {
            toast(L("not_found"))
        } else {
            editor.reveal(found)
        }
    }

    private func replaceCurrent() {
        let query = findField.text ?? ""
        let sel = editor.selectedRange
        let ns = editor.textStorage.string as NSString
        if sel.length > 0 && ns.substring(with: sel).caseInsensitiveCompare(query) == .orderedSame {
            editor.replaceRange(sel, with: replaceField.text ?? "")
        }
        find(forward: true)
    }

    private func replaceEvery() {
        let query = findField.text ?? ""
        guard !query.isEmpty else { return }
        let ns = editor.textStorage.string as NSString
        let count = ns.components(separatedBy: query).count - 1
        let replaced = ns.replacingOccurrences(of: query, with: replaceField.text ?? "", options: .caseInsensitive, range: NSRange(location: 0, length: ns.length))
        editor.replaceRange(NSRange(location: 0, length: ns.length), with: replaced)
        toast(L("replaced_n", count))
    }

    func textView(_ textView: UITextView, shouldChangeTextIn range: NSRange, replacementText text: String) -> Bool {
        lastInserted = text
        guard text == "\n" else { return true }
        let ns = textView.text as NSString
        let lineRange = ns.lineRange(for: NSRange(location: range.location, length: 0))
        let before = ns.substring(with: NSRange(location: lineRange.location, length: range.location - lineRange.location))
        var indent = before.prefix { $0 == " " || $0 == "\t" }.count
        let trimmed = before.trimmingCharacters(in: .whitespaces)
        if trimmed.hasSuffix(":") && !trimmed.hasPrefix("//") {
            indent += editor.tabWidth
        }
        editor.replaceRange(range, with: "\n" + String(repeating: " ", count: indent))
        return false
    }

    func textViewDidChange(_ textView: UITextView) {
        if lastInserted.count == 1, let c = lastInserted.first, c.isLetter {
            autoDedent()
        }
        lastInserted = ""
        edited()
    }

    func textViewDidChangeSelection(_ textView: UITextView) {
        editor.setNeedsLayout()
    }

    private func autoDedent() {
        let ns = editor.textStorage.string as NSString
        let caret = editor.selectedRange.location
        let lineRange = ns.lineRange(for: NSRange(location: caret, length: 0))
        let line = ns.substring(with: lineRange).trimmingCharacters(in: .newlines)
        let word = line.trimmingCharacters(in: .whitespaces)
        guard ["end", "else", "پایان", "وگرنه"].contains(word) else { return }
        let indent = line.prefix { $0 == " " }.count
        guard indent >= editor.tabWidth else { return }
        var expected = -1
        var p = lineRange.location
        while p > 0 {
            let prev = ns.lineRange(for: NSRange(location: p - 1, length: 0))
            let text = ns.substring(with: prev).trimmingCharacters(in: .newlines)
            if !text.trimmingCharacters(in: .whitespaces).isEmpty {
                let base = text.prefix { $0 == " " }.count
                expected = base + (text.trimmingCharacters(in: .whitespaces).hasSuffix(":") ? editor.tabWidth : 0)
                break
            }
            p = prev.location
        }
        if expected >= 0 && indent >= expected {
            editor.replaceRange(NSRange(location: lineRange.location, length: editor.tabWidth), with: "")
        }
    }

    private func edited() {
        editor.bumpVersion()
        if !dirty {
            dirty = true
            updateTitle()
        }
        diagIndex = -1
        tokensWork?.cancel()
        let t = DispatchWorkItem { [weak self] in self?.requestTokens() }
        tokensWork = t
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.09, execute: t)
        checkWork?.cancel()
        if Prefs.liveCheck {
            let c = DispatchWorkItem { [weak self] in self?.requestCheck() }
            checkWork = c
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.8, execute: c)
        }
        saveWork?.cancel()
        let s = DispatchWorkItem { [weak self] in self?.saveNow() }
        saveWork = s
        DispatchQueue.main.asyncAfter(deadline: .now() + 2, execute: s)
    }

    private func requestTokens() {
        let v = editor.version
        Engine.shared.tokens(editor.textStorage.string, lang: lang) { [weak self] packed in
            self?.editor.setTokens(packed, forVersion: v)
        }
    }

    private func requestCheck() {
        let v = editor.version
        Engine.shared.check(editor.textStorage.string, path: file.path, lang: lang) { [weak self] list in
            guard let self = self, v == self.editor.version else { return }
            self.editor.diagnostics = list
            self.showProblems(list)
        }
    }

    private func showProblems(_ list: [Diagnostic]) {
        if list.isEmpty {
            problems.setTitle(L("no_problems"), for: .normal)
            problems.tintColor = Palette.ok
            return
        }
        let errors = list.filter { $0.isError }.count
        let warnings = list.count - errors
        var head = [String]()
        if errors > 0 {
            head.append(L(errors == 1 ? "one_error" : "n_errors", errors))
        }
        if warnings > 0 {
            head.append(L(warnings == 1 ? "one_warning" : "n_warnings", warnings))
        }
        let first = list.first { $0.isError } ?? list[0]
        problems.setTitle(L("problem_line", head.joined(separator: " · "), first.line, first.message), for: .normal)
        problems.tintColor = errors > 0 ? Palette.error : Palette.warning
    }

    private func nextProblem() {
        let list = editor.diagnostics
        guard !list.isEmpty else { return }
        diagIndex = (diagIndex + 1) % list.count
        let d = list[diagIndex]
        editor.reveal(NSRange(location: d.start, length: max(0, d.end - d.start)))
        editor.becomeFirstResponder()
        problems.setTitle(L("problem_at", diagIndex + 1, list.count, d.line, d.message), for: .normal)
    }

    @objc private func saveNow() {
        saveWork?.cancel()
        guard dirty else { return }
        do {
            try editor.textStorage.string.write(to: file, atomically: true, encoding: .utf8)
            dirty = false
            updateTitle()
        } catch {
            toast(L("save_failed"))
        }
    }

    @objc private func toggleRun() {
        if Engine.shared.isRunning {
            Engine.shared.stop()
        } else {
            startJob(["exec", file.lastPathComponent], cwd: file.deletingLastPathComponent().path, interactive: true, done: nil)
        }
    }

    private func startJob(_ args: [String], cwd: String, interactive: Bool, done: ((Int32) -> Void)?) {
        if Engine.shared.isRunning {
            toast(L("busy"))
            return
        }
        saveNow()
        view.endEditing(true)
        console.isHidden = false
        console.clear()
        console.setRunning(interactive)
        Engine.shared.onOutput = { [weak self] text, isError in
            self?.console.append(text, color: isError ? Palette.error : Palette.text)
        }
        Engine.shared.onDone = { [weak self] token, rc, ms in
            self?.finished(token: token, rc: rc, ms: ms)
        }
        let token = Engine.shared.run(args, cwd: cwd)
        if let done = done {
            jobs[token] = done
        }
        if interactive {
            setRunIcon(true)
        }
    }

    private func finished(token: Int, rc: Int32, ms: Int) {
        console.setRunning(false)
        setRunIcon(false)
        let time = ms < 1000 ? "\(ms) ms" : String(format: "%.2f s", Double(ms) / 1000)
        switch rc {
        case 130:
            console.setStatus(L("stopped"), color: Palette.warning)
        case 0:
            console.setStatus(L("finished", time), color: Palette.ok)
        default:
            console.setStatus(L("exit_code", Int(rc), time), color: Palette.error)
        }
        jobs.removeValue(forKey: token)?(rc)
    }

    private func setRunIcon(_ running: Bool) {
        runItem.image = UIImage(systemName: running ? "stop.fill" : "play.fill")
        runItem.tintColor = running ? Palette.error : Palette.brand
        runItem.accessibilityLabel = L(running ? "stop" : "run")
    }

    private func buildMenu() -> UIMenu {
        let a = { (title: String, image: String, action: @escaping () -> Void) -> UIAction in
            UIAction(title: L(title), image: UIImage(systemName: image)) { _ in action() }
        }
        let inspect = UIMenu(title: L("inspect"), image: UIImage(systemName: "eye"), children: [
            a("inspect_tokens", "text.word.spacing") { [weak self] in self?.inspect("--emit-tokens", "inspect_tokens") },
            a("inspect_ast", "list.bullet.indent") { [weak self] in self?.inspect("--emit-ast", "inspect_ast") },
            a("inspect_symbols", "tablecells") { [weak self] in self?.inspect("--emit-symbol", "inspect_symbols") },
            a("inspect_js", "curlybraces") { [weak self] in self?.emit("js", "js", "inspect_js") },
            a("inspect_llvm", "cpu") { [weak self] in self?.emit("llvm", "ll", "inspect_llvm") },
        ])
        return UIMenu(children: [
            a("save", "square.and.arrow.down") { [weak self] in self?.saveNow() },
            a("format", "text.alignleft") { [weak self] in self?.formatFile() },
            a("translate", "character.book.closed") { [weak self] in self?.translateFile() },
            a("find_replace", "magnifyingglass") { [weak self] in
                self?.findBar.isHidden = false
                self?.findField.becomeFirstResponder()
            },
            a("go_to_line", "arrow.right.to.line") { [weak self] in self?.askGoToLine() },
            a("check_now", "checkmark.seal") { [weak self] in self?.requestCheck() },
            a("preview_page", "safari") { [weak self] in self?.previewPage() },
            inspect,
            a("share", "square.and.arrow.up") { [weak self] in self?.shareFile() },
            a("console", "terminal") { [weak self] in self?.console.isHidden = false },
            a("settings", "gearshape") { [weak self] in self?.navigationController?.pushViewController(SettingsViewController(), animated: true) },
        ])
    }

    private var cwd: String { file.deletingLastPathComponent().path }

    private func replaceAllText(_ text: String) {
        let len = (editor.textStorage.string as NSString).length
        editor.replaceRange(NSRange(location: 0, length: len), with: text)
    }

    private func formatFile() {
        startJob(["format", file.lastPathComponent], cwd: cwd, interactive: false) { [weak self] rc in
            guard let self = self, rc == 0, let text = try? String(contentsOf: self.file, encoding: .utf8) else { return }
            if text != self.editor.textStorage.string {
                self.replaceAllText(text)
                self.saveNow()
            }
            self.console.isHidden = true
            self.toast(L("formatted"))
        }
    }

    private func translateFile() {
        let target = lang == "fa" ? "en" : "fa"
        let out = Workspace.cache.appendingPathComponent("translated.salam")
        try? FileManager.default.removeItem(at: out)
        startJob(["translate", target, file.lastPathComponent, "--output=" + out.path], cwd: cwd, interactive: false) { [weak self] rc in
            guard let self = self, rc == 0, let text = try? String(contentsOf: out, encoding: .utf8) else { return }
            self.replaceAllText(text)
            self.setLang(target)
            self.saveNow()
            self.console.isHidden = true
            self.requestTokens()
            self.requestCheck()
        }
    }

    private func inspect(_ flag: String, _ title: String) {
        let out = Workspace.cache.appendingPathComponent("inspect.xml")
        try? FileManager.default.removeItem(at: out)
        startJob(["inspect", file.lastPathComponent, flag, "--xml-out=" + out.path], cwd: cwd, interactive: false) { [weak self] rc in
            if rc == 0 {
                self?.openViewer(out, title: L(title))
            }
        }
    }

    private func emit(_ command: String, _ ext: String, _ title: String) {
        let out = Workspace.cache.appendingPathComponent("emit.\(ext)")
        try? FileManager.default.removeItem(at: out)
        startJob([command, file.lastPathComponent, "--output=" + out.path], cwd: cwd, interactive: false) { [weak self] rc in
            if rc == 0 {
                self?.openViewer(out, title: L(title))
            }
        }
    }

    private func previewPage() {
        let out = Workspace.cache.appendingPathComponent("preview.html")
        try? FileManager.default.removeItem(at: out)
        startJob(["layout", "build", file.lastPathComponent, "--inline", "--output=" + out.path], cwd: cwd, interactive: false) { [weak self] rc in
            guard let self = self, rc == 0, let html = try? String(contentsOf: out, encoding: .utf8) else { return }
            self.console.isHidden = true
            self.navigationController?.pushViewController(PreviewViewController(html: html), animated: true)
        }
    }

    private func openViewer(_ url: URL, title: String) {
        console.isHidden = true
        navigationController?.pushViewController(TextViewerViewController(url: url, title: title), animated: true)
    }

    private func askGoToLine() {
        let alert = UIAlertController(title: L("go_to_line"), message: nil, preferredStyle: .alert)
        alert.addTextField { $0.keyboardType = .numberPad
            $0.placeholder = "1 - \(self.editor.lineCount())"
        }
        alert.addAction(UIAlertAction(title: L("cancel"), style: .cancel))
        alert.addAction(UIAlertAction(title: L("go"), style: .default) { [weak self, weak alert] _ in
            if let n = Int(alert?.textFields?.first?.text ?? "") {
                self?.editor.goToLine(n)
                self?.editor.becomeFirstResponder()
            }
        })
        present(alert, animated: true)
    }

    private func shareFile() {
        saveNow()
        let vc = UIActivityViewController(activityItems: [file], applicationActivities: nil)
        vc.popoverPresentationController?.barButtonItem = navigationItem.rightBarButtonItems?.first
        present(vc, animated: true)
    }

    private func toast(_ text: String) {
        let alert = UIAlertController(title: nil, message: text, preferredStyle: .alert)
        present(alert, animated: true)
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.1) { alert.dismiss(animated: true) }
    }
}
