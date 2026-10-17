import Foundation

struct Diagnostic {
    let start: Int
    let end: Int
    let line: Int
    let isError: Bool
    let code: String
    let message: String
}

final class Engine {
    static let shared = Engine()

    var onOutput: ((String, Bool) -> Void)?
    var onDone: ((Int, Int32, Int) -> Void)?

    private(set) var isRunning = false
    private let lock = NSLock()
    private var booted = false
    private var stdinWrite: Int32 = -1
    private var nextToken = 1
    private var activeToken = 0
    private let analysis = DispatchQueue(label: "salam.analysis", qos: .userInitiated)
    private static let mark = "\u{1E}\u{1F}SALAM-END:"
    private static let markEnd = "\u{1F}\u{1E}"

    static var stdPath: String {
        (Bundle.main.resourceURL ?? URL(fileURLWithPath: ".")).appendingPathComponent("std").path
    }

    func boot() {
        lock.lock()
        defer { lock.unlock() }
        if booted {
            return
        }
        let docs = Workspace.directory.path
        setenv("HOME", docs, 1)
        setenv("TMPDIR", NSTemporaryDirectory(), 1)
        if let ca = Bundle.main.path(forResource: "cacert", ofType: "pem") {
            setenv("SSL_CERT_FILE", ca, 1)
        }
        redirect()
        _ = salam_engine_boot(Engine.stdPath)
        booted = true
    }

    private func redirect() {
        var outPipe: [Int32] = [0, 0]
        var errPipe: [Int32] = [0, 0]
        guard pipe(&outPipe) == 0, pipe(&errPipe) == 0 else {
            return
        }
        fflush(stdout)
        fflush(stderr)
        dup2(outPipe[1], 1)
        dup2(errPipe[1], 2)
        close(outPipe[1])
        close(errPipe[1])
        setvbuf(stdout, nil, _IONBF, 0)
        setvbuf(stderr, nil, _IONBF, 0)
        newStdin()
        pump(outPipe[0], isError: false)
        pump(errPipe[0], isError: true)
    }

    private func newStdin() {
        var inPipe: [Int32] = [0, 0]
        guard pipe(&inPipe) == 0 else {
            return
        }
        dup2(inPipe[0], 0)
        close(inPipe[0])
        if stdinWrite >= 0 {
            close(stdinWrite)
        }
        stdinWrite = inPipe[1]
    }

    private func pump(_ fd: Int32, isError: Bool) {
        let thread = Thread { [weak self] in
            var bytes = [UInt8]()
            var pending = ""
            var buffer = [UInt8](repeating: 0, count: 8192)
            while true {
                let n = read(fd, &buffer, buffer.count)
                if n <= 0 {
                    break
                }
                bytes.append(contentsOf: buffer[0..<n])
                let cut = Engine.completeUtf8Prefix(bytes)
                pending += String(decoding: bytes[0..<cut], as: UTF8.self)
                bytes.removeFirst(cut)
                if isError {
                    let text = pending
                    pending = ""
                    DispatchQueue.main.async { self?.onOutput?(text, true) }
                } else {
                    pending = self?.scanMarks(pending) ?? ""
                }
            }
        }
        thread.name = isError ? "salam-stderr" : "salam-stdout"
        thread.start()
    }

    private static func completeUtf8Prefix(_ b: [UInt8]) -> Int {
        var i = b.count
        var back = 0
        while i > 0 && back < 4 {
            let c = b[i - 1]
            if c & 0xC0 != 0x80 {
                let need: Int
                if c & 0x80 == 0 {
                    need = 1
                } else if c & 0xE0 == 0xC0 {
                    need = 2
                } else if c & 0xF0 == 0xE0 {
                    need = 3
                } else if c & 0xF8 == 0xF0 {
                    need = 4
                } else {
                    need = 1
                }
                return b.count - (i - 1) >= need ? b.count : i - 1
            }
            i -= 1
            back += 1
        }
        return b.count
    }

    private func scanMarks(_ text: String) -> String {
        var pending = text
        while true {
            guard let at = pending.range(of: Engine.mark) else {
                var keep = 0
                for k in stride(from: Engine.mark.count - 1, through: 1, by: -1) where pending.hasSuffix(String(Engine.mark.prefix(k))) {
                    keep = k
                    break
                }
                let emit = String(pending.dropLast(keep))
                if !emit.isEmpty {
                    pending = String(pending.suffix(keep))
                    DispatchQueue.main.async { [weak self] in self?.onOutput?(emit, false) }
                }
                return pending
            }
            guard let close = pending.range(of: Engine.markEnd, range: at.upperBound..<pending.endIndex) else {
                let before = String(pending[..<at.lowerBound])
                if !before.isEmpty {
                    pending = String(pending[at.lowerBound...])
                    DispatchQueue.main.async { [weak self] in self?.onOutput?(before, false) }
                }
                return pending
            }
            let before = String(pending[..<at.lowerBound])
            let parts = pending[at.upperBound..<close.lowerBound].split(separator: ":")
            pending = String(pending[close.upperBound...])
            let token = parts.count > 0 ? Int(parts[0]) ?? 0 : 0
            let rc = parts.count > 1 ? Int32(parts[1]) ?? 1 : 1
            let ms = parts.count > 2 ? Int(parts[2]) ?? 0 : 0
            newStdin()
            DispatchQueue.main.async { [weak self] in
                guard let self = self else { return }
                if !before.isEmpty {
                    self.onOutput?(before, false)
                }
                if token == self.activeToken {
                    self.activeToken = 0
                    self.isRunning = false
                    self.onDone?(token, rc, ms)
                }
            }
        }
    }

