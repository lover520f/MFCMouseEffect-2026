@preconcurrency import AppKit
@preconcurrency import QuartzCore
@preconcurrency import Foundation

@_silgen_name("mfx_macos_overlay_create_window_v1")
private func mfxOverlayCreateWindow(
    _ x: Double,
    _ y: Double,
    _ width: Double,
    _ height: Double
) -> UnsafeMutableRawPointer?

@_silgen_name("mfx_macos_overlay_release_window_v1")
private func mfxOverlayReleaseWindow(_ windowHandle: UnsafeMutableRawPointer?)

@_silgen_name("mfx_macos_overlay_show_window_v1")
private func mfxOverlayShowWindow(_ windowHandle: UnsafeMutableRawPointer?)

@_silgen_name("mfx_macos_overlay_resolve_screen_frame_v1")
private func mfxOverlayResolveScreenFrame(
    _ x: Int32,
    _ y: Int32,
    _ outX: UnsafeMutablePointer<Double>?,
    _ outY: UnsafeMutablePointer<Double>?,
    _ outWidth: UnsafeMutablePointer<Double>?,
    _ outHeight: UnsafeMutablePointer<Double>?
) -> Int32

@_silgen_name("mfx_macos_overlay_apply_content_scale_v1")
private func mfxOverlayApplyContentScale(
    _ contentHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32
)

@MainActor
private final class MfxLineTrailState: NSObject {
    private struct Point {
        var x: Int32
        var y: Int32
        var tickMs: UInt64
    }

    private struct Config {
        var durationMs: Int = 300
        var lineWidth: CGFloat = 4.0
        var strokeArgb: UInt32 = 0xFFFFFFFF
        var idleFadeStartMs: Int = 60
        var idleFadeEndMs: Int = 220
    }

    private var windowHandle: UnsafeMutableRawPointer?
    private var window: NSWindow?
    private var containerLayer: CALayer?
    private var windowOriginX: Int32 = 0
    private var windowOriginY: Int32 = 0
    private var windowWidth: CGFloat = 0.0
    private var windowHeight: CGFloat = 0.0
    private var points: [Point] = []
    private var config = Config()
    private var timer: DispatchSourceTimer?
    private var lastInputMs: UInt64 = 0

    private static func nowMs() -> UInt64 {
        let ms = ProcessInfo.processInfo.systemUptime * 1000.0
        return UInt64(ms.rounded(.down))
    }

    private static func colorFromArgb(_ argb: UInt32, _ alphaScale: Double) -> NSColor {
        let baseAlpha = Double((argb >> 24) & 0xFF) / 255.0
        let alpha = CGFloat(max(0.0, min(1.0, baseAlpha * alphaScale)))
        let red = CGFloat(Double((argb >> 16) & 0xFF) / 255.0)
        let green = CGFloat(Double((argb >> 8) & 0xFF) / 255.0)
        let blue = CGFloat(Double(argb & 0xFF) / 255.0)
        return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
    }

    private static func resolveScreenFrame(forX x: Int32, y: Int32) -> NSRect? {
        var originX = 0.0
        var originY = 0.0
        var width = 0.0
        var height = 0.0
        let ok = mfxOverlayResolveScreenFrame(
            x,
            y,
            &originX,
            &originY,
            &width,
            &height
        )
        if ok == 0 || width <= 0.0 || height <= 0.0 {
            return nil
        }
        return NSRect(x: originX, y: originY, width: width, height: height)
    }

    private static func idleFadeFactor(nowMs: UInt64, lastPointMs: UInt64, startMs: Int, endMs: Int) -> Double {
        let safeStart = max(0, startMs)
        let safeEnd = max(safeStart + 1, endMs)
        let elapsed = nowMs > lastPointMs ? nowMs - lastPointMs : 0
        if elapsed <= UInt64(safeStart) {
            return 1.0
        }
        if elapsed >= UInt64(safeEnd) {
            return 0.0
        }
        let t = Double(elapsed - UInt64(safeStart)) / Double(safeEnd - safeStart)
        return max(0.0, 1.0 - t)
    }

    private func stopTimer() {
        guard let timer else {
            return
        }
        timer.cancel()
        self.timer = nil
    }

    private func closeWindow() {
        guard let windowHandle else {
            return
        }
        mfxOverlayReleaseWindow(windowHandle)
        self.windowHandle = nil
        self.window = nil
        self.containerLayer = nil
        self.windowWidth = 0.0
        self.windowHeight = 0.0
    }

    private func clearSegmentSublayers() {
        guard let containerLayer else {
            return
        }
        containerLayer.sublayers?.forEach { layer in
            layer.removeFromSuperlayer()
        }
    }

    private func resetStateOnMain() {
        stopTimer()
        closeWindow()
        points.removeAll(keepingCapacity: false)
        lastInputMs = 0
    }

