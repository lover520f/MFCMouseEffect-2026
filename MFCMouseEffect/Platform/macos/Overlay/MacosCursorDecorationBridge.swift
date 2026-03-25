@preconcurrency import AppKit
@preconcurrency import Foundation

private func mfxCursorDecorationClamp(_ value: CGFloat, min minValue: CGFloat, max maxValue: CGFloat) -> CGFloat {
    if value < minValue {
        return minValue
    }
    if value > maxValue {
        return maxValue
    }
    return value
}

private struct MfxCursorDecorationLayout {
    let frameSize: CGSize
}

private func mfxCursorDecorationLayout(pluginId: String, sizePx: Int) -> MfxCursorDecorationLayout {
    let size = max(12, min(72, sizePx))
    let side = max(36, size * 4)
    return MfxCursorDecorationLayout(frameSize: CGSize(width: side, height: side))
}

private func mfxCursorDecorationColor(from hex: String, alphaPercent: Int) -> NSColor {
    let safe = hex.count == 7 && hex.hasPrefix("#") ? hex : "#ff5a5a"
    let start = safe.index(after: safe.startIndex)
    let r = Int(safe[start...safe.index(start, offsetBy: 1)], radix: 16) ?? 255
    let gStart = safe.index(start, offsetBy: 2)
    let g = Int(safe[gStart...safe.index(gStart, offsetBy: 1)], radix: 16) ?? 90
    let bStart = safe.index(start, offsetBy: 4)
    let b = Int(safe[bStart...safe.index(bStart, offsetBy: 1)], radix: 16) ?? 90
    let alpha = CGFloat(max(15, min(100, alphaPercent))) / 100.0
    return NSColor(
        calibratedRed: CGFloat(r) / 255.0,
        green: CGFloat(g) / 255.0,
        blue: CGFloat(b) / 255.0,
        alpha: alpha
    )
}

private func mfxCursorDecorationAlpha(_ color: NSColor, scale: CGFloat) -> NSColor {
    let rgba = color.usingColorSpace(.deviceRGB) ?? color
    return NSColor(
        calibratedRed: rgba.redComponent,
        green: rgba.greenComponent,
        blue: rgba.blueComponent,
        alpha: mfxCursorDecorationClamp(rgba.alphaComponent * scale, min: 0.0, max: 1.0)
    )
}

@MainActor
private final class MfxCursorDecorationView: NSView {
    private var pluginId: String = "focus_ring"
    private var colorHex: String = "#ff5a5a"
    private var sizePx: Int = 22
    private var alphaPercent: Int = 82

    override var isOpaque: Bool {
        false
    }

    func configure(pluginId: String, colorHex: String, sizePx: Int, alphaPercent: Int) {
        self.pluginId = pluginId.isEmpty ? "focus_ring" : pluginId
        self.colorHex = colorHex.isEmpty ? "#ff5a5a" : colorHex
        self.sizePx = max(12, min(72, sizePx))
        self.alphaPercent = max(15, min(100, alphaPercent))
        frame = NSRect(origin: .zero, size: Self.desiredFrameSize(pluginId: self.pluginId, sizePx: self.sizePx))
        needsDisplay = true
    }

    static func desiredFrameSize(pluginId: String, sizePx: Int) -> CGSize {
        mfxCursorDecorationLayout(pluginId: pluginId, sizePx: sizePx).frameSize
    }

    override func draw(_ dirtyRect: NSRect) {
        NSColor.clear.setFill()
        dirtyRect.fill()

        let accent = mfxCursorDecorationColor(from: colorHex, alphaPercent: alphaPercent)
        switch pluginId {
        case "halo_orb":
            drawHaloOrb(accent: accent)
        case "soft_orb":
            drawOrb(accent: accent)
        case "signal_ring":
            drawSignalRing(accent: accent)
        default:
            drawRing(accent: accent)
        }
    }

