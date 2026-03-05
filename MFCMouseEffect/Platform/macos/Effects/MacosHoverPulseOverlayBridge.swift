@preconcurrency import AppKit
@preconcurrency import QuartzCore
@preconcurrency import Foundation

@_silgen_name("mfx_macos_overlay_create_window_v1")
private func mfx_macos_overlay_create_window_v1(
    _ x: Double,
    _ y: Double,
    _ width: Double,
    _ height: Double
) -> UnsafeMutableRawPointer?

@_silgen_name("mfx_macos_overlay_apply_content_scale_v1")
private func mfx_macos_overlay_apply_content_scale_v1(
    _ contentHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32
)

@_silgen_name("mfx_macos_overlay_timer_interval_ms_v1")
private func mfx_macos_overlay_timer_interval_ms_v1(
    _ x: Int32,
    _ y: Int32
) -> Int32

private func mfxClamp(_ value: CGFloat, min minValue: CGFloat, max maxValue: CGFloat) -> CGFloat {
    if value < minValue {
        return minValue
    }
    if value > maxValue {
        return maxValue
    }
    return value
}

private func mfxColorFromArgb(_ argb: UInt32) -> NSColor {
    let alpha = CGFloat(Double((argb >> 24) & 0xFF) / 255.0)
    let red = CGFloat(Double((argb >> 16) & 0xFF) / 255.0)
    let green = CGFloat(Double((argb >> 8) & 0xFF) / 255.0)
    let blue = CGFloat(Double(argb & 0xFF) / 255.0)
    return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
}

private func mfxColorWithScaledAlpha(_ color: NSColor, _ alphaScale: CGFloat) -> NSColor {
    let safeScale = mfxClamp(alphaScale, min: 0.0, max: 1.0)
    let alpha = mfxClamp(color.alphaComponent * safeScale, min: 0.0, max: 1.0)
    return color.withAlphaComponent(alpha)
}

private func mfxHslToRgbColor(hueDeg: CGFloat, saturation: CGFloat, lightness: CGFloat, alpha: CGFloat) -> NSColor {
    let safeHue = hueDeg.truncatingRemainder(dividingBy: 360.0)
    let h = (safeHue < 0.0 ? safeHue + 360.0 : safeHue) / 360.0
    let s = mfxClamp(saturation, min: 0.0, max: 1.0)
    let l = mfxClamp(lightness, min: 0.0, max: 1.0)
    if s <= 0.0001 {
        return NSColor(calibratedRed: l, green: l, blue: l, alpha: mfxClamp(alpha, min: 0.0, max: 1.0))
    }

    let q = l < 0.5 ? l * (1.0 + s) : l + s - l * s
    let p = 2.0 * l - q
    func hue2rgb(_ tIn: CGFloat) -> CGFloat {
        var t = tIn
        if t < 0.0 {
            t += 1.0
        }
        if t > 1.0 {
            t -= 1.0
        }
        if t < 1.0 / 6.0 {
            return p + (q - p) * 6.0 * t
        }
        if t < 1.0 / 2.0 {
            return q
        }
        if t < 2.0 / 3.0 {
            return p + (q - p) * (2.0 / 3.0 - t) * 6.0
        }
        return p
    }

    let r = hue2rgb(h + 1.0 / 3.0)
    let g = hue2rgb(h)
    let b = hue2rgb(h - 1.0 / 3.0)
    return NSColor(
        calibratedRed: r,
        green: g,
        blue: b,
        alpha: mfxClamp(alpha, min: 0.0, max: 1.0)
    )
}

@MainActor
private final class MfxHoverPulseCanvasView: NSView {
    private let tubesMode: Bool
    private let chromaticMode: Bool
    private let glowStrokeColor: NSColor
    private let tubesStrokeColor: NSColor
    private let baseOpacity: CGFloat
    private let breatheDurationSec: Double
    private let tubesSpinDurationSec: Double
    private let timerIntervalMs: Int
    private let startTick: CFTimeInterval
    private var timer: DispatchSourceTimer?

