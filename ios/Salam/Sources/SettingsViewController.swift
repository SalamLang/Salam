import UIKit

final class SettingsViewController: UITableViewController {
    private enum Row {
        case theme, language, fontSize, tabWidth, wrap, lineNumbers, liveCheck, engine, website, source
    }

    private let sections: [(String, [Row])] = [
        ("appearance", [.theme, .language]),
        ("editor", [.fontSize, .tabWidth, .wrap, .lineNumbers, .liveCheck]),
        ("about", [.engine, .website, .source]),
    ]
    private var engineVersion = ""

    init() {
        super.init(style: .insetGrouped)
        title = L("settings")
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationItem.largeTitleDisplayMode = .never
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "c")
        Engine.shared.version { [weak self] v in
            self?.engineVersion = v
            self?.tableView.reloadData()
        }
    }

    override func numberOfSections(in tableView: UITableView) -> Int { sections.count }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int { sections[section].1.count }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? { L(sections[section].0) }

    override func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        section == 2 ? L("about_text") : nil
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .value1, reuseIdentifier: nil)
        cell.selectionStyle = .none
        var content = cell.defaultContentConfiguration()
        switch sections[indexPath.section].1[indexPath.row] {
        case .theme:
            content.text = L("theme")
            cell.accessoryView = segmented([L("theme_system"), L("theme_light"), L("theme_dark")], Prefs.theme) { v in
                Prefs.theme = v
                AppDelegate.applyAppearance()
            }
        case .language:
            content.text = L("ui_language")
            cell.accessoryView = segmented([L("lang_fa"), L("lang_en")], Prefs.effectiveUiLang == "fa" ? 0 : 1) { v in
                Prefs.uiLang = v == 0 ? "fa" : "en"
                AppDelegate.rebuildInterface()
            }
        case .fontSize:
            content.text = L("font_size", Int(Prefs.fontSize))
            let stepper = UIStepper()
            stepper.minimumValue = 10
            stepper.maximumValue = 32
            stepper.value = Double(Prefs.fontSize)
            stepper.addAction(UIAction { [weak self] action in
                guard let s = action.sender as? UIStepper else { return }
                Prefs.fontSize = CGFloat(s.value)
                self?.tableView.reloadRows(at: [indexPath], with: .none)
            }, for: .valueChanged)
            cell.accessoryView = stepper
        case .tabWidth:
            content.text = L("tab_width")
            cell.accessoryView = segmented(["2", "4"], Prefs.tabWidth == 2 ? 0 : 1) { Prefs.tabWidth = $0 == 0 ? 2 : 4 }
        case .wrap:
            content.text = L("word_wrap")
            cell.accessoryView = toggle(Prefs.wrap) { Prefs.wrap = $0 }
        case .lineNumbers:
            content.text = L("line_numbers")
            cell.accessoryView = toggle(Prefs.lineNumbers) { Prefs.lineNumbers = $0 }
        case .liveCheck:
            content.text = L("live_check")
            cell.accessoryView = toggle(Prefs.liveCheck) { Prefs.liveCheck = $0 }
        case .engine:
            content.text = L("engine_version", engineVersion)
            content.textProperties.color = Palette.textDim
        case .website:
            content.text = L("website")
            content.textProperties.color = Palette.brand
            cell.selectionStyle = .default
        case .source:
            content.text = L("source_code")
            content.textProperties.color = Palette.brand
            cell.selectionStyle = .default
        }
        content.textProperties.numberOfLines = 2
        cell.contentConfiguration = content
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        switch sections[indexPath.section].1[indexPath.row] {
        case .website:
            UIApplication.shared.open(URL(string: "https://salamlang.ir")!)
        case .source:
            UIApplication.shared.open(URL(string: "https://github.com/SalamLang/Salam")!)
        default:
            break
        }
    }

    private func segmented(_ items: [String], _ selected: Int, _ onChange: @escaping (Int) -> Void) -> UISegmentedControl {
        let s = UISegmentedControl(items: items)
        s.selectedSegmentIndex = selected
        s.selectedSegmentTintColor = Palette.brand
        s.setTitleTextAttributes([.foregroundColor: UIColor.white], for: .selected)
        s.addAction(UIAction { action in
            guard let c = action.sender as? UISegmentedControl else { return }
            onChange(c.selectedSegmentIndex)
        }, for: .valueChanged)
        s.sizeToFit()
        return s
    }

    private func toggle(_ value: Bool, _ onChange: @escaping (Bool) -> Void) -> UISwitch {
        let s = UISwitch()
        s.isOn = value
        s.onTintColor = Palette.brand
        s.addAction(UIAction { action in
            guard let c = action.sender as? UISwitch else { return }
            onChange(c.isOn)
        }, for: .valueChanged)
        return s
    }
}
