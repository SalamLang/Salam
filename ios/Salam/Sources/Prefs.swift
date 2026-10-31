import UIKit

enum Prefs {
    private static let d = UserDefaults.standard

    static var theme: Int {
        get { d.integer(forKey: "theme") }
        set { d.set(newValue, forKey: "theme") }
    }

    static var uiLang: String {
        get { d.string(forKey: "ui_lang") ?? "" }
        set { d.set(newValue, forKey: "ui_lang") }
    }

    static var fontSize: CGFloat {
        get {
            let v = d.double(forKey: "font_size")
            return v > 0 ? CGFloat(v) : 15
        }
        set { d.set(Double(newValue), forKey: "font_size") }
    }

    static var tabWidth: Int {
        get {
            let v = d.integer(forKey: "tab_width")
            return v > 0 ? v : 4
        }
        set { d.set(newValue, forKey: "tab_width") }
    }

    static var wrap: Bool {
        get { d.object(forKey: "wrap") as? Bool ?? true }
        set { d.set(newValue, forKey: "wrap") }
    }

    static var lineNumbers: Bool {
        get { d.object(forKey: "line_numbers") as? Bool ?? true }
        set { d.set(newValue, forKey: "line_numbers") }
    }

    static var liveCheck: Bool {
        get { d.object(forKey: "live_check") as? Bool ?? true }
        set { d.set(newValue, forKey: "live_check") }
    }

    static var newFileLang: String {
        get { d.string(forKey: "new_lang") ?? "" }
        set { d.set(newValue, forKey: "new_lang") }
    }

    static var effectiveUiLang: String {
        if !uiLang.isEmpty {
            return uiLang
        }
        let preferred = Locale.preferredLanguages.first ?? "fa"
        return preferred.hasPrefix("en") ? "en" : "fa"
    }

    static var interfaceStyle: UIUserInterfaceStyle {
        switch theme {
        case 1: return .light
        case 2: return .dark
        default: return .unspecified
        }
    }
}

enum L10n {
    private static var cachedLang = ""
    private static var cachedBundle = Bundle.main

    static var bundle: Bundle {
        let lang = Prefs.effectiveUiLang
        if lang != cachedLang {
            cachedLang = lang
            if let path = Bundle.main.path(forResource: lang, ofType: "lproj"), let b = Bundle(path: path) {
                cachedBundle = b
            } else {
                cachedBundle = Bundle.main
            }
        }
        return cachedBundle
    }

    static var isRTL: Bool { Prefs.effectiveUiLang == "fa" }

    static func applyDirection() {
        let attr: UISemanticContentAttribute = isRTL ? .forceRightToLeft : .forceLeftToRight
        UIView.appearance().semanticContentAttribute = attr
        UINavigationBar.appearance().semanticContentAttribute = attr
        UITableView.appearance().semanticContentAttribute = attr
    }
}

func L(_ key: String) -> String {
    L10n.bundle.localizedString(forKey: key, value: key, table: nil)
}

func L(_ key: String, _ args: CVarArg...) -> String {
    String(format: L(key), locale: Locale(identifier: Prefs.effectiveUiLang), arguments: args)
}