    private func drawRing(accent: NSColor) {
        let rect = bounds.insetBy(dx: bounds.width * 0.22, dy: bounds.height * 0.22)
        let outerGlow = rect.insetBy(dx: -rect.width * 0.22, dy: -rect.height * 0.22)
        mfxCursorDecorationAlpha(accent, scale: 0.12).setFill()
        NSBezierPath(ovalIn: outerGlow).fill()

        mfxCursorDecorationAlpha(accent, scale: 0.86).setStroke()
        let ring = NSBezierPath(ovalIn: rect)
        ring.lineWidth = max(2.0, CGFloat(sizePx) * 0.14)
        ring.stroke()

        let inner = rect.insetBy(dx: rect.width * 0.26, dy: rect.height * 0.26)
        mfxCursorDecorationAlpha(accent, scale: 0.36).setStroke()
        let innerRing = NSBezierPath(ovalIn: inner)
        innerRing.lineWidth = max(1.0, CGFloat(sizePx) * 0.06)
        innerRing.stroke()

        mfxCursorDecorationAlpha(accent, scale: 0.20).setFill()
        NSBezierPath(ovalIn: inner.insetBy(dx: inner.width * 0.22, dy: inner.height * 0.22)).fill()
    }

    private func drawSignalRing(accent: NSColor) {
        let rect = bounds.insetBy(dx: bounds.width * 0.22, dy: bounds.height * 0.22)
        let outerGlow = rect.insetBy(dx: -rect.width * 0.30, dy: -rect.height * 0.30)
        mfxCursorDecorationAlpha(accent, scale: 0.18).setFill()
        NSBezierPath(ovalIn: outerGlow).fill()

        mfxCursorDecorationAlpha(accent, scale: 0.96).setStroke()
        let outerRing = NSBezierPath(ovalIn: rect)
        outerRing.lineWidth = max(2.4, CGFloat(sizePx) * 0.16)
        outerRing.stroke()

        let innerRingRect = rect.insetBy(dx: rect.width * 0.14, dy: rect.height * 0.14)
        mfxCursorDecorationAlpha(accent, scale: 0.60).setStroke()
        let innerRing = NSBezierPath(ovalIn: innerRingRect)
        innerRing.lineWidth = max(1.3, CGFloat(sizePx) * 0.08)
        innerRing.stroke()

        mfxCursorDecorationAlpha(.white, scale: 0.94).setFill()
        let core = rect.insetBy(dx: rect.width * 0.38, dy: rect.height * 0.38)
        NSBezierPath(ovalIn: core).fill()
    }

    private func drawOrb(accent: NSColor) {
        let glow = bounds.insetBy(dx: bounds.width * 0.18, dy: bounds.height * 0.18)
        mfxCursorDecorationAlpha(accent, scale: 0.10).setFill()
        NSBezierPath(ovalIn: glow.insetBy(dx: -glow.width * 0.22, dy: -glow.height * 0.22)).fill()

        let orb = bounds.insetBy(dx: bounds.width * 0.28, dy: bounds.height * 0.28)
        mfxCursorDecorationAlpha(accent, scale: 0.82).setFill()
        NSBezierPath(ovalIn: orb).fill()

        mfxCursorDecorationAlpha(.white, scale: 0.92).setFill()
        let core = orb.insetBy(dx: orb.width * 0.34, dy: orb.height * 0.34)
        NSBezierPath(ovalIn: core).fill()
    }

    private func drawHaloOrb(accent: NSColor) {
        let glow = bounds.insetBy(dx: bounds.width * 0.12, dy: bounds.height * 0.12)
        mfxCursorDecorationAlpha(accent, scale: 0.12).setFill()
        NSBezierPath(ovalIn: glow.insetBy(dx: -glow.width * 0.20, dy: -glow.height * 0.20)).fill()

        let orb = bounds.insetBy(dx: bounds.width * 0.24, dy: bounds.height * 0.24)
        mfxCursorDecorationAlpha(accent, scale: 0.54).setFill()
        NSBezierPath(ovalIn: orb).fill()

        mfxCursorDecorationAlpha(accent, scale: 0.48).setStroke()
        let halo = NSBezierPath(ovalIn: orb.insetBy(dx: orb.width * 0.08, dy: orb.height * 0.08))
        halo.lineWidth = max(1.2, CGFloat(sizePx) * 0.07)
        halo.stroke()

        mfxCursorDecorationAlpha(.white, scale: 0.92).setFill()
        let core = orb.insetBy(dx: orb.width * 0.38, dy: orb.height * 0.38)
        NSBezierPath(ovalIn: core).fill()
    }

}

