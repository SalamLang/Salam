import UIKit
import UniformTypeIdentifiers

final class HomeViewController: UITableViewController, UIDocumentPickerDelegate {
    private var files: [URL] = []
    private let dateFormatter: RelativeDateTimeFormatter = {
        let f = RelativeDateTimeFormatter()
        f.unitsStyle = .short
        return f
    }()

    init() {
        super.init(style: .insetGrouped)
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        title = L("app_name")
        navigationItem.largeTitleDisplayMode = .always
        navigationController?.navigationBar.prefersLargeTitles = true
        navigationItem.rightBarButtonItem = UIBarButtonItem(
            image: UIImage(systemName: "gearshape"),
            style: .plain,
            target: self,
            action: #selector(openSettings)
        )
        navigationItem.rightBarButtonItem?.accessibilityLabel = L("settings")
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "action")
        tableView.register(FileCell.self, forCellReuseIdentifier: "file")
        dateFormatter.locale = Locale(identifier: Prefs.effectiveUiLang)
        Engine.shared.version { [weak self] v in
            self?.navigationItem.prompt = nil
            self?.tableView.tableHeaderView = self?.header(L("version_line", v))
        }
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        refresh()
    }

    private func header(_ text: String) -> UIView {
        let label = UILabel()
        label.text = text
        label.font = .systemFont(ofSize: 13)
        label.textColor = Palette.textDim
        label.textAlignment = .natural
        let box = UIView(frame: CGRect(x: 0, y: 0, width: tableView.bounds.width, height: 28))
        box.addSubview(label)
        label.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            label.leadingAnchor.constraint(equalTo: box.layoutMarginsGuide.leadingAnchor, constant: 8),
            label.centerYAnchor.constraint(equalTo: box.centerYAnchor),
        ])
        return box
    }

    private func refresh() {
        files = Workspace.files()
        tableView.reloadData()
    }

    @objc private func openSettings() {
        navigationController?.pushViewController(SettingsViewController(), animated: true)
    }

    override func numberOfSections(in tableView: UITableView) -> Int { 2 }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        section == 0 ? 3 : max(1, files.count)
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        section == 1 ? L("your_files") : nil
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        if indexPath.section == 0 {
            let cell = tableView.dequeueReusableCell(withIdentifier: "action", for: indexPath)
            var content = cell.defaultContentConfiguration()
            let items = [("new_file", "plus.circle.fill"), ("import_file", "square.and.arrow.down"), ("examples", "book")]
            content.text = L(items[indexPath.row].0)
            content.image = UIImage(systemName: items[indexPath.row].1)
            content.imageProperties.tintColor = Palette.brand
            if indexPath.row == 0 {
                content.textProperties.color = Palette.brand
                content.textProperties.font = .boldSystemFont(ofSize: 17)
            }
            cell.contentConfiguration = content
            cell.accessoryType = indexPath.row == 2 ? .disclosureIndicator : .none
            return cell
        }
        let cell = tableView.dequeueReusableCell(withIdentifier: "file", for: indexPath)
        var content = UIListContentConfiguration.subtitleCell()
        if files.isEmpty {
            content.text = L("empty_title")
            content.secondaryText = L("empty_hint")
            content.textProperties.color = Palette.textDim
            cell.accessoryType = .none
            cell.selectionStyle = .none
        } else {
            let f = files[indexPath.row]
            content.text = f.lastPathComponent
            content.image = UIImage(systemName: "doc.text.fill")
            content.imageProperties.tintColor = Palette.brand
            let size = (try? f.resourceValues(forKeys: [.fileSizeKey]).fileSize) ?? 0
            let when = dateFormatter.localizedString(for: Workspace.modified(f), relativeTo: Date())
            content.secondaryText = "\(when) · \(ByteCountFormatter.string(fromByteCount: Int64(size), countStyle: .file))"
            content.secondaryTextProperties.color = Palette.textDim
            cell.accessoryType = .disclosureIndicator
            cell.selectionStyle = .default
        }
        cell.contentConfiguration = content
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        if indexPath.section == 0 {
            switch indexPath.row {
            case 0: askNewFile()
            case 1: pickImport()
            default: navigationController?.pushViewController(ExamplesViewController(), animated: true)
            }
            return
        }
        guard !files.isEmpty else { return }
        open(files[indexPath.row])
    }

    func open(_ url: URL) {
        navigationController?.pushViewController(EditorViewController(file: url), animated: true)
    }

    override func tableView(_ tableView: UITableView, trailingSwipeActionsConfigurationForRowAt indexPath: IndexPath) -> UISwipeActionsConfiguration? {
        guard indexPath.section == 1, !files.isEmpty else { return nil }
        let f = files[indexPath.row]
        let delete = UIContextualAction(style: .destructive, title: L("delete")) { [weak self] _, _, done in
            try? FileManager.default.removeItem(at: f)
            self?.refresh()
            done(true)
        }
        let rename = UIContextualAction(style: .normal, title: L("rename")) { [weak self] _, _, done in
            self?.rename(f)
            done(true)
        }
        rename.backgroundColor = Palette.brand
        return UISwipeActionsConfiguration(actions: [delete, rename])
    }

    override func tableView(_ tableView: UITableView, contextMenuConfigurationForRowAt indexPath: IndexPath, point: CGPoint) -> UIContextMenuConfiguration? {
        guard indexPath.section == 1, !files.isEmpty else { return nil }
        let f = files[indexPath.row]
        return UIContextMenuConfiguration(identifier: nil, previewProvider: nil) { _ in
            UIMenu(children: [
                UIAction(title: L("rename"), image: UIImage(systemName: "pencil")) { [weak self] _ in self?.rename(f) },
                UIAction(title: L("duplicate"), image: UIImage(systemName: "plus.square.on.square")) { [weak self] _ in
                    if let text = try? String(contentsOf: f, encoding: .utf8) {
                        try? text.write(to: Workspace.unique(f.lastPathComponent), atomically: true, encoding: .utf8)
                    }
                    self?.refresh()
                },
                UIAction(title: L("share"), image: UIImage(systemName: "square.and.arrow.up")) { [weak self] _ in
                    let vc = UIActivityViewController(activityItems: [f], applicationActivities: nil)
                    vc.popoverPresentationController?.sourceView = self?.tableView.cellForRow(at: indexPath)
                    self?.present(vc, animated: true)
                },
                UIAction(title: L("delete"), image: UIImage(systemName: "trash"), attributes: .destructive) { [weak self] _ in
                    try? FileManager.default.removeItem(at: f)
                    self?.refresh()
                },
            ])
        }
    }

    private func askNewFile() {
        let alert = UIAlertController(title: L("new_file"), message: nil, preferredStyle: .alert)
        alert.addTextField { $0.placeholder = L("file_name_hint") }
        for (code, label) in [("fa", "lang_fa"), ("en", "lang_en")] {
            alert.addAction(UIAlertAction(title: L(label), style: .default) { [weak self, weak alert] _ in
                let name = alert?.textFields?.first?.text ?? ""
                Prefs.newFileLang = code
                let url = Workspace.unique(name.isEmpty ? "main" : name)
                try? Workspace.template(code).write(to: url, atomically: true, encoding: .utf8)
                self?.open(url)
            })
        }
        alert.addAction(UIAlertAction(title: L("cancel"), style: .cancel))
        present(alert, animated: true)
    }

    private func rename(_ f: URL) {
        let alert = UIAlertController(title: L("rename"), message: nil, preferredStyle: .alert)
        alert.addTextField { $0.text = f.lastPathComponent }
        alert.addAction(UIAlertAction(title: L("cancel"), style: .cancel))
        alert.addAction(UIAlertAction(title: L("ok"), style: .default) { [weak self, weak alert] _ in
            let target = Workspace.directory.appendingPathComponent(Workspace.cleanName(alert?.textFields?.first?.text ?? ""))
            if FileManager.default.fileExists(atPath: target.path) && target != f {
                return
            }
            try? FileManager.default.moveItem(at: f, to: target)
            self?.refresh()
        })
        present(alert, animated: true)
    }

    private func pickImport() {
        let picker = UIDocumentPickerViewController(forOpeningContentTypes: [.item], asCopy: true)
        picker.delegate = self
        present(picker, animated: true)
    }

    func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentsAt urls: [URL]) {
        guard let src = urls.first, let data = try? Data(contentsOf: src) else { return }
        let dst = Workspace.unique(src.lastPathComponent)
        try? data.write(to: dst)
        open(dst)
    }

    private final class FileCell: UITableViewCell {}
}

final class ExamplesViewController: UITableViewController {
    private let examples = Workspace.examples()

    init() {
        super.init(style: .insetGrouped)
        title = L("examples")
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "c")
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int { examples.count }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "c", for: indexPath)
        var content = cell.defaultContentConfiguration()
        let ex = examples[indexPath.row]
        content.text = L10n.isRTL ? ex.titleFa : ex.titleEn
        content.image = UIImage(systemName: ex.mode == "layout" ? "safari" : "chevron.left.forwardslash.chevron.right")
        content.imageProperties.tintColor = Palette.brand
        cell.contentConfiguration = content
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        let ex = examples[indexPath.row]
        let lang = L10n.isRTL ? "fa" : "en"
        guard let src = Workspace.exampleSource(ex.id, lang: lang) ?? Workspace.exampleSource(ex.id, lang: "en") else { return }
        let url = Workspace.unique(ex.id)
        try? src.write(to: url, atomically: true, encoding: .utf8)
        var stack = navigationController?.viewControllers ?? []
        stack.removeLast()
        stack.append(EditorViewController(file: url))
        navigationController?.setViewControllers(stack, animated: true)
    }
}