    private static let chainSpeeds: [CGFloat] = [0.002, 0.003, 0.0015]
    private static let chainRadiusScales: [CGFloat] = [0.8, 1.0, 1.2]
    private static let chainBaseColors: [NSColor] = [
        NSColor(calibratedRed: 249.0 / 255.0, green: 103.0 / 255.0, blue: 251.0 / 255.0, alpha: 1.0),
        NSColor(calibratedRed: 83.0 / 255.0, green: 188.0 / 255.0, blue: 40.0 / 255.0, alpha: 1.0),
        NSColor(calibratedRed: 105.0 / 255.0, green: 88.0 / 255.0, blue: 213.0 / 255.0, alpha: 1.0),
    ]
    private static let nodeCount: Int = 20

    init(
        frame frameRect: NSRect,
        tubesMode: Bool,
        chromaticMode: Bool,
        glowStrokeColor: NSColor,
        tubesStrokeColor: NSColor,
        baseOpacity: CGFloat,
        breatheDurationSec: Double,
        tubesSpinDurationSec: Double,
        timerIntervalMs: Int
    ) {
        self.tubesMode = tubesMode
        self.chromaticMode = chromaticMode
        self.glowStrokeColor = glowStrokeColor
        self.tubesStrokeColor = tubesStrokeColor
        self.baseOpacity = mfxClamp(baseOpacity, min: 0.0, max: 1.0)
        self.breatheDurationSec = max(0.05, breatheDurationSec)
        self.tubesSpinDurationSec = max(0.05, tubesSpinDurationSec)
        self.timerIntervalMs = max(4, min(1000, timerIntervalMs))
        self.startTick = CACurrentMediaTime()
        super.init(frame: frameRect)
        wantsLayer = true
        layer?.backgroundColor = NSColor.clear.cgColor
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        return nil
    }

    override var isOpaque: Bool {
        return false
    }

    func start() {
        if timer != nil {
            return
        }
        let source = DispatchSource.makeTimerSource(queue: DispatchQueue.main)
        source.schedule(
            deadline: .now(),
            repeating: .milliseconds(timerIntervalMs),
            leeway: .milliseconds(max(1, timerIntervalMs / 8))
        )
        source.setEventHandler { [weak self] in
            guard let self else {
                return
            }
            self.needsDisplay = true
        }
        source.resume()
        timer = source
    }

    func stop() {
        guard let source = timer else {
            return
        }
        source.setEventHandler {}
        source.cancel()
        timer = nil
    }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        if window == nil {
            stop()
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        guard let context = NSGraphicsContext.current?.cgContext else {
            return
        }
        context.clear(bounds)
        context.saveGState()
        context.setShouldAntialias(true)
        context.setAllowsAntialiasing(true)

        let elapsedSec = CACurrentMediaTime() - startTick
        if tubesMode {
            drawTubes(context: context, elapsedSec: elapsedSec)
        } else {
            drawGlow(context: context, elapsedSec: elapsedSec)
        }
        context.restoreGState()
    }

    private func drawGlow(context: CGContext, elapsedSec: CFTimeInterval) {
        let phase = CGFloat(fmod(elapsedSec / breatheDurationSec, 1.0))
        let pulse = 0.4 + 0.6 * sin(phase * 2.0 * .pi)
        let alphaScale = mfxClamp(pulse, min: 0.0, max: 1.0) * baseOpacity

        let cx = bounds.midX
        let cy = bounds.midY
        let refSize = min(bounds.width, bounds.height)
        let radius = mfxClamp(refSize * 0.32, min: 18.0, max: 96.0)
        let tick = radius * 0.22
        let strokeWidth = mfxClamp(refSize * 0.0125, min: 1.0, max: 6.0)
        let glowWidth = strokeWidth + 6.0

        let glowColor = mfxColorWithScaledAlpha(glowStrokeColor, alphaScale * 0.60)
        drawCrosshairSegments(
            context: context,
            cx: cx,
            cy: cy,
            radius: radius,
            tick: tick,
            strokeWidth: glowWidth,
            strokeColor: glowColor.cgColor
        )

        let strokeColor = mfxColorWithScaledAlpha(glowStrokeColor, alphaScale)
        drawCrosshairSegments(
            context: context,
            cx: cx,
            cy: cy,
            radius: radius,
            tick: tick,
            strokeWidth: strokeWidth,
            strokeColor: strokeColor.cgColor
        )

        let dotAngle = phase * 2.0 * .pi
        let dotOrbit = radius * 0.6
        let dotRadius = mfxClamp(strokeWidth * 1.6, min: 1.5, max: 12.0)
        let dotX = cx + cos(dotAngle) * dotOrbit
        let dotY = cy + sin(dotAngle) * dotOrbit
        context.setFillColor(strokeColor.cgColor)
        context.fillEllipse(in: CGRect(x: dotX - dotRadius, y: dotY - dotRadius, width: dotRadius * 2.0, height: dotRadius * 2.0))
    }

