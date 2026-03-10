@preconcurrency import AppKit
@preconcurrency import Foundation
@preconcurrency import QuartzCore

private enum MfxInputIndicatorVisualKind {
    case pointerLeft
    case pointerRight
    case pointerMiddle
    case wheelUp
    case wheelDown
    case keyboard
}

private func mfxIndicatorVisualKind(for text: String) -> MfxInputIndicatorVisualKind {
    if text.hasPrefix("L") {
        return .pointerLeft
    }
    if text.hasPrefix("R") {
        return .pointerRight
    }
    if text.hasPrefix("M") {
        return .pointerMiddle
    }
    if text.hasPrefix("W+") {
        return .wheelUp
    }
    if text.hasPrefix("W-") {
        return .wheelDown
    }
    return .keyboard
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

private func mfxRoundedRectPath(_ rect: NSRect, radius: CGFloat) -> NSBezierPath {
    let maxRadius = min(rect.width, rect.height) * 0.5
    return NSBezierPath(roundedRect: rect, xRadius: min(radius, maxRadius), yRadius: min(radius, maxRadius))
}

private struct MfxIndicatorAnimParams {
    let t: CGFloat
    let life: CGFloat
    let pulse: CGFloat
    let eased: CGFloat
    let alpha: CGFloat
    let hotAlpha: CGFloat
}

private func mfxEaseOutCubic(_ t: CGFloat) -> CGFloat {
    let x = mfxClamp(t, min: 0.0, max: 1.0)
    let inv = 1.0 - x
    return 1.0 - inv * inv * inv
}

private func mfxIndicatorAnimParams(for progress: CGFloat) -> MfxIndicatorAnimParams {
    let t = mfxClamp(progress, min: 0.0, max: 1.0)
    let life = 1.0 - t
    let pulse = 0.90 + 0.10 * sin(t * .pi * 4.0)
    let eased = mfxEaseOutCubic(t)
    let alpha = 230.0 * life
    let hotAlpha = min(255.0, 170.0 * life * pulse)
    return MfxIndicatorAnimParams(t: t, life: life, pulse: pulse, eased: eased, alpha: alpha, hotAlpha: hotAlpha)
}

private func mfxMeasureLabelWidth(_ text: String, font: NSFont) -> CGFloat {
    let attributes: [NSAttributedString.Key: Any] = [.font: font]
    return ceil((text as NSString).size(withAttributes: attributes).width)
}

private func mfxResolveIndicatorFrameSize(sizePx: Int, text: String) -> CGSize {
    let side = CGFloat(max(1, sizePx))
    if mfxIndicatorVisualKind(for: text) != .keyboard {
        return CGSize(width: side, height: side)
    }

    let font = NSFont.systemFont(ofSize: side * 0.14, weight: .bold)
    let measuredWidth = mfxMeasureLabelWidth(text.isEmpty ? "Key" : text, font: font)
    let desiredPanelWidth = measuredWidth + 18.0
    let desiredWindowWidth = desiredPanelWidth / 0.84
    let width = mfxClamp(ceil(desiredWindowWidth), min: side, max: side * 12.0)
    return CGSize(width: width, height: side)
}

@MainActor
private final class MfxInputIndicatorPanelView: NSView {
    private var indicatorText: String = ""
    private var indicatorSizePx: Int = 72
    private var animationStart: CFTimeInterval = 0
    private var animationDuration: CFTimeInterval = 0.6
    private var animationTimer: Timer?

    override var isOpaque: Bool {
        false
    }

    func configure(sizePx: Int, text: String, durationMs: Int) {
        indicatorSizePx = max(1, sizePx)
        indicatorText = text
        animationStart = CACurrentMediaTime()
        animationDuration = max(0.12, Double(max(0, durationMs)) / 1000.0)
        frame = NSRect(origin: .zero, size: MfxInputIndicatorPanelView.desiredFrameSize(sizePx: sizePx, text: text))
        startAnimationTimer()
        needsDisplay = true
    }

    func stopAnimation() {
        animationTimer?.invalidate()
        animationTimer = nil
    }

    static func desiredFrameSize(sizePx: Int, text: String) -> CGSize {
        mfxResolveIndicatorFrameSize(sizePx: sizePx, text: text)
    }

    override func draw(_ dirtyRect: NSRect) {
        NSColor.clear.setFill()
        dirtyRect.fill()

        let anim = currentAnimParams()
        switch mfxIndicatorVisualKind(for: indicatorText) {
        case .pointerLeft, .pointerRight, .pointerMiddle, .wheelUp, .wheelDown:
            drawPointerIndicator(in: bounds, text: indicatorText, anim: anim)
        case .keyboard:
            drawKeyboardIndicator(in: bounds, text: indicatorText, anim: anim)
        }
    }

    private func drawPointerIndicator(in bounds: NSRect, text: String, anim: MfxIndicatorAnimParams) {
        let size = min(bounds.width, bounds.height)
        let body = NSRect(
            x: bounds.minX + bounds.width * 0.12,
            y: bounds.minY + bounds.height * 0.07,
            width: bounds.width * 0.76,
            height: bounds.height * 0.86)
        let radius = size * 0.19
        let bodyPath = mfxRoundedRectPath(body, radius: radius)
        let shadowPath = mfxRoundedRectPath(body.offsetBy(dx: 2.0, dy: -3.0), radius: radius)

        NSGraphicsContext.saveGraphicsState()
        let shadowAlpha = 0.28 * (anim.alpha / 230.0)
        NSColor(calibratedWhite: 0.0, alpha: shadowAlpha).setFill()
        shadowPath.fill()
        NSGraphicsContext.restoreGraphicsState()

        let baseAlpha = anim.alpha / 230.0
        let bodyGradient = NSGradient(colors: [
            NSColor(calibratedRed: 28.0 / 255.0, green: 44.0 / 255.0, blue: 72.0 / 255.0, alpha: 0.92 * baseAlpha),
            NSColor(calibratedRed: 8.0 / 255.0, green: 15.0 / 255.0, blue: 31.0 / 255.0, alpha: 0.92 * baseAlpha),
        ])
        bodyGradient?.draw(in: bodyPath, angle: -90.0)
        NSColor(calibratedRed: 182.0 / 255.0, green: 205.0 / 255.0, blue: 235.0 / 255.0, alpha: 0.96 * baseAlpha).setStroke()
        bodyPath.lineWidth = 1.8
        bodyPath.stroke()

        let splitX = body.minX + body.width * 0.5
        let topY = body.minY + body.height * 0.66
        let leftTop = NSRect(x: body.minX, y: topY, width: body.width * 0.5, height: body.maxY - topY)
        let rightTop = NSRect(x: splitX, y: topY, width: body.width * 0.5, height: body.maxY - topY)

        let kind = mfxIndicatorVisualKind(for: text)
        if kind == .pointerLeft {
            drawButtonHighlight(clipPath: bodyPath, region: leftTop,
                                top: NSColor(calibratedRed: 74.0 / 255.0, green: 160.0 / 255.0, blue: 1.0, alpha: 0.78 * (anim.hotAlpha / 255.0)),
                                bottom: NSColor(calibratedRed: 52.0 / 255.0, green: 98.0 / 255.0, blue: 178.0 / 255.0, alpha: 0.42 * (anim.hotAlpha / 255.0)))
            drawRipple(in: leftTop, anim: anim)
        }
        if kind == .pointerRight {
            drawButtonHighlight(clipPath: bodyPath, region: rightTop,
                                top: NSColor(calibratedRed: 1.0, green: 150.0 / 255.0, blue: 92.0 / 255.0, alpha: 0.76 * (anim.hotAlpha / 255.0)),
                                bottom: NSColor(calibratedRed: 182.0 / 255.0, green: 94.0 / 255.0, blue: 54.0 / 255.0, alpha: 0.40 * (anim.hotAlpha / 255.0)))
            drawRipple(in: rightTop, anim: anim)
        }

        NSColor(calibratedRed: 114.0 / 255.0, green: 140.0 / 255.0, blue: 172.0 / 255.0, alpha: 0.92 * baseAlpha).setStroke()
        let separator = NSBezierPath()
        separator.lineWidth = 1.1
        separator.move(to: NSPoint(x: splitX, y: body.maxY - 2.0))
        separator.line(to: NSPoint(x: splitX, y: topY))
        separator.move(to: NSPoint(x: body.minX + 2.0, y: topY))
        separator.line(to: NSPoint(x: body.maxX - 2.0, y: topY))
        separator.stroke()

        let wheelRect = NSRect(
            x: splitX - body.width * 0.078,
            y: body.minY + body.height * 0.70,
            width: body.width * 0.156,
            height: body.height * 0.205)
        drawWheel(in: wheelRect, active: kind == .pointerMiddle || kind == .wheelUp || kind == .wheelDown, anim: anim)

        let labelRect = NSRect(
            x: body.minX,
            y: body.minY + size * 0.02,
            width: body.width,
            height: size * 0.23)
        drawCenteredText(text, in: labelRect, font: NSFont.systemFont(ofSize: size * 0.17, weight: .bold), alphaScale: baseAlpha)

        if kind == .wheelUp || kind == .wheelDown {
            drawWheelArrow(in: body, up: kind == .wheelUp, anim: anim)
        }
        if kind == .pointerMiddle {
            drawRipple(in: wheelRect, anim: anim)
        }
    }

    private func drawKeyboardIndicator(in bounds: NSRect, text: String, anim: MfxIndicatorAnimParams) {
        let unit = min(bounds.width, bounds.height)
        let panel = NSRect(
            x: bounds.width * 0.08,
            y: bounds.height * 0.31,
            width: bounds.width * 0.84,
            height: bounds.height * 0.38)

        NSGraphicsContext.saveGraphicsState()
        let baseAlpha = anim.alpha / 230.0
        NSColor(calibratedWhite: 0.0, alpha: 0.28 * baseAlpha).setFill()
        mfxRoundedRectPath(panel.offsetBy(dx: 2.0, dy: -3.0), radius: unit * 0.11).fill()
        NSGraphicsContext.restoreGraphicsState()

        let path = mfxRoundedRectPath(panel, radius: unit * 0.11)
        let gradient = NSGradient(colors: [
            NSColor(calibratedRed: 43.0 / 255.0, green: 66.0 / 255.0, blue: 103.0 / 255.0, alpha: 0.92 * baseAlpha),
            NSColor(calibratedRed: 17.0 / 255.0, green: 30.0 / 255.0, blue: 52.0 / 255.0, alpha: 0.92 * baseAlpha),
        ])
        gradient?.draw(in: path, angle: -90.0)
        NSColor(calibratedRed: 190.0 / 255.0, green: 222.0 / 255.0, blue: 1.0, alpha: 0.96 * baseAlpha).setStroke()
        path.lineWidth = 1.6
        path.stroke()

        drawCenteredText(
            text.isEmpty ? "Key" : text,
            in: panel,
            font: NSFont.systemFont(ofSize: bounds.height * 0.14, weight: .bold),
            alphaScale: baseAlpha)
    }

    private func drawButtonHighlight(clipPath: NSBezierPath, region: NSRect, top: NSColor, bottom: NSColor) {
        NSGraphicsContext.saveGraphicsState()
        clipPath.addClip()
        let gradient = NSGradient(colors: [top, bottom])
        gradient?.draw(in: region, angle: -90.0)
        NSGraphicsContext.restoreGraphicsState()
    }

    private func drawRipple(in rect: NSRect, anim: MfxIndicatorAnimParams) {
        let cx = rect.midX
        let cy = rect.midY
        let radius = min(rect.width, rect.height) * 0.45 * anim.eased
        let ringRect = NSRect(x: cx - radius, y: cy - radius, width: radius * 2.0, height: radius * 2.0)
        let alpha = 0.34 * anim.life * anim.life
        NSColor(calibratedRed: 200.0 / 255.0, green: 230.0 / 255.0, blue: 1.0, alpha: alpha).setStroke()
        let ring = NSBezierPath(ovalIn: ringRect)
        ring.lineWidth = 1.5
        ring.stroke()
    }

    private func drawWheel(in rect: NSRect, active: Bool, anim: MfxIndicatorAnimParams) {
        let baseAlpha = anim.alpha / 230.0
        NSColor(calibratedRed: 142.0 / 255.0, green: 166.0 / 255.0, blue: 200.0 / 255.0, alpha: 0.92 * baseAlpha).setFill()
        NSBezierPath(ovalIn: rect).fill()
        if active {
            let hotScale = anim.hotAlpha / 255.0
            NSColor(calibratedRed: 108.0 / 255.0, green: 236.0 / 255.0, blue: 1.0, alpha: 0.78 * hotScale).setFill()
            NSBezierPath(ovalIn: rect.insetBy(dx: 0.5, dy: 0.5)).fill()
            NSColor(calibratedRed: 154.0 / 255.0, green: 244.0 / 255.0, blue: 1.0, alpha: 0.92 * hotScale).setStroke()
            let ring = NSBezierPath(ovalIn: rect.insetBy(dx: 0.5, dy: 0.5))
            ring.lineWidth = 1.0
            ring.stroke()
        }
    }

    private func drawWheelArrow(in body: NSRect, up: Bool, anim: MfxIndicatorAnimParams) {
        let size = min(body.width, body.height)
        let cx = body.midX
        let cy = body.minY + body.height * 0.52
        let shift = size * (0.016 + 0.028 * anim.eased)

        let tipY = up ? (cy + shift + size * 0.06) : (cy - shift - size * 0.06)
        let baseY = up ? (cy + shift - size * 0.02) : (cy - shift + size * 0.02)
        let path = NSBezierPath()
        path.move(to: NSPoint(x: cx, y: tipY))
        path.line(to: NSPoint(x: cx - size * 0.045, y: baseY))
        path.line(to: NSPoint(x: cx + size * 0.045, y: baseY))
        path.close()
        let arrowAlpha = min(1.0, (anim.alpha / 230.0) + 0.05)
        NSColor(calibratedRed: 124.0 / 255.0, green: 236.0 / 255.0, blue: 1.0, alpha: 0.95 * arrowAlpha).setFill()
        path.fill()

        let trail = size * 0.035 * anim.eased
        let trailAlpha = 0.35 * (anim.alpha / 230.0)
        let ghost = NSBezierPath()
        if up {
            ghost.move(to: NSPoint(x: cx, y: tipY + trail))
            ghost.line(to: NSPoint(x: cx - size * 0.03, y: baseY + trail))
            ghost.line(to: NSPoint(x: cx + size * 0.03, y: baseY + trail))
        } else {
            ghost.move(to: NSPoint(x: cx, y: tipY - trail))
            ghost.line(to: NSPoint(x: cx - size * 0.03, y: baseY - trail))
            ghost.line(to: NSPoint(x: cx + size * 0.03, y: baseY - trail))
        }
        ghost.close()
        NSColor(calibratedRed: 124.0 / 255.0, green: 236.0 / 255.0, blue: 1.0, alpha: trailAlpha).setFill()
        ghost.fill()
    }

    private func drawCenteredText(_ text: String, in rect: NSRect, font: NSFont, alphaScale: CGFloat) {
        let paragraph = NSMutableParagraphStyle()
        paragraph.alignment = .center
        let attrs: [NSAttributedString.Key: Any] = [
            .font: font,
            .foregroundColor: NSColor(calibratedRed: 244.0 / 255.0, green: 250.0 / 255.0, blue: 1.0, alpha: 0.98 * alphaScale),
            .paragraphStyle: paragraph,
        ]
        let attributed = NSAttributedString(string: text, attributes: attrs)
        let measured = attributed.size()
        let drawRect = NSRect(
            x: rect.minX,
            y: rect.midY - measured.height * 0.5,
            width: rect.width,
            height: measured.height)
        attributed.draw(in: drawRect)
    }

    private func currentAnimParams() -> MfxIndicatorAnimParams {
        if animationDuration <= 0.0 {
            return mfxIndicatorAnimParams(for: 0.0)
        }
        let elapsed = max(0.0, CACurrentMediaTime() - animationStart)
        let progress = min(1.0, elapsed / animationDuration)
        return mfxIndicatorAnimParams(for: CGFloat(progress))
    }

    private func startAnimationTimer() {
        animationTimer?.invalidate()
        animationTimer = Timer.scheduledTimer(
            timeInterval: 1.0 / 60.0,
            target: self,
            selector: #selector(onAnimationTick(_:)),
            userInfo: nil,
            repeats: true)
    }

    @objc private func onAnimationTick(_ timer: Timer) {
        let elapsed = CACurrentMediaTime() - animationStart
        if elapsed >= animationDuration {
            stopAnimation()
            return
        }
        needsDisplay = true
    }
}

@MainActor
private final class MfxInputIndicatorPanelHandle: NSObject {
    private let panel: NSPanel
    private let indicatorView: MfxInputIndicatorPanelView

    init(sizePx: Int) {
        let size = MfxInputIndicatorPanelView.desiredFrameSize(sizePx: sizePx, text: "")
        panel = NSPanel(
            contentRect: NSRect(origin: .zero, size: size),
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

        indicatorView = MfxInputIndicatorPanelView(frame: NSRect(origin: .zero, size: size))
        indicatorView.wantsLayer = true
        panel.contentView = indicatorView
    }

    func present(x: Int, y: Int, sizePx: Int, text: String, durationMs: Int) {
        let frameSize = MfxInputIndicatorPanelView.desiredFrameSize(sizePx: sizePx, text: text)
        indicatorView.configure(sizePx: sizePx, text: text, durationMs: durationMs)
        panel.setFrame(NSRect(x: CGFloat(x), y: CGFloat(y), width: frameSize.width, height: frameSize.height), display: false)
        panel.alphaValue = 1.0
        panel.orderFrontRegardless()
    }

    func hide() {
        indicatorView.stopAnimation()
        panel.orderOut(nil)
    }

    func closeAndCleanup() {
        hide()
        indicatorView.removeFromSuperview()
        panel.close()
    }
}

@MainActor
private func mfxCreateInputIndicatorPanelOnMainThread(_ sizePx: Int) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)
    let handle = MfxInputIndicatorPanelHandle(sizePx: sizePx)
    return Unmanaged.passRetained(handle).toOpaque()
}

