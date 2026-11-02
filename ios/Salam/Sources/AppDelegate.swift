import UIKit

@main
final class AppDelegate: UIResponder, UIApplicationDelegate {
    var window: UIWindow?

    static var shared: AppDelegate? {
        UIApplication.shared.delegate as? AppDelegate
    }

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        let window = UIWindow(frame: UIScreen.main.bounds)
        self.window = window
        AppDelegate.rebuildInterface()
        window.makeKeyAndVisible()
        DispatchQueue.global(qos: .userInitiated).async {
            Engine.shared.boot()
        }
        return true
    }

    static func applyAppearance() {
        shared?.window?.overrideUserInterfaceStyle = Prefs.interfaceStyle
        shared?.window?.tintColor = Palette.brand
    }

    static func rebuildInterface() {
        guard let window = shared?.window else { return }
        L10n.applyDirection()
        let nav = UINavigationController(rootViewController: HomeViewController())
        nav.navigationBar.tintColor = Palette.brand
        nav.view.semanticContentAttribute = L10n.isRTL ? .forceRightToLeft : .forceLeftToRight
        nav.navigationBar.semanticContentAttribute = nav.view.semanticContentAttribute
        window.rootViewController = nav
        applyAppearance()
    }
}
