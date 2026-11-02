import Foundation

enum Workspace {
    static var directory: URL {
        let url = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
        try? FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    static var cache: URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("salam", isDirectory: true)
        try? FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    static func files() -> [URL] {
        let fm = FileManager.default
        let keys: [URLResourceKey] = [.contentModificationDateKey, .isRegularFileKey]
        let items = (try? fm.contentsOfDirectory(at: directory, includingPropertiesForKeys: keys, options: [.skipsHiddenFiles])) ?? []
        return items
            .filter { (try? $0.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile) == true }
            .sorted { modified($0) > modified($1) }
    }

    static func modified(_ url: URL) -> Date {
        (try? url.resourceValues(forKeys: [.contentModificationDateKey]).contentModificationDate) ?? .distantPast
    }

    static func cleanName(_ raw: String) -> String {
        var name = raw.trimmingCharacters(in: .whitespacesAndNewlines)
        let bad = CharacterSet(charactersIn: "\\/:*?\"<>|").union(.controlCharacters)
        name = name.components(separatedBy: bad).joined(separator: "_")
        if name.isEmpty {
            name = "main"
        }
        if !name.contains(".") {
            name += ".salam"
        }
        return name
    }

    static func unique(_ wanted: String) -> URL {
        let base = cleanName(wanted)
        var url = directory.appendingPathComponent(base)
        let stem = (base as NSString).deletingPathExtension
        let ext = (base as NSString).pathExtension.isEmpty ? "salam" : (base as NSString).pathExtension
        var i = 2
        while FileManager.default.fileExists(atPath: url.path) {
            url = directory.appendingPathComponent("\(stem)-\(i).\(ext)")
            i += 1
        }
        return url
    }

    static func template(_ lang: String) -> String {
        lang == "fa"
            ? "تابع ریشه:\n    سرچاپ «سلام دنیا!»\nپایان\n"
            : "func main:\n    println \"Hello, Salam!\"\nend\n"
    }

    struct Example {
        let id: String
        let mode: String
        let titleEn: String
        let titleFa: String
    }

    static var examplesRoot: URL {
        (Bundle.main.resourceURL ?? URL(fileURLWithPath: ".")).appendingPathComponent("examples")
    }

    static func examples() -> [Example] {
        guard let text = try? String(contentsOf: examplesRoot.appendingPathComponent("index.tsv"), encoding: .utf8) else {
            return []
        }
        return text.split(separator: "\n").compactMap { line in
            let p = line.split(separator: "\t", omittingEmptySubsequences: false).map(String.init)
            return p.count >= 4 ? Example(id: p[0], mode: p[1], titleEn: p[2], titleFa: p[3]) : nil
        }
    }

    static func exampleSource(_ id: String, lang: String) -> String? {
        try? String(contentsOf: examplesRoot.appendingPathComponent(lang).appendingPathComponent("\(id).salam"), encoding: .utf8)
    }
}
