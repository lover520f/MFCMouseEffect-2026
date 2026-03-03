@preconcurrency import AppKit
@preconcurrency import QuartzCore
@preconcurrency import Foundation

private func mfxClamp01(_ value: Double) -> Double {
    if value < 0.0 {
        return 0.0
    }
    if value > 1.0 {
        return 1.0
    }
    return value
}

private func mfxClamp(_ value: CGFloat, min minValue: CGFloat, max maxValue: CGFloat) -> CGFloat {
    if value < minValue {
        return minValue
    }
    if value > maxValue {
        return maxValue
    }
    return value
}

private func mfxColorFromArgb(_ argb: UInt32, _ alphaScale: Double) -> NSColor {
    let baseAlpha = Double((argb >> 24) & 0xFF) / 255.0
    let alpha = CGFloat(mfxClamp01(baseAlpha * alphaScale))
    let red = CGFloat(Double((argb >> 16) & 0xFF) / 255.0)
    let green = CGFloat(Double((argb >> 8) & 0xFF) / 255.0)
    let blue = CGFloat(Double(argb & 0xFF) / 255.0)
    return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
}

private func mfxResolveLabelFont(_ fontFamily: String, _ size: CGFloat, _ emoji: Bool) -> NSFont {
    if emoji {
        if let emojiFont = NSFont(name: "Apple Color Emoji", size: size) {
            return emojiFont
        }
        return NSFont.systemFont(ofSize: size, weight: .regular)
    }

    if !fontFamily.isEmpty, let custom = NSFont(name: fontFamily, size: size) {
        return custom
    }
    return NSFont.boldSystemFont(ofSize: size)
}

@MainActor
private final class MfxTextPanelHandle: NSObject {
    private let panel: NSPanel
    private let label: CATextLayer
    private let content: NSView
    private let textValue: String
    private var panelSide: CGFloat = 1.0
    private var currentFont: NSFont = NSFont.boldSystemFont(ofSize: 12.0)

    init(
        text: String,
        panelSize: Double,
        fontSize: Double,
        argb: UInt32,
        fontFamily: String,
        emoji: Bool
    ) {
        let side = CGFloat(max(1.0, panelSize))
        panel = NSPanel(
            contentRect: NSRect(x: 0, y: 0, width: side, height: side),
            styleMask: .borderless,
            backing: .buffered,
            defer: false
        )
        panel.isOpaque = false
        panel.backgroundColor = .clear
        panel.hasShadow = false
        panel.ignoresMouseEvents = true
        panel.hidesOnDeactivate = false
        panel.level = .statusBar
        panel.collectionBehavior = [.canJoinAllSpaces, .transient]
        textValue = text

        content = NSView(frame: NSRect(x: 0, y: 0, width: side, height: side))
        content.wantsLayer = true
        content.layer?.backgroundColor = NSColor.clear.cgColor
        panel.contentView = content

        label = CATextLayer()
        label.alignmentMode = .center
        label.isWrapped = true
        label.truncationMode = .end
        label.string = text
        content.layer?.addSublayer(label)

        super.init()

        applyStyle(fontSize: fontSize, argb: argb, alphaScale: 1.0, fontFamily: fontFamily, emoji: emoji)
        setFrame(x: 0.0, y: 0.0, panelSize: panelSize)
    }

    private func updateLabelLayout() {
        let safeSide = max(1.0, panelSide)
        let maxWidth = max(8.0, safeSide * 0.94)
        let maxHeight = max(8.0, safeSide * 0.92)
        let attributed = NSAttributedString(
            string: textValue,
            attributes: [.font: currentFont]
        )
        var textBounds = attributed.boundingRect(
            with: CGSize(width: maxWidth, height: maxHeight),
            options: [.usesLineFragmentOrigin, .usesFontLeading],
            context: nil
        ).integral
        if textBounds.width <= 0.0 {
            textBounds.size.width = currentFont.pointSize
        }
        if textBounds.height <= 0.0 {
            textBounds.size.height = max(10.0, currentFont.ascender - currentFont.descender)
        }
        let fontSize = max(6.0, currentFont.pointSize)
        let labelWidth = mfxClamp(
            textBounds.width + fontSize * 0.16,
            min: max(10.0, fontSize * 0.9),
            max: maxWidth
        )
        let labelHeight = mfxClamp(
            textBounds.height + fontSize * 0.18,
            min: max(10.0, fontSize * 1.05),
            max: maxHeight
        )
        label.frame = CGRect(
            x: (safeSide - labelWidth) * 0.5,
            y: (safeSide - labelHeight) * 0.5,
            width: labelWidth,
            height: labelHeight
        )
        label.contentsScale = max(1.0, panel.backingScaleFactor)
    }