    private func drawCrosshairSegments(
        context: CGContext,
        cx: CGFloat,
        cy: CGFloat,
        radius: CGFloat,
        tick: CGFloat,
        strokeWidth: CGFloat,
        strokeColor: CGColor
    ) {
        context.setLineWidth(strokeWidth)
        context.setLineCap(.round)
        context.setLineJoin(.round)
        context.setStrokeColor(strokeColor)

        context.beginPath()
        context.move(to: CGPoint(x: cx - radius, y: cy))
        context.addLine(to: CGPoint(x: cx - radius + tick, y: cy))
        context.move(to: CGPoint(x: cx + radius - tick, y: cy))
        context.addLine(to: CGPoint(x: cx + radius, y: cy))
        context.move(to: CGPoint(x: cx, y: cy - radius))
        context.addLine(to: CGPoint(x: cx, y: cy - radius + tick))
        context.move(to: CGPoint(x: cx, y: cy + radius - tick))
        context.addLine(to: CGPoint(x: cx, y: cy + radius))
        context.strokePath()
    }

    private func resolveChainColor(chainIndex: Int, nowMs: UInt64) -> NSColor {
        if chromaticMode {
            let hue = CGFloat(fmod(Double(nowMs) * 0.2 + Double(chainIndex) * 30.0, 360.0))
            return mfxHslToRgbColor(hueDeg: hue, saturation: 0.9, lightness: 0.6, alpha: 1.0)
        }

        if chainIndex >= 0 && chainIndex < Self.chainBaseColors.count {
            return Self.chainBaseColors[chainIndex]
        }
        return tubesStrokeColor
    }

    private func drawTubes(context: CGContext, elapsedSec: CFTimeInterval) {
        _ = elapsedSec
        let nowMs = UInt64(ProcessInfo.processInfo.systemUptime * 1000.0)
        let cx = bounds.midX
        let cy = bounds.midY
        let sizeScale = mfxClamp(min(bounds.width, bounds.height) / 190.0, min: 0.72, max: 1.68)
        let baseAlpha: CGFloat = 200.0 / 255.0

        for chainIndex in 0..<Self.chainSpeeds.count {
            let speed = Self.chainSpeeds[chainIndex]
            let radiusScale = Self.chainRadiusScales[chainIndex]
            let time = CGFloat(nowMs) * speed
            let chainColor = resolveChainColor(chainIndex: chainIndex, nowMs: nowMs)

            for nodeIndex in 0..<Self.nodeCount {
                let idx = CGFloat(nodeIndex)
                let nodeCountF = CGFloat(Self.nodeCount)
                let baseAngle = (idx / nodeCountF) * (.pi * 4.0)
                let baseRadius = (10.0 + idx * 2.0) * radiusScale * sizeScale
                let currentRadius = baseRadius + sin(time * 2.0 + idx * 0.1) * (5.0 * sizeScale)
                let rotation = time + baseAngle

                let x = cx + cos(rotation) * currentRadius
                let y = cy + sin(rotation) * currentRadius
                let ratio = 1.0 - idx / nodeCountF
                let radius = (3.0 + 10.0 * ratio) * sizeScale

                drawTubeNodeGradient(
                    context: context,
                    center: CGPoint(x: x, y: y),
                    radius: radius,
                    baseColor: chainColor,
                    alphaScale: baseAlpha
                )
            }
        }
    }