    private func ensureWindow(forX x: Int32, y: Int32) -> Bool {
        guard let frame = Self.resolveScreenFrame(forX: x, y: y) else {
            return false
        }

        let needsNewWindow =
            (window == nil) ||
            abs(Double(windowOriginX) - frame.origin.x) > 0.5 ||
            abs(Double(windowOriginY) - frame.origin.y) > 0.5 ||
            abs(Double(windowWidth) - frame.size.width) > 0.5 ||
            abs(Double(windowHeight) - frame.size.height) > 0.5

        if !needsNewWindow {
            return true
        }

        closeWindow()

        guard let newWindowHandle = mfxOverlayCreateWindow(
            Double(frame.origin.x),
            Double(frame.origin.y),
            Double(frame.size.width),
            Double(frame.size.height)
        ) else {
            return false
        }

        let newWindow = Unmanaged<NSWindow>.fromOpaque(newWindowHandle).takeUnretainedValue()

        guard let content = newWindow.contentView else {
            mfxOverlayReleaseWindow(newWindowHandle)
            return false
        }
        content.wantsLayer = true
        let contentHandle = Unmanaged.passUnretained(content).toOpaque()
        mfxOverlayApplyContentScale(contentHandle, x, y)
        let scale = max(CGFloat(1.0), newWindow.backingScaleFactor)

        let container = CALayer()
        container.frame = content.bounds
        container.contentsScale = scale
        content.layer?.addSublayer(container)

        windowHandle = newWindowHandle
        window = newWindow
        containerLayer = container
        windowOriginX = Int32(frame.origin.x.rounded())
        windowOriginY = Int32(frame.origin.y.rounded())
        windowWidth = frame.size.width
        windowHeight = frame.size.height
        mfxOverlayShowWindow(newWindowHandle)
        return true
    }

    private func rebuildPath(nowMs: UInt64) {
        guard let containerLayer else {
            return
        }

        clearSegmentSublayers()
        if points.isEmpty {
            return
        }

        let idleFactor = Self.idleFadeFactor(
            nowMs: nowMs,
            lastPointMs: points[points.count - 1].tickMs,
            startMs: config.idleFadeStartMs,
            endMs: config.idleFadeEndMs
        )
        let durationMs = max(1, config.durationMs)
        let lineWidth = max(0.2, config.lineWidth)

        CATransaction.begin()
        CATransaction.setDisableActions(true)

        let fullAlphaStroke = config.strokeArgb | 0xFF000000

        if points.count == 1 {
            let point = points[0]
            let x = CGFloat(point.x - windowOriginX)
            let y = CGFloat(point.y - windowOriginY)
            let radius = max(lineWidth * 0.6, 2.0)

            let age = Float(nowMs - point.tickMs)
            let life = max(0.0, 1.0 - age / Float(durationMs))
            let opacity = max(0.0, min(1.0, Double(life) * idleFactor))

            let dot = CAShapeLayer()
            dot.frame = containerLayer.bounds
            dot.path = CGPath(
                ellipseIn: CGRect(x: x - radius, y: y - radius, width: radius * 2.0, height: radius * 2.0),
                transform: nil
            )
            dot.fillColor = Self.colorFromArgb(fullAlphaStroke, 1.0).cgColor
            dot.strokeColor = NSColor.clear.cgColor
            dot.opacity = Float(opacity)
            containerLayer.addSublayer(dot)

            CATransaction.commit()
            return
        }

        let pathLayer = CAShapeLayer()
        pathLayer.frame = containerLayer.bounds

        let path = CGMutablePath()
        var started = false
        for point in points {
            let x = CGFloat(point.x - windowOriginX)
            let y = CGFloat(point.y - windowOriginY)
            if !started {
                path.move(to: CGPoint(x: x, y: y))
                started = true
            } else {
                path.addLine(to: CGPoint(x: x, y: y))
            }
        }
        pathLayer.path = path
        pathLayer.strokeColor = Self.colorFromArgb(fullAlphaStroke, 1.0).cgColor
        pathLayer.fillColor = NSColor.clear.cgColor
        pathLayer.lineWidth = lineWidth
        pathLayer.lineCap = .round
        pathLayer.lineJoin = .round

        let newestAge = Float(nowMs - points[points.count - 1].tickMs)
        let newestLife = max(0.0, 1.0 - newestAge / Float(durationMs))
        pathLayer.opacity = Float(max(0.0, min(1.0, Double(newestLife) * idleFactor)))
        pathLayer.strokeStart = 0.0
        pathLayer.strokeEnd = 1.0

        containerLayer.addSublayer(pathLayer)
        CATransaction.commit()
    }