@MainActor
private final class MfxCursorDecorationPanelHandle: NSObject {
    private let panel: NSPanel
    private let decorationView: MfxCursorDecorationView

    override init() {
        let initialSize = MfxCursorDecorationView.desiredFrameSize(pluginId: "focus_ring", sizePx: 22)
        panel = NSPanel(
            contentRect: NSRect(origin: .zero, size: initialSize),
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

        decorationView = MfxCursorDecorationView(frame: NSRect(origin: .zero, size: initialSize))
        decorationView.wantsLayer = true
        panel.contentView = decorationView
    }

    func present(
        x: Int,
        y: Int,
        pluginId: String,
        colorHex: String,
        sizePx: Int,
        alphaPercent: Int
    ) {
        let size = MfxCursorDecorationView.desiredFrameSize(pluginId: pluginId, sizePx: sizePx)
        decorationView.configure(pluginId: pluginId, colorHex: colorHex, sizePx: sizePx, alphaPercent: alphaPercent)
        panel.setFrame(NSRect(x: CGFloat(x), y: CGFloat(y), width: size.width, height: size.height), display: false)
        panel.alphaValue = 1.0
        panel.orderFrontRegardless()
    }

    func hide() {
        panel.orderOut(nil)
    }

    func closeAndCleanup() {
        hide()
        decorationView.removeFromSuperview()
        panel.close()
    }
}

@MainActor
private func mfxCreateCursorDecorationPanelOnMainThread() -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)
    return Unmanaged.passRetained(MfxCursorDecorationPanelHandle()).toOpaque()
}

@MainActor
private func mfxReleaseCursorDecorationPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0,
          let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxCursorDecorationPanelHandle>.fromOpaque(ptr).takeRetainedValue()
    handle.closeAndCleanup()
}

@MainActor
private func mfxHideCursorDecorationPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0,
          let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxCursorDecorationPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.hide()
}

@MainActor
private func mfxPresentCursorDecorationPanelOnMainThread(
    _ panelHandleBits: UInt,
    _ x: Int,
    _ y: Int,
    _ pluginId: String,
    _ colorHex: String,
    _ sizePx: Int,
    _ alphaPercent: Int
) {
    guard panelHandleBits != 0,
          let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxCursorDecorationPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.present(
        x: x,
        y: y,
        pluginId: pluginId,
        colorHex: colorHex,
        sizePx: sizePx,
        alphaPercent: alphaPercent
    )
}

@_cdecl("mfx_macos_cursor_decoration_panel_create_v1")
public func mfx_macos_cursor_decoration_panel_create_v1() -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateCursorDecorationPanelOnMainThread())
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateCursorDecorationPanelOnMainThread())
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_cursor_decoration_panel_release_v1")
public func mfx_macos_cursor_decoration_panel_release_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseCursorDecorationPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxReleaseCursorDecorationPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_cursor_decoration_panel_hide_v1")
public func mfx_macos_cursor_decoration_panel_hide_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxHideCursorDecorationPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxHideCursorDecorationPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_cursor_decoration_panel_present_v1")
public func mfx_macos_cursor_decoration_panel_present_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32,
    _ pluginIdUtf8: UnsafePointer<CChar>?,
    _ colorHexUtf8: UnsafePointer<CChar>?,
    _ sizePx: Int32,
    _ alphaPercent: Int32
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    let pluginId = pluginIdUtf8.map(String.init(cString:)) ?? "focus_ring"
    let colorHex = colorHexUtf8.map(String.init(cString:)) ?? "#ff5a5a"
    let xInt = Int(x)
    let yInt = Int(y)
    let sizeInt = Int(sizePx)
    let alphaInt = Int(alphaPercent)

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxPresentCursorDecorationPanelOnMainThread(
                panelHandleBits,
                xInt,
                yInt,
                pluginId,
                colorHex,
                sizeInt,
                alphaInt
            )
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxPresentCursorDecorationPanelOnMainThread(
                panelHandleBits,
                xInt,
                yInt,
                pluginId,
                colorHex,
                sizeInt,
                alphaInt
            )
        }
    }
}