    @discardableResult
    func run(_ args: [String], cwd: String) -> Int {
        if isRunning {
            return 0
        }
        boot()
        let token = nextToken
        nextToken += 1
        activeToken = token
        isRunning = true
        let joined = args.joined(separator: "\n")
        let thread = Thread { [weak self] in
            guard let self = self else { return }
            self.lock.lock()
            Engine.drainStdin()
            salam_engine_clear_stop()
            chdir(cwd)
            let t0 = Date()
            let rc = salam_engine_cli(joined)
            fflush(stdout)
            let ms = Int(Date().timeIntervalSince(t0) * 1000)
            self.lock.unlock()
            let marker = "\(Engine.mark)\(token):\(rc):\(ms)\(Engine.markEnd)"
            marker.withCString { p in
                _ = write(1, p, strlen(p))
            }
        }
        thread.stackSize = 64 << 20
        thread.name = "salam-run"
        thread.start()
        return token
    }

    func input(_ text: String) {
        guard stdinWrite >= 0 else { return }
        text.withCString { p in
            _ = write(stdinWrite, p, strlen(p))
        }
    }

    func stop() {
        guard isRunning else { return }
        salam_engine_stop()
        if stdinWrite >= 0 {
            close(stdinWrite)
            stdinWrite = -1
        }
    }

    private static func drainStdin() {
        let flags = fcntl(0, F_GETFL)
        if flags < 0 {
            return
        }
        _ = fcntl(0, F_SETFL, flags | O_NONBLOCK)
        var buf = [UInt8](repeating: 0, count: 512)
        while read(0, &buf, buf.count) > 0 {}
        _ = fcntl(0, F_SETFL, flags)
    }

    func tokens(_ source: String, lang: String, done: @escaping ([Int32]) -> Void) {
        analysis.async { [weak self] in
            guard let self = self else { return }
            self.boot()
            guard self.lock.try() else { return }
            let raw = salam_engine_tokens(source, lang)
            self.lock.unlock()
            let packed = Engine.take(raw)
            let result = Engine.parseTokens(packed)
            DispatchQueue.main.async { done(result) }
        }
    }

    func check(_ source: String, path: String, lang: String, done: @escaping ([Diagnostic]) -> Void) {
        analysis.async { [weak self] in
            guard let self = self else { return }
            self.boot()
            guard self.lock.try() else { return }
            let raw = salam_engine_check(source, path, lang)
            self.lock.unlock()
            let json = Engine.take(raw)
            let list = Engine.parseDiagnostics(source, json)
            DispatchQueue.main.async { done(list) }
        }
    }

    func detect(_ source: String, done: @escaping (String) -> Void) {
        analysis.async { [weak self] in
            guard let self = self else { return }
            self.boot()
            self.lock.lock()
            let lang = Engine.take(salam_engine_detect(source))
            self.lock.unlock()
            DispatchQueue.main.async { done(lang) }
        }
    }

    func version(done: @escaping (String) -> Void) {
        analysis.async { [weak self] in
            guard let self = self else { return }
            self.boot()
            self.lock.lock()
            let v = Engine.take(salam_engine_version())
            self.lock.unlock()
            DispatchQueue.main.async { done(v.trimmingCharacters(in: .whitespacesAndNewlines)) }
        }
    }

    private static func take(_ p: UnsafeMutablePointer<CChar>?) -> String {
        guard let p = p else { return "" }
        let s = String(cString: p)
        salam_engine_free(p)
        return s
    }

    private static func parseTokens(_ packed: String) -> [Int32] {
        var out = [Int32]()
        out.reserveCapacity(packed.utf8.count / 4)
        var value: Int32 = 0
        for c in packed.utf8 {
            if c == 44 || c == 59 {
                out.append(value)
                value = 0
            } else if c >= 48 && c <= 57 {
                value = value * 10 + Int32(c - 48)
            }
        }
        return out
    }

    static func parseDiagnostics(_ source: String, _ json: String) -> [Diagnostic] {
        let ns = source as NSString
        var starts = [0]
        for i in 0..<ns.length where ns.character(at: i) == 10 {
            starts.append(i + 1)
        }
        var list = [Diagnostic]()
        for raw in json.split(separator: "\n") {
            guard raw.hasPrefix("{"),
                  let data = raw.data(using: .utf8),
                  let o = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any]
            else { continue }
            let line = max(1, o["line"] as? Int ?? 1)
            let col = max(1, o["column"] as? Int ?? 1)
            let len = o["length"] as? Int ?? 0
            let lineStart = line - 1 < starts.count ? starts[line - 1] : ns.length
            let lineEnd = line < starts.count ? starts[line] - 1 : ns.length
            let start = byteColumn(ns, lineStart, lineEnd, col - 1)
            var end = byteColumn(ns, lineStart, lineEnd, col - 1 + len)
            if end <= start {
                end = min(ns.length, start + 1)
            }
            list.append(
                Diagnostic(
                    start: start,
                    end: end,
                    line: line,
                    isError: (o["level"] as? String) != "warning",
                    code: o["code"] as? String ?? "",
                    message: o["message"] as? String ?? ""
                )
            )
        }
        return list.sorted { $0.start < $1.start }
    }

    private static func byteColumn(_ s: NSString, _ lineStart: Int, _ lineEnd: Int, _ bytes: Int) -> Int {
        var i = lineStart
        var b = 0
        let limit = min(lineEnd, s.length)
        while i < limit && b < bytes {
            let c = s.character(at: i)
            if c < 0x80 {
                b += 1
            } else if c < 0x800 {
                b += 2
            } else if c >= 0xD800 && c <= 0xDBFF {
                b += 4
            } else if c >= 0xDC00 && c <= 0xDFFF {
                b += 0
            } else {
                b += 3
            }
            i += 1
        }
        return min(i, s.length)
    }
}