    private func tickOnMain() {
        let nowMs = Self.nowMs()
        let duration = UInt64(max(1, config.durationMs))
        while !points.isEmpty {
            let age = nowMs > points[0].tickMs ? nowMs - points[0].tickMs : 0
            if age <= duration {
                break
            }
            points.removeFirst()
        }

        if !points.isEmpty {
            rebuildPath(nowMs: nowMs)
        } else {
            clearSegmentSublayers()
        }

        if points.count < 2 {
            if lastInputMs == 0 || nowMs > lastInputMs + duration {
                resetStateOnMain()
            }
        }
    }

    private func ensureTimerOnMain() {
        if timer != nil {
            return
        }
        let source = DispatchSource.makeTimerSource(queue: DispatchQueue.main)
        source.schedule(
            deadline: .now(),
            repeating: .milliseconds(33),
            leeway: .milliseconds(2)
        )
        source.setEventHandler { [weak self] in
            self?.tickOnMain()
        }
        source.resume()
        timer = source
    }

    func updateOnMain(
        x: Int32,
        y: Int32,
        durationMs: Int,
        lineWidth: Float,
        strokeArgb: UInt32,
        idleFadeStartMs: Int,
        idleFadeEndMs: Int
    ) {
        config.durationMs = max(1, durationMs)
        config.lineWidth = CGFloat(max(0.2, lineWidth))
        config.strokeArgb = strokeArgb
        config.idleFadeStartMs = idleFadeStartMs
        config.idleFadeEndMs = idleFadeEndMs

        guard ensureWindow(forX: x, y: y) else {
            return
        }

        let nowMs = Self.nowMs()
        lastInputMs = nowMs

        if let last = points.last {
            let dx = Double(x - last.x)
            let dy = Double(y - last.y)
            if (dx * dx + dy * dy) < 1.0 {
                return
            }
        }

        points.append(Point(x: x, y: y, tickMs: nowMs))

        let duration = UInt64(max(1, config.durationMs))
        while !points.isEmpty {
            let age = nowMs > points[0].tickMs ? nowMs - points[0].tickMs : 0
            if age <= duration {
                break
            }
            points.removeFirst()
        }

        rebuildPath(nowMs: nowMs)
        ensureTimerOnMain()
    }

    func reset() {
        resetStateOnMain()
    }

    func releaseHandle() {
        resetStateOnMain()
    }
}

@_cdecl("mfx_macos_line_trail_create_v1")
public func mfx_macos_line_trail_create_v1() -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: Unmanaged.passRetained(MfxLineTrailState()).toOpaque())
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }
    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: Unmanaged.passRetained(MfxLineTrailState()).toOpaque())
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_line_trail_release_v1")
public func mfx_macos_line_trail_release_v1(_ handle: UnsafeMutableRawPointer?) {
    let bits = UInt(bitPattern: handle)
    if bits == 0 {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else {
        return
    }
    let state = Unmanaged<MfxLineTrailState>.fromOpaque(ptr).takeRetainedValue()
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            state.releaseHandle()
        }
        return
    }
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            state.releaseHandle()
        }
    }
}

@_cdecl("mfx_macos_line_trail_reset_v1")
public func mfx_macos_line_trail_reset_v1(_ handle: UnsafeMutableRawPointer?) {
    let bits = UInt(bitPattern: handle)
    if bits == 0 {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else {
        return
    }
    let state = Unmanaged<MfxLineTrailState>.fromOpaque(ptr).takeUnretainedValue()
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            state.reset()
        }
        return
    }
    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            state.reset()
        }
    }
}

@_cdecl("mfx_macos_line_trail_update_v1")
public func mfx_macos_line_trail_update_v1(
    _ handle: UnsafeMutableRawPointer?,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ durationMs: Int32,
    _ lineWidth: Float,
    _ strokeArgb: UInt32,
    _ idleFadeStartMs: Int32,
    _ idleFadeEndMs: Int32
) {
    let bits = UInt(bitPattern: handle)
    if bits == 0 {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else {
        return
    }
    let state = Unmanaged<MfxLineTrailState>.fromOpaque(ptr).takeUnretainedValue()

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            state.updateOnMain(
                x: overlayX,
                y: overlayY,
                durationMs: Int(durationMs),
                lineWidth: lineWidth,
                strokeArgb: strokeArgb,
                idleFadeStartMs: Int(idleFadeStartMs),
                idleFadeEndMs: Int(idleFadeEndMs)
            )
        }
        return
    }
    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            state.updateOnMain(
                x: overlayX,
                y: overlayY,
                durationMs: Int(durationMs),
                lineWidth: lineWidth,
                strokeArgb: strokeArgb,
                idleFadeStartMs: Int(idleFadeStartMs),
                idleFadeEndMs: Int(idleFadeEndMs)
            )
        }
    }
}
