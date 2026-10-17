import UIKit

final class ConsoleView: UIView, UITextFieldDelegate {
    var onInput: ((String) -> Void)?
    var onStop: (() -> Void)?
    var onClose: (() -> Void)?
    var onResize: ((CGFloat) -> Void)?

    private let output = UITextView()
    private let status = UILabel()
    private let stopButton = UIButton(type: .system)
    private let inputRow = UIStackView()
    private let field = UITextField()
    private let buffer = NSMutableAttributedString()
    private var flushScheduled = false
    private let limit = 200_000
    private let mono = UIFont.monospacedSystemFont(ofSize: 13, weight: .regular)

    override init(frame: CGRect) {
        super.init(frame: frame)
        backgroundColor = Palette.surface

        let handle = UIView()
        handle.backgroundColor = Palette.border
        handle.layer.cornerRadius = 2
        let handleBox = UIView()
        handleBox.addSubview(handle)
        handle.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            handle.centerXAnchor.constraint(equalTo: handleBox.centerXAnchor),
            handle.centerYAnchor.constraint(equalTo: handleBox.centerYAnchor),
            handle.widthAnchor.constraint(equalToConstant: 40),
            handle.heightAnchor.constraint(equalToConstant: 4),
            handleBox.heightAnchor.constraint(equalToConstant: 14),
        ])
        handleBox.addGestureRecognizer(UIPanGestureRecognizer(target: self, action: #selector(dragged(_:))))

        let title = UILabel()
        title.text = L("console")
        title.font = .boldSystemFont(ofSize: 14)
        title.textColor = Palette.text
        status.font = .systemFont(ofSize: 13)
        status.textColor = Palette.textDim
        stopButton.setImage(UIImage(systemName: "stop.fill"), for: .normal)
        stopButton.tintColor = Palette.error
        stopButton.accessibilityLabel = L("stop")
        stopButton.isHidden = true
        stopButton.addAction(UIAction { [weak self] _ in self?.onStop?() }, for: .touchUpInside)
        let close = UIButton(type: .system)
        close.setImage(UIImage(systemName: "xmark"), for: .normal)
        close.tintColor = Palette.textDim
        close.accessibilityLabel = L("close")
        close.addAction(UIAction { [weak self] _ in self?.onClose?() }, for: .touchUpInside)
        let header = UIStackView(arrangedSubviews: [title, status, UIView(), stopButton, close])
        header.spacing = 10
        header.alignment = .center
        header.isLayoutMarginsRelativeArrangement = true
        header.directionalLayoutMargins = NSDirectionalEdgeInsets(top: 0, leading: 12, bottom: 4, trailing: 8)

        output.isEditable = false
        output.isSelectable = true
        output.font = mono
        output.textColor = Palette.text
        output.backgroundColor = Palette.background
        output.textContainerInset = UIEdgeInsets(top: 6, left: 8, bottom: 8, right: 8)
        output.alwaysBounceVertical = true

        field.placeholder = L("input_hint")
        field.font = .monospacedSystemFont(ofSize: 14, weight: .regular)
        field.borderStyle = .roundedRect
        field.autocorrectionType = .no
        field.autocapitalizationType = .none
        field.returnKeyType = .send
        field.delegate = self
        let send = UIButton(type: .system)
        send.setImage(UIImage(systemName: "paperplane.fill"), for: .normal)
        send.tintColor = Palette.brand
        send.accessibilityLabel = L("send")
        send.addAction(UIAction { [weak self] _ in self?.submit() }, for: .touchUpInside)
        inputRow.addArrangedSubview(field)
        inputRow.addArrangedSubview(send)
        inputRow.spacing = 8
        inputRow.isLayoutMarginsRelativeArrangement = true
        inputRow.directionalLayoutMargins = NSDirectionalEdgeInsets(top: 6, leading: 8, bottom: 6, trailing: 8)
        inputRow.isHidden = true

        let stack = UIStackView(arrangedSubviews: [handleBox, header, output, inputRow])
        stack.axis = .vertical
        addSubview(stack)
        stack.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            stack.topAnchor.constraint(equalTo: topAnchor),
            stack.bottomAnchor.constraint(equalTo: bottomAnchor),
            stack.leadingAnchor.constraint(equalTo: leadingAnchor),
            stack.trailingAnchor.constraint(equalTo: trailingAnchor),
        ])
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    @objc private func dragged(_ g: UIPanGestureRecognizer) {
        let dy = g.translation(in: self).y
        g.setTranslation(.zero, in: self)
        onResize?(-dy)
    }

    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        submit()
        return false
    }

    private func submit() {
        let line = field.text ?? ""
        field.text = ""
        append(line + "\n", color: Palette.textDim)
        onInput?(line + "\n")
    }

    func clear() {
        buffer.setAttributedString(NSAttributedString())
        output.attributedText = buffer
    }

    func setRunning(_ running: Bool) {
        stopButton.isHidden = !running
        inputRow.isHidden = !running
        if running {
            status.text = L("running")
            status.textColor = Palette.textDim
        }
    }

    func focusInput() {
        field.becomeFirstResponder()
    }

    func setStatus(_ text: String, color: UIColor) {
        status.text = text
        status.textColor = color
    }

    func append(_ text: String, color: UIColor = Palette.text) {
        guard !text.isEmpty else { return }
        buffer.append(NSAttributedString(string: text, attributes: [.font: mono, .foregroundColor: color]))
        if buffer.length > limit {
            buffer.deleteCharacters(in: NSRange(location: 0, length: buffer.length - limit / 2))
        }
        guard !flushScheduled else { return }
        flushScheduled = true
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.05) { [weak self] in
            guard let self = self else { return }
            self.flushScheduled = false
            self.output.attributedText = self.buffer
            let end = NSRange(location: max(0, self.buffer.length - 1), length: 1)
            self.output.scrollRangeToVisible(end)
        }
    }
}
