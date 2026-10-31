import UIKit

final class CodeTextView: UITextView {
    var tabWidth = 4
    var lang = "en" {
        didSet { applyWrap() }
    }

    var wrapLines = true {
        didSet { applyWrap() }
    }

    var showLineNumbers = true {
        didSet { updateInsets() }
    }

    var diagnostics: [Diagnostic] = [] {
        didSet { overlay.setNeedsDisplay() }
    }

    private(set) var version = 0
    private var tokens: [Int32] = []
    private var tokensVersion = -1
    private var hlRange = NSRange(location: 0, length: 0)
    private var hlVersion = -1
    private var newlines: [Int] = []
    private var newlinesVersion = -1
    private let gutter = GutterView()
    private let overlay = SquiggleView()
    var gutterWidth: CGFloat = 0
    var onZoom: ((CGFloat) -> Void)?

    init(fontSize: CGFloat) {
        let storage = NSTextStorage()
        let manager = NSLayoutManager()
        let container = NSTextContainer(size: CGSize(width: 0, height: CGFloat.greatestFiniteMagnitude))
        container.widthTracksTextView = true
        manager.addTextContainer(container)
        storage.addLayoutManager(manager)
        super.init(frame: .zero, textContainer: container)
        font = UIFont.monospacedSystemFont(ofSize: fontSize, weight: .regular)
        textColor = Palette.text
        backgroundColor = Palette.background
        autocorrectionType = .no
        autocapitalizationType = .none
        spellCheckingType = .no
        smartQuotesType = .no
        smartDashesType = .no
        smartInsertDeleteType = .no
        keyboardDismissMode = .interactive
        alwaysBounceVertical = true
        textContainer.lineFragmentPadding = 6
        layoutManager.allowsNonContiguousLayout = true
        typingAttributes = baseAttributes()
        gutter.owner = self
        overlay.owner = self
        gutter.isUserInteractionEnabled = false
        overlay.isUserInteractionEnabled = false
        gutter.backgroundColor = Palette.gutter
        overlay.backgroundColor = .clear
        overlay.contentMode = .redraw
        gutter.contentMode = .redraw
        addSubview(overlay)
        addSubview(gutter)
        addGestureRecognizer(UIPinchGestureRecognizer(target: self, action: #selector(pinched(_:))))
        updateInsets()
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    func baseAttributes() -> [NSAttributedString.Key: Any] {
        [.font: font ?? UIFont.monospacedSystemFont(ofSize: 15, weight: .regular), .foregroundColor: Palette.text]
    }

    func load(_ text: String) {
        textStorage.setAttributedString(NSAttributedString(string: text, attributes: baseAttributes()))
        undoManager?.removeAllActions()
        bumpVersion()
    }

    func bumpVersion() {
        version += 1
        updateInsets()
        gutter.setNeedsDisplay()
        overlay.setNeedsDisplay()
    }

    func setTokens(_ packed: [Int32], forVersion v: Int) {
        guard v == version else { return }
        tokens = packed
        tokensVersion = v
        hlVersion = -1
        applyHighlight()
    }

    func setFontSize(_ size: CGFloat) {
        font = UIFont.monospacedSystemFont(ofSize: size, weight: .regular)
        typingAttributes = baseAttributes()
        hlVersion = -1
        applyHighlight()
        updateInsets()
    }

    @objc private func pinched(_ g: UIPinchGestureRecognizer) {
        guard g.state == .changed, let f = font else { return }
        let next = min(34, max(9, f.pointSize * g.scale))
        g.scale = 1
        setFontSize(next)
        onZoom?(next)
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        let w = showLineNumbers ? gutterWidth : 0
        gutter.isHidden = !showLineNumbers
        gutter.frame = CGRect(x: contentOffset.x, y: contentOffset.y, width: w, height: bounds.height)
        overlay.frame = CGRect(x: contentOffset.x, y: contentOffset.y, width: bounds.width, height: bounds.height)
        gutter.setNeedsDisplay()
        overlay.setNeedsDisplay()
        let visible = visibleCharacterRange()
        if hlVersion == tokensVersion && (visible.location < hlRange.location || NSMaxRange(visible) > NSMaxRange(hlRange)) {
            hlVersion = -1
            DispatchQueue.main.async { [weak self] in self?.applyHighlight() }
        }
    }

    private func applyWrap() {
        let wide = !wrapLines && lang != "fa"
        textContainer.widthTracksTextView = !wide
        if wide {
            textContainer.size = CGSize(width: 20000, height: CGFloat.greatestFiniteMagnitude)
        } else {
            textContainer.size = CGSize(width: bounds.width - textContainerInset.left - textContainerInset.right, height: CGFloat.greatestFiniteMagnitude)
        }
        showsHorizontalScrollIndicator = wide
        setNeedsLayout()
    }

    private func updateInsets() {
        let digits = max(2, String(lineCount()).count)
        let numberFont = UIFont.monospacedDigitSystemFont(ofSize: (font?.pointSize ?? 15) * 0.82, weight: .regular)
        let width = ceil(("9".repeated(digits) as NSString).size(withAttributes: [.font: numberFont]).width) + 20
        gutterWidth = width
        let left = showLineNumbers ? width + 4 : 8
        if textContainerInset.left != left {
            textContainerInset = UIEdgeInsets(top: 8, left: left, bottom: 40, right: 8)
        }
    }

    func visibleCharacterRange() -> NSRange {
        var rect = bounds
        rect.origin.x -= textContainerInset.left
        rect.origin.y -= textContainerInset.top
        let glyphs = layoutManager.glyphRange(forBoundingRect: rect, in: textContainer)
        return layoutManager.characterRange(forGlyphRange: glyphs, actualGlyphRange: nil)
    }

    func applyHighlight() {
        guard tokensVersion == version, markedTextRange == nil else { return }
        let length = textStorage.length
        let visible = visibleCharacterRange()
        let pad = max(2000, visible.length)
        let from = max(0, visible.location - pad)
        let to = min(length, NSMaxRange(visible) + pad)
        if hlVersion == version && from >= hlRange.location && to <= NSMaxRange(hlRange) {
            return
        }
        let range = NSRange(location: from, length: to - from)
        textStorage.beginEditing()
        textStorage.addAttribute(.foregroundColor, value: Palette.text, range: range)
        let n = tokens.count / 3
        var lo = 0
        var hi = n
        while lo < hi {
            let mid = (lo + hi) / 2
            if Int(tokens[mid * 3 + 1] + tokens[mid * 3 + 2]) <= from {
                lo = mid + 1
            } else {
                hi = mid
            }
        }
        var i = lo
        while i < n {
            let s = Int(tokens[i * 3 + 1])
            if s >= to {
                break
            }
            let e = min(length, s + Int(tokens[i * 3 + 2]))
            if s < e {
                textStorage.addAttribute(.foregroundColor, value: Palette.token(Int(tokens[i * 3])), range: NSRange(location: s, length: e - s))
            }
            i += 1
        }
        textStorage.endEditing()
        hlRange = range
        hlVersion = version
    }

    func lineCount() -> Int {
        refreshNewlines()
        return newlines.count + 1
    }

    func lineOf(_ offset: Int) -> Int {
        refreshNewlines()
        var lo = 0
        var hi = newlines.count
        while lo < hi {
            let mid = (lo + hi) / 2
            if newlines[mid] < offset {
                lo = mid + 1
            } else {
                hi = mid
            }
        }
        return lo + 1
    }

    private func refreshNewlines() {
        guard newlinesVersion != version else { return }
        let ns = textStorage.string as NSString
        var out = [Int]()
        for i in 0..<ns.length where ns.character(at: i) == 10 {
            out.append(i)
        }
        newlines = out
        newlinesVersion = version
    }

    func rect(forCharacterRange r: NSRange) -> [CGRect] {
        let glyphs = layoutManager.glyphRange(forCharacterRange: r, actualCharacterRange: nil)
        var rects = [CGRect]()
        layoutManager.enumerateEnclosingRects(forGlyphRange: glyphs, withinSelectedGlyphRange: NSRange(location: NSNotFound, length: 0), in: textContainer) { rect, _ in
            rects.append(rect)
        }
        return rects
    }

    func goToLine(_ n: Int) {
        let ns = textStorage.string as NSString
        var line = 1
        var i = 0
        while line < n && i < ns.length {
            if ns.character(at: i) == 10 {
                line += 1
            }
            i += 1
        }
        reveal(NSRange(location: i, length: 0))
    }

    func reveal(_ r: NSRange) {
        let len = textStorage.length
        let loc = min(max(0, r.location), len)
        selectedRange = NSRange(location: loc, length: min(r.length, len - loc))
        scrollRangeToVisible(selectedRange)
    }

    func replaceRange(_ r: NSRange, with text: String) {
        guard let start = position(from: beginningOfDocument, offset: r.location),
              let end = position(from: start, offset: r.length),
              let range = textRange(from: start, to: end)
        else { return }
        replace(range, withText: text)
    }

    func insertAtCaret(_ s: String, caretBack: Int = 0) {
        let r = selectedRange
        replaceRange(r, with: s)
        let loc = r.location + (s as NSString).length - caretBack
        selectedRange = NSRange(location: max(0, loc), length: 0)
    }

    func indentSelection(outdent: Bool) {
        let ns = textStorage.string as NSString
        let sel = selectedRange
        if sel.length == 0 && !outdent {
            insertAtCaret(String(repeating: " ", count: tabWidth))
            return
        }
        let lines = ns.lineRange(for: sel)
        let block = ns.substring(with: lines)
        let pad = String(repeating: " ", count: tabWidth)
        var out = [String]()
        for line in block.components(separatedBy: "\n") {
            if outdent {
                var drop = 0
                for ch in line.prefix(tabWidth) {
                    if ch == " " { drop += 1 } else { break }
                }
                out.append(String(line.dropFirst(drop)))
            } else {
                out.append(line.isEmpty ? line : pad + line)
            }
        }
        replaceRange(lines, with: out.joined(separator: "\n"))
    }

    final class GutterView: UIView {
        weak var owner: CodeTextView?

        override func draw(_ rect: CGRect) {
            guard let tv = owner, let font = tv.font else { return }
            let ns = tv.textStorage.string as NSString
            let numberFont = UIFont.monospacedDigitSystemFont(ofSize: font.pointSize * 0.82, weight: .regular)
            var errorLines = [Int: Bool]()
            for d in tv.diagnostics {
                errorLines[d.line] = (errorLines[d.line] ?? false) || d.isError
            }
            let caretLine = tv.lineOf(tv.selectedRange.location)
            var visible = tv.bounds
            visible.origin.x = 0
            visible.origin.y -= tv.textContainerInset.top
            let glyphs = tv.layoutManager.glyphRange(forBoundingRect: visible, in: tv.textContainer)
            let para = NSMutableParagraphStyle()
            para.alignment = .right
            tv.layoutManager.enumerateLineFragments(forGlyphRange: glyphs) { fragment, _, _, glyphRange, _ in
                let charIndex = tv.layoutManager.characterIndexForGlyph(at: glyphRange.location)
                if charIndex > 0 && charIndex <= ns.length && ns.character(at: charIndex - 1) != 10 {
                    return
                }
                let number = tv.lineOf(charIndex)
                let y = fragment.minY + tv.textContainerInset.top - tv.contentOffset.y
                let err = errorLines[number]
                let color: UIColor
                if let e = err {
                    color = e ? Palette.error : Palette.warning
                } else {
                    color = number == caretLine ? Palette.text : Palette.gutterText
                }
                let attrs: [NSAttributedString.Key: Any] = [.font: numberFont, .foregroundColor: color, .paragraphStyle: para]
                let h = fragment.height
                let textH = numberFont.lineHeight
                ("\(number)" as NSString).draw(
                    in: CGRect(x: 0, y: y + (h - textH) / 2, width: self.bounds.width - 8, height: textH),
                    withAttributes: attrs
                )
                if err != nil {
                    color.setFill()
                    UIBezierPath(ovalIn: CGRect(x: 4, y: y + h / 2 - 2.5, width: 5, height: 5)).fill()
                }
            }
        }
    }

    final class SquiggleView: UIView {
        weak var owner: CodeTextView?

        override func draw(_ rect: CGRect) {
            guard let tv = owner, !tv.diagnostics.isEmpty else { return }
            let len = tv.textStorage.length
            let visible = tv.visibleCharacterRange()
            for d in tv.diagnostics {
                let s = min(max(0, d.start), len)
                let e = min(max(s, d.end), len)
                if e < visible.location || s > NSMaxRange(visible) {
                    continue
                }
                let color = d.isError ? Palette.error : Palette.warning
                color.setStroke()
                for r in tv.rect(forCharacterRange: NSRange(location: s, length: max(1, e - s)).clamped(len)) {
                    let x1 = r.minX + tv.textContainerInset.left - tv.contentOffset.x
                    let x2 = max(r.maxX, r.minX + 6) + tv.textContainerInset.left - tv.contentOffset.x
                    let y = r.maxY + tv.textContainerInset.top - tv.contentOffset.y - 1
                    let path = UIBezierPath()
                    path.lineWidth = 1.2
                    path.move(to: CGPoint(x: x1, y: y))
                    var x = x1
                    var up = true
                    while x < x2 {
                        x += 2.5
                        path.addLine(to: CGPoint(x: min(x, x2), y: up ? y - 1.6 : y))
                        up.toggle()
                    }
                    path.stroke()
                }
            }
        }
    }
}

extension NSRange {
    func clamped(_ length: Int) -> NSRange {
        let loc = Swift.min(Swift.max(0, location), length)
        return NSRange(location: loc, length: Swift.min(self.length, length - loc))
    }
}

extension String {
    func repeated(_ n: Int) -> String {
        String(repeating: self, count: n)
    }
}