    func setFrame(x: Double, y: Double, panelSize: Double) {
        let side = CGFloat(max(1.0, panelSize))
        panelSide = side
        panel.setFrame(NSRect(x: x, y: y, width: side, height: side), display: false)
        updateLabelLayout()
    }

    func applyStyle(
        fontSize: Double,
        argb: UInt32,
        alphaScale: Double,
        fontFamily: String,
        emoji: Bool
    ) {
        let clampedSize = CGFloat(max(6.0, fontSize))
        currentFont = mfxResolveLabelFont(fontFamily, clampedSize, emoji)
        label.fontSize = clampedSize
        label.font = currentFont.fontName as CFTypeRef
        label.foregroundColor = mfxColorFromArgb(argb, alphaScale).cgColor
        updateLabelLayout()
    }

    func show() {
        panel.orderFrontRegardless()
    }

    func close() {
        panel.orderOut(nil)
        label.removeFromSuperlayer()
        panel.close()
    }
}

@MainActor
private func mfxCreateTextPanelOnMainThread(
    text: String,
    panelSize: Double,
    fontSize: Double,
    argb: UInt32,
    fontFamily: String,
    emoji: Bool
) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)
    let handle = MfxTextPanelHandle(
        text: text,
        panelSize: panelSize,
        fontSize: fontSize,
        argb: argb,
        fontFamily: fontFamily,
        emoji: emoji
    )
    return Unmanaged.passRetained(handle).toOpaque()
}

@MainActor
private func mfxReleaseTextPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxTextPanelHandle>.fromOpaque(ptr).takeRetainedValue()
    handle.close()
}

@MainActor
private func mfxShowTextPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxTextPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.show()
}

@MainActor
private func mfxSetTextPanelFrameOnMainThread(
    _ panelHandleBits: UInt,
    _ x: Double,
    _ y: Double,
    _ panelSize: Double
) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxTextPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.setFrame(x: x, y: y, panelSize: panelSize)
}

@MainActor
private func mfxApplyTextPanelStyleOnMainThread(
    _ panelHandleBits: UInt,
    _ fontSize: Double,
    _ argb: UInt32,
    _ alphaScale: Double,
    _ fontFamily: String,
    _ emoji: Bool
) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxTextPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.applyStyle(
        fontSize: fontSize,
        argb: argb,
        alphaScale: alphaScale,
        fontFamily: fontFamily,
        emoji: emoji
    )
}

@_cdecl("mfx_macos_text_panel_create_v1")
public func mfx_macos_text_panel_create_v1(
    _ textUtf8: UnsafePointer<CChar>?,
    _ panelSize: Double,
    _ fontSize: Double,
    _ argb: UInt32,
    _ fontFamilyUtf8: UnsafePointer<CChar>?,
    _ emojiText: Int32
) -> UnsafeMutableRawPointer? {
    let text = textUtf8.map(String.init(cString:)) ?? ""
    let fontFamily = fontFamilyUtf8.map(String.init(cString:)) ?? ""
    let emoji = emojiText != 0

    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTextPanelOnMainThread(
                    text: text,
                    panelSize: panelSize,
                    fontSize: fontSize,
                    argb: argb,
                    fontFamily: fontFamily,
                    emoji: emoji
                )
            )
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTextPanelOnMainThread(
                    text: text,
                    panelSize: panelSize,
                    fontSize: fontSize,
                    argb: argb,
                    fontFamily: fontFamily,
                    emoji: emoji
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_text_panel_release_v1")
public func mfx_macos_text_panel_release_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseTextPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxReleaseTextPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_text_panel_show_v1")
public func mfx_macos_text_panel_show_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxShowTextPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxShowTextPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_text_panel_set_frame_v1")
public func mfx_macos_text_panel_set_frame_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ x: Double,
    _ y: Double,
    _ panelSize: Double
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxSetTextPanelFrameOnMainThread(panelHandleBits, x, y, panelSize)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxSetTextPanelFrameOnMainThread(panelHandleBits, x, y, panelSize)
        }
    }
}

@_cdecl("mfx_macos_text_panel_apply_style_v1")
public func mfx_macos_text_panel_apply_style_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ fontSize: Double,
    _ argb: UInt32,
    _ alphaScale: Double,
    _ fontFamilyUtf8: UnsafePointer<CChar>?,
    _ emojiText: Int32
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }
    let fontFamily = fontFamilyUtf8.map(String.init(cString:)) ?? ""
    let emoji = emojiText != 0

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxApplyTextPanelStyleOnMainThread(
                panelHandleBits,
                fontSize,
                argb,
                alphaScale,
                fontFamily,
                emoji
            )
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxApplyTextPanelStyleOnMainThread(
                panelHandleBits,
                fontSize,
                argb,
                alphaScale,
                fontFamily,
                emoji
            )
        }
    }
}
