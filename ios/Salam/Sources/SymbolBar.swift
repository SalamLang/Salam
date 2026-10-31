import UIKit

final class SymbolBar: UIInputView {
    private weak var editor: CodeTextView?
    private let scroll = UIScrollView()
    private let row = UIStackView()
    private var lang = ""

    init(editor: CodeTextView) {
        self.editor = editor
        super.init(frame: CGRect(x: 0, y: 0, width: 320, height: 46), inputViewStyle: .keyboard)
        allowsSelfSizing = true
        scroll.showsHorizontalScrollIndicator = false
        row.spacing = 6
        row.isLayoutMarginsRelativeArrangement = true
        row.directionalLayoutMargins = NSDirectionalEdgeInsets(top: 5, leading: 6, bottom: 5, trailing: 6)
        scroll.addSubview(row)
        addSubview(scroll)
        scroll.translatesAutoresizingMaskIntoConstraints = false
        row.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            scroll.topAnchor.constraint(equalTo: topAnchor),
            scroll.bottomAnchor.constraint(equalTo: bottomAnchor),
            scroll.leadingAnchor.constraint(equalTo: leadingAnchor),
            scroll.trailingAnchor.constraint(equalTo: trailingAnchor),
            scroll.heightAnchor.constraint(equalToConstant: 46),
            row.topAnchor.constraint(equalTo: scroll.contentLayoutGuide.topAnchor),
            row.bottomAnchor.constraint(equalTo: scroll.contentLayoutGuide.bottomAnchor),
            row.leadingAnchor.constraint(equalTo: scroll.contentLayoutGuide.leadingAnchor),
            row.trailingAnchor.constraint(equalTo: scroll.contentLayoutGuide.trailingAnchor),
            row.heightAnchor.constraint(equalTo: scroll.frameLayoutGuide.heightAnchor),
        ])
        setLang("en")
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    func setLang(_ code: String) {
        guard code != lang else { return }
        lang = code
        row.arrangedSubviews.forEach { $0.removeFromSuperview() }
        let end = code == "fa" ? "پایان" : "end"
        let quote = code == "fa" ? "«»" : "\"\""
        key("⇥") { $0.indentSelection(outdent: false) }
        key("⇤") { $0.indentSelection(outdent: true) }
        key(":") { $0.insertAtCaret(":") }
        key(end) { $0.insertAtCaret(end) }
        key(":=") { $0.insertAtCaret(" := ") }
        key("( )") { $0.insertAtCaret("()", caretBack: 1) }
        key("[ ]") { $0.insertAtCaret("[]", caretBack: 1) }
        key(quote) { $0.insertAtCaret(quote, caretBack: 1) }
        for s in [".", ",", "=", "+", "-", "*", "/", "%", "<", ">", "!", "&", "|", "_", "{", "}"] {
            key(s) { $0.insertAtCaret(s) }
        }
        key("//") { $0.insertAtCaret("// ") }
        key("↶") { $0.undoManager?.undo() }
        key("↷") { $0.undoManager?.redo() }
        key("⌄") { $0.resignFirstResponder() }
    }

    private func key(_ title: String, _ action: @escaping (CodeTextView) -> Void) {
        var config = UIButton.Configuration.filled()
        config.title = "\u{200E}" + title
        config.baseBackgroundColor = Palette.surfaceHigh
        config.baseForegroundColor = Palette.text
        config.cornerStyle = .medium
        config.contentInsets = NSDirectionalEdgeInsets(top: 4, leading: 10, bottom: 4, trailing: 10)
        config.titleTextAttributesTransformer = UIConfigurationTextAttributesTransformer { attrs in
            var a = attrs
            a.font = UIFont.monospacedSystemFont(ofSize: 15, weight: .medium)
            return a
        }
        let b = UIButton(configuration: config)
        b.widthAnchor.constraint(greaterThanOrEqualToConstant: 38).isActive = true
        b.addAction(UIAction { [weak self] _ in
            guard let editor = self?.editor else { return }
            UIDevice.current.playInputClick()
            action(editor)
        }, for: .touchUpInside)
        row.addArrangedSubview(b)
    }
}

extension SymbolBar: UIInputViewAudioFeedback {
    var enableInputClicksWhenVisible: Bool { true }
}