@MainActor
private func mfxReleaseInputIndicatorPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxInputIndicatorPanelHandle>.fromOpaque(ptr).takeRetainedValue()
    handle.closeAndCleanup()
}

@MainActor
private func mfxHideInputIndicatorPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxInputIndicatorPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.hide()
}

@MainActor
private func mfxPresentInputIndicatorPanelOnMainThread(
    _ panelHandleBits: UInt,
    _ x: Int,
    _ y: Int,
    _ sizePx: Int,
    _ text: String,
    _ durationMs: Int
) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxInputIndicatorPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    let resolvedDuration = durationMs > 0 ? durationMs : 600
    handle.present(x: x, y: y, sizePx: sizePx, text: text, durationMs: resolvedDuration)
}

@_cdecl("mfx_macos_input_indicator_panel_create_v1")
public func mfx_macos_input_indicator_panel_create_v1(_ sizePx: Int32) -> UnsafeMutableRawPointer? {
    let size = Int(sizePx)
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateInputIndicatorPanelOnMainThread(size))
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateInputIndicatorPanelOnMainThread(size))
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_input_indicator_panel_release_v1")
public func mfx_macos_input_indicator_panel_release_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseInputIndicatorPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxReleaseInputIndicatorPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_input_indicator_panel_hide_v1")
public func mfx_macos_input_indicator_panel_hide_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxHideInputIndicatorPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxHideInputIndicatorPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_input_indicator_panel_present_v1")
public func mfx_macos_input_indicator_panel_present_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32,
    _ sizePx: Int32,
    _ textUtf8: UnsafePointer<CChar>?
) {
    mfx_macos_input_indicator_panel_present_v2(panelHandle, x, y, sizePx, textUtf8, 0)
}

@_cdecl("mfx_macos_input_indicator_panel_present_v2")
public func mfx_macos_input_indicator_panel_present_v2(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32,
    _ sizePx: Int32,
    _ textUtf8: UnsafePointer<CChar>?,
    _ durationMs: Int32
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    let text = textUtf8.map(String.init(cString:)) ?? ""
    let xInt = Int(x)
    let yInt = Int(y)
    let sizeInt = Int(sizePx)

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxPresentInputIndicatorPanelOnMainThread(panelHandleBits, xInt, yInt, sizeInt, text, Int(durationMs))
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxPresentInputIndicatorPanelOnMainThread(panelHandleBits, xInt, yInt, sizeInt, text, Int(durationMs))
        }
    }
}
