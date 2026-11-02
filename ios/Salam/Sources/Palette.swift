import UIKit

enum Palette {
    private static func dyn(_ light: UInt32, _ dark: UInt32) -> UIColor {
        UIColor { $0.userInterfaceStyle == .dark ? hex(dark) : hex(light) }
    }

    static func hex(_ v: UInt32, alpha: CGFloat = 1) -> UIColor {
        UIColor(
            red: CGFloat((v >> 16) & 0xFF) / 255,
            green: CGFloat((v >> 8) & 0xFF) / 255,
            blue: CGFloat(v & 0xFF) / 255,
            alpha: alpha
        )
    }

    static let brand = hex(0xFF5C00)
    static let background = dyn(0xFFFFFF, 0x15171C)
    static let surface = dyn(0xF5F6F8, 0x1E2128)
    static let surfaceHigh = dyn(0xEBEDF0, 0x272B33)
    static let border = dyn(0xDDE0E5, 0x30343D)
    static let text = dyn(0x1B1D22, 0xE6E8EC)
    static let textDim = dyn(0x6B7280, 0x8D93A0)
    static let gutter = dyn(0xF7F8FA, 0x181A20)
    static let gutterText = dyn(0xA0A6B1, 0x5C6370)
    static let error = dyn(0xD93025, 0xFF6B6B)
    static let warning = dyn(0xB7791F, 0xFFC14D)
    static let ok = dyn(0x15803D, 0x4ADE80)

    private static let tokenColors: [UIColor] = [
        text,
        dyn(0xD4480B, 0xFF8A4C),
        dyn(0x2E7D32, 0x98C379),
        dyn(0x9C5D00, 0xD19A66),
        dyn(0x8A919C, 0x6A7280),
        dyn(0x00838F, 0x56B6C2),
        dyn(0x1E5BC6, 0x61AFEF),
        dyn(0x7A4BC2, 0xE5C07B),
        dyn(0x4B5563, 0xC8CCD4),
        dyn(0xA21CAF, 0xC678DD),
        dyn(0x6B7280, 0x8B95A5),
        dyn(0xC2185B, 0xE06C75),
        dyn(0x9C5D00, 0xD19A66),
    ]

    static func token(_ kind: Int) -> UIColor {
        kind > 0 && kind < tokenColors.count ? tokenColors[kind] : text
    }
}