    private func drawTubeNodeGradient(
        context: CGContext,
        center: CGPoint,
        radius: CGFloat,
        baseColor: NSColor,
        alphaScale: CGFloat
    ) {
        let rgbSpace = CGColorSpaceCreateDeviceRGB()
        let boost: CGFloat = 150.0 / 255.0
        let centerColor = NSColor(
            calibratedRed: mfxClamp(baseColor.redComponent + boost, min: 0.0, max: 1.0),
            green: mfxClamp(baseColor.greenComponent + boost, min: 0.0, max: 1.0),
            blue: mfxClamp(baseColor.blueComponent + boost, min: 0.0, max: 1.0),
            alpha: mfxClamp(alphaScale, min: 0.0, max: 1.0)
        )
        let edgeColor = NSColor(
            calibratedRed: baseColor.redComponent,
            green: baseColor.greenComponent,
            blue: baseColor.blueComponent,
            alpha: 0.0
        )
        guard let gradient = CGGradient(
            colorsSpace: rgbSpace,
            colors: [centerColor.cgColor, edgeColor.cgColor] as CFArray,
            locations: [0.0, 1.0]
        ) else {
            context.setFillColor(centerColor.cgColor)
            context.fillEllipse(in: CGRect(x: center.x - radius, y: center.y - radius, width: radius * 2.0, height: radius * 2.0))
            return
        }

        context.drawRadialGradient(
            gradient,
            startCenter: center,
            startRadius: 0.0,
            endCenter: center,
            endRadius: max(1.0, radius),
            options: [.drawsBeforeStartLocation, .drawsAfterEndLocation]
        )
    }
}

@MainActor
private func mfxCreateHoverPulseOverlayOnMainThread(
    frameX: Double,
    frameY: Double,
    frameSize: Double,
    overlayX: Int32,
    overlayY: Int32,
    glowFillArgb: UInt32,
    glowStrokeArgb: UInt32,
    tubesStrokeArgb: UInt32,
    baseOpacity: Double,
    breatheDurationSec: Double,
    tubesSpinDurationSec: Double,
    tubesMode: Bool,
    chromaticMode: Bool
) -> UnsafeMutableRawPointer? {
    _ = glowFillArgb
    let size = max(1.0, frameSize)
    let windowHandle = mfx_macos_overlay_create_window_v1(frameX, frameY, size, size)
    guard let windowHandle else {
        return nil
    }

    let window = Unmanaged<NSWindow>.fromOpaque(windowHandle).takeUnretainedValue()
    guard let content = window.contentView else {
        return windowHandle
    }
    content.wantsLayer = true
    content.layer?.backgroundColor = NSColor.clear.cgColor
    mfx_macos_overlay_apply_content_scale_v1(
        Unmanaged.passUnretained(content).toOpaque(),
        overlayX,
        overlayY
    )

    let hoverView = MfxHoverPulseCanvasView(
        frame: content.bounds,
        tubesMode: tubesMode,
        chromaticMode: chromaticMode,
        glowStrokeColor: mfxColorFromArgb(glowStrokeArgb),
        tubesStrokeColor: mfxColorFromArgb(tubesStrokeArgb),
        baseOpacity: CGFloat(baseOpacity),
        breatheDurationSec: breatheDurationSec,
        tubesSpinDurationSec: tubesSpinDurationSec,
        timerIntervalMs: Int(mfx_macos_overlay_timer_interval_ms_v1(overlayX, overlayY))
    )
    hoverView.autoresizingMask = [.width, .height]
    content.addSubview(hoverView)
    hoverView.start()
    return windowHandle
}

@_cdecl("mfx_macos_hover_pulse_overlay_create_v1")
public func mfx_macos_hover_pulse_overlay_create_v1(
    _ frameX: Double,
    _ frameY: Double,
    _ frameSize: Double,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ glowFillArgb: UInt32,
    _ glowStrokeArgb: UInt32,
    _ tubesStrokeArgb: UInt32,
    _ baseOpacity: Double,
    _ breatheDurationSec: Double,
    _ tubesSpinDurationSec: Double,
    _ tubesMode: Int32,
    _ chromaticMode: Int32
) -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateHoverPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    glowFillArgb: glowFillArgb,
                    glowStrokeArgb: glowStrokeArgb,
                    tubesStrokeArgb: tubesStrokeArgb,
                    baseOpacity: baseOpacity,
                    breatheDurationSec: breatheDurationSec,
                    tubesSpinDurationSec: tubesSpinDurationSec,
                    tubesMode: tubesMode != 0,
                    chromaticMode: chromaticMode != 0
                )
            )
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateHoverPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    glowFillArgb: glowFillArgb,
                    glowStrokeArgb: glowStrokeArgb,
                    tubesStrokeArgb: tubesStrokeArgb,
                    baseOpacity: baseOpacity,
                    breatheDurationSec: breatheDurationSec,
                    tubesSpinDurationSec: tubesSpinDurationSec,
                    tubesMode: tubesMode != 0,
                    chromaticMode: chromaticMode != 0
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}
