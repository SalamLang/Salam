import UIKit
import WebKit

final class TextViewerViewController: UIViewController {
    private let url: URL
    private let text: String

    init(url: URL, title: String) {
        self.url = url
        let raw = (try? String(contentsOf: url, encoding: .utf8)) ?? ""
        text = raw.count > 400_000 ? String(raw.prefix(400_000)) + "\n…" : raw
        super.init(nibName: nil, bundle: nil)
        self.title = title
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationItem.largeTitleDisplayMode = .never
        view.backgroundColor = Palette.background
        let tv = UITextView()
        tv.isEditable = false
        tv.font = .monospacedSystemFont(ofSize: 12.5, weight: .regular)
        tv.textColor = Palette.text
        tv.backgroundColor = Palette.background
        tv.text = text
        tv.textContainerInset = UIEdgeInsets(top: 10, left: 10, bottom: 24, right: 10)
        tv.semanticContentAttribute = .forceLeftToRight
        tv.textAlignment = .left
        view.addSubview(tv)
        tv.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            tv.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            tv.bottomAnchor.constraint(equalTo: view.bottomAnchor),
            tv.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            tv.trailingAnchor.constraint(equalTo: view.trailingAnchor),
        ])
        navigationItem.rightBarButtonItem = UIBarButtonItem(
            image: UIImage(systemName: "doc.on.doc"),
            style: .plain,
            target: self,
            action: #selector(copyAll)
        )
        navigationItem.rightBarButtonItem?.accessibilityLabel = L("copy")
    }

    @objc private func copyAll() {
        UIPasteboard.general.string = text
    }
}

final class PreviewViewController: UIViewController {
    private let html: String

    init(html: String) {
        self.html = html
        super.init(nibName: nil, bundle: nil)
        title = L("preview_page")
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationItem.largeTitleDisplayMode = .never
        let web = WKWebView(frame: .zero, configuration: WKWebViewConfiguration())
        view.addSubview(web)
        web.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            web.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            web.bottomAnchor.constraint(equalTo: view.bottomAnchor),
            web.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            web.trailingAnchor.constraint(equalTo: view.trailingAnchor),
        ])
        web.loadHTMLString(html, baseURL: URL(string: "https://preview.salamlang.ir/"))
    }
}
