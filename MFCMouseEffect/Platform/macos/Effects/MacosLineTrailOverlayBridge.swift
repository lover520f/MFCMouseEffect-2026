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

@_silgen_name("mfx_macos_overlay_timer_interval_ms_v1")
private func mfxOverlayTimerIntervalMs(
    _ x: Int32,
    _ y: Int32
) -> Int32

@_silgen_name("mfx_compute_streamer_trail_segment_metrics_v1")
private func mfxComputeStreamerTrailSegmentMetrics(
    _ segmentRatio: Double,
    _ life: Double,
    _ headPower: Double,
    _ outWidthPx: UnsafeMutablePointer<Double>?,
    _ outCoreOpacity: UnsafeMutablePointer<Double>?,
    _ outGlowOpacity: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_electric_trail_segment_metrics_v1")
private func mfxComputeElectricTrailSegmentMetrics(
    _ frameBucket: UInt64,
    _ segmentIndex: UInt32,
    _ life: Double,
    _ lengthPx: Double,
    _ amplitudeScale: Double,
    _ forkChance: Double,
    _ outJitterA: UnsafeMutablePointer<Double>?,
    _ outJitterB: UnsafeMutablePointer<Double>?,
    _ outGlowWidthPx: UnsafeMutablePointer<Double>?,
    _ outCoreWidthPx: UnsafeMutablePointer<Double>?,
    _ outGlowOpacity: UnsafeMutablePointer<Double>?,
    _ outCoreOpacity: UnsafeMutablePointer<Double>?,
    _ outEmitFork: UnsafeMutablePointer<Int32>?,
    _ outForkT: UnsafeMutablePointer<Double>?,
    _ outForkLengthPx: UnsafeMutablePointer<Double>?,
    _ outForkWidthPx: UnsafeMutablePointer<Double>?,
    _ outForkOpacity: UnsafeMutablePointer<Double>?,
    _ outForkSide: UnsafeMutablePointer<Int32>?
)

@_silgen_name("mfx_compute_meteor_trail_segment_metrics_v1")
private func mfxComputeMeteorTrailSegmentMetrics(
    _ segmentRatio: Double,
    _ life: Double,
    _ outWidthPx: UnsafeMutablePointer<Double>?,
    _ outTrailOpacity: UnsafeMutablePointer<Double>?,
    _ outEmitCore: UnsafeMutablePointer<Int32>?,
    _ outCoreWidthPx: UnsafeMutablePointer<Double>?,
    _ outCoreOpacity: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_particle_trail_segment_metrics_v1")
private func mfxComputeParticleTrailSegmentMetrics(
    _ segmentRatio: Double,
    _ life: Double,
    _ intensity: Double,
    _ outRadiusPx: UnsafeMutablePointer<Double>?,
    _ outOpacity: UnsafeMutablePointer<Double>?,
    _ outEmitHalo: UnsafeMutablePointer<Int32>?,
    _ outHaloRadiusPx: UnsafeMutablePointer<Double>?,
    _ outHaloOpacity: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_particle_trail_emit_count_v1")
private func mfxComputeParticleTrailEmitCount(
    _ distancePx: Double
) -> Int32

@_silgen_name("mfx_compute_particle_trail_spawn_metrics_v1")
private func mfxComputeParticleTrailSpawnMetrics(
    _ ioSeed: UnsafeMutablePointer<UInt32>?,
    _ chromatic: Int32,
    _ globalHueDeg: Double,
    _ outAngleRad: UnsafeMutablePointer<Double>?,
    _ outSpeedPxPerTick: UnsafeMutablePointer<Double>?,
    _ outSizePx: UnsafeMutablePointer<Double>?,
    _ outHueDeg: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_particle_trail_step_metrics_v1")
private func mfxComputeParticleTrailStepMetrics(
    _ x: Double,
    _ y: Double,
    _ vx: Double,
    _ vy: Double,
    _ life: Double,
    _ sizePx: Double,
    _ dtSec: Double,
    _ outNextX: UnsafeMutablePointer<Double>?,
    _ outNextY: UnsafeMutablePointer<Double>?,
    _ outNextVx: UnsafeMutablePointer<Double>?,
    _ outNextVy: UnsafeMutablePointer<Double>?,
    _ outNextLife: UnsafeMutablePointer<Double>?,
    _ outRenderRadiusPx: UnsafeMutablePointer<Double>?,
    _ outRenderOpacity: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_tubes_node_render_metrics_v1")
private func mfxComputeTubesNodeRenderMetrics(
    _ chainIndex: UInt32,
    _ nodeIndex: UInt32,
    _ nodesCount: UInt32,
    _ fadeScale: Double,
    _ outRadiusPx: UnsafeMutablePointer<Double>?,
    _ outAmplitudePx: UnsafeMutablePointer<Double>?,
    _ outAlpha: UnsafeMutablePointer<Double>?,
    _ outNodePhase: UnsafeMutablePointer<Double>?,
    _ outChainPhase: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_tubes_head_follow_v1")
private func mfxComputeTubesHeadFollow(
    _ targetX: Double,
    _ targetY: Double,
    _ currentX: Double,
    _ currentY: Double,
    _ lag: Double,
    _ outNextX: UnsafeMutablePointer<Double>?,
    _ outNextY: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_tubes_node_follow_v1")
private func mfxComputeTubesNodeFollow(
    _ prevX: Double,
    _ prevY: Double,
    _ currentX: Double,
    _ currentY: Double,
    _ lag: Double,
    _ minSegmentDistance: Double,
    _ outNextX: UnsafeMutablePointer<Double>?,
    _ outNextY: UnsafeMutablePointer<Double>?
)

@_silgen_name("mfx_compute_trail_chromatic_hue_deg_v1")
private func mfxComputeTrailChromaticHueDeg(
    _ nowMs: UInt64,
    _ styleKind: Int32,
    _ segmentIndex: UInt32,
    _ laneIndex: UInt32
) -> Double

@MainActor
private final class MfxLineTrailState: NSObject {
    private struct Point {
        var x: Int32
        var y: Int32
        var tickMs: UInt64
    }

    private enum StyleKind: Int32 {
        case line = 0
        case streamer = 1
        case electric = 2
        case meteor = 3
        case tubes = 4
        case particle = 5

        static func resolve(_ rawValue: Int32) -> StyleKind {
            return StyleKind(rawValue: rawValue) ?? .line
        }
    }

    private struct Config {
        var durationMs: Int = 300
        var lineWidth: CGFloat = 4.0
        var strokeArgb: UInt32 = 0xFFFFFFFF
        var fillArgb: UInt32 = 0x66FFFFFF
        var style: StyleKind = .line
        var intensity: Double = 0.0
        var chromatic: Bool = false
        var streamerGlowWidthScale: CGFloat = 1.8
        var streamerCoreWidthScale: CGFloat = 0.55
        var streamerHeadPower: CGFloat = 1.6
        var electricAmplitudeScale: CGFloat = 1.0
        var electricForkChance: CGFloat = 0.10
        var meteorSparkRateScale: CGFloat = 1.0
        var meteorSparkSpeedScale: CGFloat = 1.0
        var idleFadeStartMs: Int = 60
        var idleFadeEndMs: Int = 220
    }

    private struct MeteorSpark {
        var x: CGFloat
        var y: CGFloat
        var vx: CGFloat
        var vy: CGFloat
        var life: CGFloat
        var size: CGFloat
        var hue: CGFloat
    }

    private struct TrailParticle {
        var x: CGFloat
        var y: CGFloat
        var vx: CGFloat
        var vy: CGFloat
        var life: CGFloat
        var size: CGFloat
        var hue: CGFloat
        var renderRadius: CGFloat
        var renderOpacity: CGFloat
    }

    private struct TubeChain {
        var nodes: [CGPoint]
        var color: NSColor
        var lag: CGFloat
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
    private var timerIntervalMs: Int = 16
    private var lastInputMs: UInt64 = 0
    private var rngState: UInt32 = 0x7F4A7C15
    private var meteorSparks: [MeteorSpark] = []
    private var meteorLastUpdateMs: UInt64 = 0
    private var trailParticles: [TrailParticle] = []
    private var particleLastUpdateMs: UInt64 = 0
    private var particleGlobalHueDeg: Double = 0.0
    private var particleLastEmitPoint: CGPoint = .zero
    private var particleHasLastEmitPoint = false
    private var particleHasPendingPoint = false
    private var tubeChains: [TubeChain] = []
    private var tubeFadeAlpha: CGFloat = 255.0
    private var tubeLastTarget: CGPoint = .zero
    private var tubeHasLastTarget = false
    private var tubeLastFrameMs: UInt64 = 0

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

    private static func argbWithForcedAlpha(_ argb: UInt32, _ alpha: UInt8) -> UInt32 {
        return (UInt32(alpha) << 24) | (argb & 0x00FFFFFF)
    }

    private static func clamp01(_ value: Double) -> Double {
        if value <= 0.0 {
            return 0.0
        }
        if value >= 1.0 {
            return 1.0
        }
        return value
    }

    private static func nearScreenOrigin(x: Int32, y: Int32) -> Bool {
        let tolerance: Int32 = 6
        return abs(x) <= tolerance && abs(y) <= tolerance
    }

    private static func isOriginConnector(from: Point, toX: Int32, toY: Int32) -> Bool {
        let fromOrigin = nearScreenOrigin(x: from.x, y: from.y)
        let toOrigin = nearScreenOrigin(x: toX, y: toY)
        if fromOrigin == toOrigin {
            return false
        }
        let dx = Double(toX - from.x)
        let dy = Double(toY - from.y)
        return (dx * dx + dy * dy) >= (24.0 * 24.0)
    }

    private func localPoint(_ point: Point) -> CGPoint {
        return CGPoint(
            x: CGFloat(point.x - windowOriginX),
            y: CGFloat(point.y - windowOriginY)
        )
    }

    private func segmentLife(nowMs: UInt64, point: Point, durationMs: Int, idleFactor: Double) -> Double {
        let safeDuration = max(1, durationMs)
        let age = Double(nowMs > point.tickMs ? nowMs - point.tickMs : 0)
        let life = 1.0 - (age / Double(safeDuration))
        return Self.clamp01(life) * idleFactor
    }

    private func addPathLayer(
        containerLayer: CALayer,
        layerScale: CGFloat,
        path: CGPath,
        color: NSColor,
        lineWidth: CGFloat,
        opacity: Double
    ) {
        if opacity <= 0.0 {
            return
        }
        let layer = CAShapeLayer()
        layer.frame = containerLayer.bounds
        layer.contentsScale = layerScale
        layer.path = path
        layer.strokeColor = color.cgColor
        layer.fillColor = NSColor.clear.cgColor
        layer.lineWidth = max(0.2, lineWidth)
        layer.lineCap = .round
        layer.lineJoin = .round
        layer.allowsEdgeAntialiasing = true
        layer.opacity = Float(Self.clamp01(opacity))
        containerLayer.addSublayer(layer)
    }

    private static func normalizeRgb(_ color: NSColor) -> (r: CGFloat, g: CGFloat, b: CGFloat) {
        let resolved = color.usingColorSpace(.deviceRGB) ?? color
        var r: CGFloat = 0.0
        var g: CGFloat = 0.0
        var b: CGFloat = 0.0
        var a: CGFloat = 0.0
        resolved.getRed(&r, green: &g, blue: &b, alpha: &a)
        return (r, g, b)
    }

    private func addTubeNodeLayer(
        containerLayer: CALayer,
        layerScale: CGFloat,
        center: CGPoint,
        radius: CGFloat,
        baseColor: NSColor,
        alpha: CGFloat
    ) {
        let clampedAlpha = CGFloat(Self.clamp01(Double(alpha)))
        if clampedAlpha <= 0.001 {
            return
        }
        let safeRadius = max(0.4, radius)
        let frame = CGRect(
            x: center.x - safeRadius,
            y: center.y - safeRadius,
            width: safeRadius * 2.0,
            height: safeRadius * 2.0
        )
        let rgb = Self.normalizeRgb(baseColor)
        let centerColor = NSColor(
            calibratedRed: min(1.0, rgb.r + (100.0 / 255.0)),
            green: min(1.0, rgb.g + (100.0 / 255.0)),
            blue: min(1.0, rgb.b + (100.0 / 255.0)),
            alpha: clampedAlpha
        )
        let edgeColor = NSColor(
            calibratedRed: rgb.r,
            green: rgb.g,
            blue: rgb.b,
            alpha: 0.0
        )

        let gradient = CAGradientLayer()
        gradient.frame = frame
        gradient.contentsScale = layerScale
        gradient.type = .radial
        gradient.startPoint = CGPoint(x: 0.5, y: 0.5)
        gradient.endPoint = CGPoint(x: 1.0, y: 1.0)
        gradient.colors = [centerColor.cgColor, edgeColor.cgColor]
        gradient.locations = [0.0, 1.0]
        gradient.cornerRadius = safeRadius
        gradient.masksToBounds = true
        gradient.opacity = 1.0
        containerLayer.addSublayer(gradient)
    }

    private func nextRandom01() -> CGFloat {
        // xorshift32
        var x = rngState
        x ^= x << 13
        x ^= x >> 17
        x ^= x << 5
        rngState = x
        let normalized = Double(x) / Double(UInt32.max)
        return CGFloat(normalized)
    }

    private func random(in range: ClosedRange<CGFloat>) -> CGFloat {
        let t = nextRandom01()
        return range.lowerBound + (range.upperBound - range.lowerBound) * t
    }

    private static func hslToRgbColor(hueDegrees: CGFloat, saturation: CGFloat, lightness: CGFloat, alpha: CGFloat) -> NSColor {
        let h = hueDegrees.truncatingRemainder(dividingBy: 360.0)
        let hh = h < 0.0 ? h + 360.0 : h
        let s = max(0.0, min(1.0, saturation))
        let l = max(0.0, min(1.0, lightness))
        let c = (1.0 - abs(2.0 * l - 1.0)) * s
        let x = c * (1.0 - abs(((hh / 60.0).truncatingRemainder(dividingBy: 2.0)) - 1.0))
        let m = l - c * 0.5
        let (r1, g1, b1): (CGFloat, CGFloat, CGFloat)
        switch hh {
        case 0.0..<60.0:
            (r1, g1, b1) = (c, x, 0.0)
        case 60.0..<120.0:
            (r1, g1, b1) = (x, c, 0.0)
        case 120.0..<180.0:
            (r1, g1, b1) = (0.0, c, x)
        case 180.0..<240.0:
            (r1, g1, b1) = (0.0, x, c)
        case 240.0..<300.0:
            (r1, g1, b1) = (x, 0.0, c)
        default:
            (r1, g1, b1) = (c, 0.0, x)
        }
        return NSColor(
            calibratedRed: r1 + m,
            green: g1 + m,
            blue: b1 + m,
            alpha: max(0.0, min(1.0, alpha))
        )
    }

    private func resetStyleRuntimeStateOnMain() {
        meteorSparks.removeAll(keepingCapacity: false)
        meteorLastUpdateMs = 0
        trailParticles.removeAll(keepingCapacity: false)
        particleLastUpdateMs = 0
        particleGlobalHueDeg = 0.0
        particleLastEmitPoint = .zero
        particleHasLastEmitPoint = false
        particleHasPendingPoint = false
        tubeChains.removeAll(keepingCapacity: false)
        tubeFadeAlpha = 255.0
        tubeHasLastTarget = false
        tubeLastTarget = .zero
        tubeLastFrameMs = 0
    }

    private static func normalizedTubeLag(baseLag: CGFloat, frameDeltaMs: UInt64) -> Double {
        let clampedBase = max(0.01, min(0.99, Double(baseLag)))
        let clampedDt = max(4.0, min(100.0, Double(frameDeltaMs)))
        let frameUnits = clampedDt / 16.6667
        let normalized = 1.0 - pow(1.0 - clampedBase, frameUnits)
        return max(0.01, min(0.99, normalized))
    }

    private func ensureTubeChains(initialTarget: CGPoint) {
        if !tubeChains.isEmpty {
            return
        }
        let nodeCount = 30
        let tubeColors: [NSColor] = [
            NSColor(calibratedRed: 249.0 / 255.0, green: 103.0 / 255.0, blue: 251.0 / 255.0, alpha: 1.0),
            NSColor(calibratedRed: 83.0 / 255.0, green: 188.0 / 255.0, blue: 40.0 / 255.0, alpha: 1.0),
            NSColor(calibratedRed: 105.0 / 255.0, green: 88.0 / 255.0, blue: 213.0 / 255.0, alpha: 1.0),
        ]
        let lags: [CGFloat] = [0.30, 0.40, 0.50]
        tubeChains = (0..<3).map { idx in
            TubeChain(
                nodes: Array(repeating: initialTarget, count: nodeCount),
                color: tubeColors[idx],
                lag: lags[idx]
            )
        }
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

    private static func clampTimerIntervalMs(_ value: Int32) -> Int {
        return max(4, min(1000, Int(value)))
    }

    private func resolveTimerIntervalMs(forX x: Int32, y: Int32) -> Int {
        let interval = mfxOverlayTimerIntervalMs(x, y)
        return Self.clampTimerIntervalMs(interval)
    }

    private func stopTimer() {
        guard let timer else {
            return
        }
        timer.cancel()
        self.timer = nil
        timerIntervalMs = 16
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
        resetStyleRuntimeStateOnMain()
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
        // Point coordinates are relative to the active screen frame.
        // When the overlay window is recreated (screen switch / frame change),
        // drop historical points to prevent cross-screen long connector lines.
        points.removeAll(keepingCapacity: true)

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

    private func emitParticleBurstIfNeeded(head: CGPoint) {
        if !particleHasLastEmitPoint {
            particleLastEmitPoint = head
            particleHasLastEmitPoint = true
            return
        }

        let dx = head.x - particleLastEmitPoint.x
        let dy = head.y - particleLastEmitPoint.y
        let distance = sqrt(dx * dx + dy * dy)
        let emitCount = Int(mfxComputeParticleTrailEmitCount(Double(distance)))
        if emitCount <= 0 {
            return
        }

        particleGlobalHueDeg = fmod(particleGlobalHueDeg + 5.0, 360.0)
        withUnsafeMutablePointer(to: &rngState) { seedPtr in
            for _ in 0..<emitCount {
                var angle = 0.0
                var speed = 0.0
                var size = 0.0
                var hue = 0.0
                mfxComputeParticleTrailSpawnMetrics(
                    seedPtr,
                    config.chromatic ? 1 : 0,
                    particleGlobalHueDeg,
                    &angle,
                    &speed,
                    &size,
                    &hue
                )
                trailParticles.append(
                    TrailParticle(
                        x: head.x,
                        y: head.y,
                        vx: CGFloat(cos(angle) * speed),
                        vy: CGFloat(sin(angle) * speed),
                        life: 1.0,
                        size: CGFloat(size),
                        hue: CGFloat(hue),
                        renderRadius: CGFloat(max(0.0, size * 0.5)),
                        renderOpacity: 1.0
                    )
                )
            }
        }

        if trailParticles.count > 320 {
            trailParticles.removeFirst(trailParticles.count - 320)
        }
        particleLastEmitPoint = head
    }

    private func updateTrailParticles(nowMs: UInt64) {
        let dtSec: Double
        if particleLastUpdateMs == 0 || nowMs <= particleLastUpdateMs {
            dtSec = 0.016
        } else {
            dtSec = min(0.1, Double(nowMs - particleLastUpdateMs) / 1000.0)
        }
        particleLastUpdateMs = nowMs

        if trailParticles.isEmpty {
            return
        }

        var nextParticles: [TrailParticle] = []
        nextParticles.reserveCapacity(trailParticles.count)
        for particle in trailParticles {
            var nextX = 0.0
            var nextY = 0.0
            var nextVx = 0.0
            var nextVy = 0.0
            var nextLife = 0.0
            var nextRadius = 0.0
            var nextOpacity = 0.0
            mfxComputeParticleTrailStepMetrics(
                Double(particle.x),
                Double(particle.y),
                Double(particle.vx),
                Double(particle.vy),
                Double(particle.life),
                Double(particle.size),
                dtSec,
                &nextX,
                &nextY,
                &nextVx,
                &nextVy,
                &nextLife,
                &nextRadius,
                &nextOpacity
            )
            if nextLife <= 0.0 {
                continue
            }
            nextParticles.append(
                TrailParticle(
                    x: CGFloat(nextX),
                    y: CGFloat(nextY),
                    vx: CGFloat(nextVx),
                    vy: CGFloat(nextVy),
                    life: CGFloat(nextLife),
                    size: particle.size,
                    hue: particle.hue,
                    renderRadius: CGFloat(max(0.0, nextRadius)),
                    renderOpacity: CGFloat(max(0.0, min(1.0, nextOpacity)))
                )
            )
        }
        trailParticles = nextParticles
    }

    private func renderTrailParticles(containerLayer: CALayer, layerScale: CGFloat) {
        for particle in trailParticles {
            let radius = max(0.2, particle.renderRadius)
            let alpha = max(0.0, min(1.0, particle.renderOpacity))
            if alpha <= 0.001 {
                continue
            }
            let localX = particle.x - CGFloat(windowOriginX)
            let localY = particle.y - CGFloat(windowOriginY)
            let color = Self.hslToRgbColor(
                hueDegrees: particle.hue,
                saturation: 0.8,
                lightness: 0.6,
                alpha: 1.0
            )
            let layer = CAShapeLayer()
            layer.frame = containerLayer.bounds
            layer.contentsScale = layerScale
            layer.path = CGPath(
                ellipseIn: CGRect(
                    x: localX - radius,
                    y: localY - radius,
                    width: radius * 2.0,
                    height: radius * 2.0
                ),
                transform: nil
            )
            layer.fillColor = color.withAlphaComponent(alpha).cgColor
            layer.strokeColor = NSColor.clear.cgColor
            layer.opacity = 1.0
            containerLayer.addSublayer(layer)
        }
    }

    private func rebuildPath(nowMs: UInt64) {
        guard let containerLayer else {
            return
        }

        clearSegmentSublayers()
        if points.isEmpty && !(config.style == .particle && !trailParticles.isEmpty) {
            return
        }

        let idleFactor: Double
        if let lastPoint = points.last {
            idleFactor = Self.idleFadeFactor(
                nowMs: nowMs,
                lastPointMs: lastPoint.tickMs,
                startMs: config.idleFadeStartMs,
                endMs: config.idleFadeEndMs
            )
        } else {
            idleFactor = 1.0
        }
        let durationMs = max(1, config.durationMs)
        let lineWidth = max(1.0, config.lineWidth)
        let layerScale = max(containerLayer.contentsScale, CGFloat(1.0))

        CATransaction.begin()
        CATransaction.setDisableActions(true)

        let fullAlphaStroke = Self.argbWithForcedAlpha(config.strokeArgb, 0xFF)
        let fullAlphaFill = Self.argbWithForcedAlpha(config.fillArgb, 0xFF)
        let strokeColor = Self.colorFromArgb(fullAlphaStroke, 1.0)
        let fillColor = Self.colorFromArgb(fullAlphaFill, 1.0)

        if points.count == 1 && config.style != .particle {
            let point = points[0]
            let local = localPoint(point)
            let radius = max(lineWidth * 0.6, 2.0)
            let opacity = segmentLife(nowMs: nowMs, point: point, durationMs: durationMs, idleFactor: idleFactor)

            let dot = CAShapeLayer()
            dot.frame = containerLayer.bounds
            dot.contentsScale = layerScale
            dot.path = CGPath(
                ellipseIn: CGRect(x: local.x - radius, y: local.y - radius, width: radius * 2.0, height: radius * 2.0),
                transform: nil
            )
            dot.fillColor = strokeColor.cgColor
            dot.strokeColor = NSColor.clear.cgColor
            dot.opacity = Float(opacity)
            containerLayer.addSublayer(dot)

            CATransaction.commit()
            return
        }

        if config.style == .particle {
            if let head = points.last {
                emitParticleBurstIfNeeded(head: CGPoint(x: CGFloat(head.x), y: CGFloat(head.y)))
            }
            updateTrailParticles(nowMs: nowMs)
            renderTrailParticles(containerLayer: containerLayer, layerScale: layerScale)
            CATransaction.commit()
            return
        }

        if config.style == .tubes {
            let frameDeltaMs: UInt64
            if tubeLastFrameMs == 0 || nowMs <= tubeLastFrameMs {
                frameDeltaMs = UInt64(max(4, min(100, timerIntervalMs)))
            } else {
                frameDeltaMs = min(100, nowMs - tubeLastFrameMs)
            }
            tubeLastFrameMs = nowMs

            let hasInput = !points.isEmpty
            let target: CGPoint
            if let head = points.last {
                target = localPoint(head)
                tubeLastTarget = target
                tubeHasLastTarget = true
            } else if tubeHasLastTarget {
                target = tubeLastTarget
            } else {
                CATransaction.commit()
                return
            }
            ensureTubeChains(initialTarget: target)

            let isIdle = !hasInput || (nowMs > points[points.count - 1].tickMs + 50)
            if !isIdle {
                tubeFadeAlpha = 255.0
            } else {
                var converged = true
                for chain in tubeChains {
                    guard let head = chain.nodes.first, let tail = chain.nodes.last else {
                        continue
                    }
                    let dx = head.x - tail.x
                    let dy = head.y - tail.y
                    if (dx * dx + dy * dy) > 100.0 {
                        converged = false
                        break
                    }
                }
                tubeFadeAlpha -= converged ? 35.0 : 10.0
                if tubeFadeAlpha < 0.0 {
                    tubeFadeAlpha = 0.0
                }
            }
            if tubeFadeAlpha <= 0.0 {
                CATransaction.commit()
                return
            }

            for chainIndex in 0..<tubeChains.count {
                var chain = tubeChains[chainIndex]
                if !chain.nodes.isEmpty {
                    var headNextX = 0.0
                    var headNextY = 0.0
                    mfxComputeTubesHeadFollow(
                        Double(target.x),
                        Double(target.y),
                        Double(chain.nodes[0].x),
                        Double(chain.nodes[0].y),
                        Self.normalizedTubeLag(baseLag: chain.lag, frameDeltaMs: frameDeltaMs),
                        &headNextX,
                        &headNextY
                    )
                    chain.nodes[0].x = CGFloat(headNextX)
                    chain.nodes[0].y = CGFloat(headNextY)

                    if chain.nodes.count > 1 {
                        for nodeIndex in 1..<chain.nodes.count {
                            let prev = chain.nodes[nodeIndex - 1]
                            var curr = chain.nodes[nodeIndex]
                            var nextX = 0.0
                            var nextY = 0.0
                            mfxComputeTubesNodeFollow(
                                Double(prev.x),
                                Double(prev.y),
                                Double(curr.x),
                                Double(curr.y),
                                Self.normalizedTubeLag(baseLag: chain.lag, frameDeltaMs: frameDeltaMs),
                                3.5,
                                &nextX,
                                &nextY
                            )
                            curr.x = CGFloat(nextX)
                            curr.y = CGFloat(nextY)
                            chain.nodes[nodeIndex] = curr
                        }
                    }
                }
                tubeChains[chainIndex] = chain
            }

            let fadeScale = tubeFadeAlpha / 255.0
            let frameTime = CGFloat(Double(nowMs) * 0.005)
            for chainIndex in 0..<tubeChains.count {
                let chain = tubeChains[chainIndex]
                let nodesCount = max(1, chain.nodes.count)
                var chainColor = chain.color
                if config.chromatic {
                    let hue = CGFloat(
                        mfxComputeTrailChromaticHueDeg(
                            nowMs,
                            4,
                            0,
                            UInt32(chainIndex)
                        )
                    )
                    chainColor = Self.hslToRgbColor(
                        hueDegrees: hue,
                        saturation: 0.9,
                        lightness: 0.6,
                        alpha: 1.0
                    )
                }
                for nodeIndex in stride(from: nodesCount - 1, through: 0, by: -1) {
                    let node = chain.nodes[nodeIndex]
                    var radius = 0.0
                    var amplitude = 0.0
                    var alpha = 0.0
                    var nodePhase = 0.0
                    var chainPhase = 0.0
                    mfxComputeTubesNodeRenderMetrics(
                        UInt32(chainIndex),
                        UInt32(nodeIndex),
                        UInt32(nodesCount),
                        Double(fadeScale),
                        &radius,
                        &amplitude,
                        &alpha,
                        &nodePhase,
                        &chainPhase
                    )
                    let renderX = node.x + cos(frameTime + CGFloat(nodePhase) + CGFloat(chainPhase)) * CGFloat(amplitude)
                    let renderY = node.y + sin(frameTime + CGFloat(nodePhase) + CGFloat(chainPhase)) * CGFloat(amplitude)
                    addTubeNodeLayer(
                        containerLayer: containerLayer,
                        layerScale: layerScale,
                        center: CGPoint(x: renderX, y: renderY),
                        radius: CGFloat(radius),
                        baseColor: chainColor,
                        alpha: CGFloat(alpha)
                    )
                }
            }

            CATransaction.commit()
            return
        }

        if config.style == .meteor {
            let dt: CGFloat
            if meteorLastUpdateMs == 0 || nowMs <= meteorLastUpdateMs {
                dt = 0.016
            } else {
                dt = min(0.1, CGFloat(Double(nowMs - meteorLastUpdateMs) / 1000.0))
            }
            meteorLastUpdateMs = nowMs
            if points.count >= 2 {
                let head = localPoint(points[points.count - 1])
                let prev = localPoint(points[points.count - 2])
                let dx = head.x - prev.x
                let dy = head.y - prev.y
                let dist = sqrt(dx * dx + dy * dy)
                if dist > 1.0 {
                    let baseEmit = min(4, Int(dist / 7.0) + 1)
                    let emitCount = max(1, min(8, Int(CGFloat(baseEmit) * config.meteorSparkRateScale)))
                    for sparkIndex in 0..<emitCount {
                        let baseAngle = atan2(-dy, -dx)
                        let angle = baseAngle + random(in: -0.55...0.55)
                        let speed = random(in: 0.6...3.2) * config.meteorSparkSpeedScale
                        meteorSparks.append(
                            MeteorSpark(
                                x: head.x,
                                y: head.y,
                                vx: cos(angle) * speed,
                                vy: sin(angle) * speed,
                                life: 1.0,
                                size: random(in: 1.0...3.5),
                                hue: CGFloat(fmod(Double(nowMs) * 0.1 + Double(sparkIndex) * 20.0, 360.0))
                            )
                        )
                    }
                }
            }

            let sparkFadeBoost = 1.0 + CGFloat(1.0 - idleFactor) * 2.0
            var updatedSparks: [MeteorSpark] = []
            updatedSparks.reserveCapacity(meteorSparks.count)
            for var spark in meteorSparks {
                spark.x += spark.vx
                spark.y += spark.vy
                spark.vy += 0.05
                spark.life -= dt * 1.9 * sparkFadeBoost
                if spark.life > 0.0 {
                    updatedSparks.append(spark)
                }
            }
            if updatedSparks.count > 220 {
                updatedSparks.removeFirst(updatedSparks.count - 220)
            }
            meteorSparks = updatedSparks
        }

        let segmentCount = points.count - 1
        let safeSegmentCount = max(1, segmentCount)
        let electricFrameBucket = UInt64(nowMs / 30)

        for i in 0..<segmentCount {
            let startPoint = points[i]
            let endPoint = points[i + 1]
            let life = segmentLife(nowMs: nowMs, point: startPoint, durationMs: durationMs, idleFactor: idleFactor)
            if life <= 0.0 {
                continue
            }
            let start = localPoint(startPoint)
            let end = localPoint(endPoint)

            switch config.style {
            case .line:
                var c = strokeColor
                if config.chromatic {
                    let hue = CGFloat(
                        mfxComputeTrailChromaticHueDeg(
                            nowMs,
                            0,
                            UInt32(i),
                            0
                        )
                    )
                    c = Self.hslToRgbColor(hueDegrees: hue, saturation: 0.85, lightness: 0.60, alpha: 1.0)
                }
                let p = CGMutablePath()
                p.move(to: start)
                p.addLine(to: end)
                addPathLayer(containerLayer: containerLayer, layerScale: layerScale, path: p, color: c, lineWidth: lineWidth, opacity: life)
            case .streamer:
                let t: Double = (segmentCount <= 1)
                    ? 1.0
                    : (Double(i) / Double(max(1, segmentCount - 1)))
                var width = 0.0
                var alphaCore = 0.0
                var alphaGlow = 0.0
                mfxComputeStreamerTrailSegmentMetrics(
                    t,
                    life,
                    Double(config.streamerHeadPower),
                    &width,
                    &alphaCore,
                    &alphaGlow
                )
                var coreColor = strokeColor
                var glowColor = fillColor
                if config.chromatic {
                    let hue = CGFloat(
                        mfxComputeTrailChromaticHueDeg(
                            nowMs,
                            1,
                            UInt32(i),
                            0
                        )
                    )
                    coreColor = Self.hslToRgbColor(hueDegrees: hue, saturation: 0.95, lightness: 0.62, alpha: 1.0)
                    glowColor = Self.hslToRgbColor(hueDegrees: hue, saturation: 0.95, lightness: 0.58, alpha: 1.0)
                }
                let p = CGMutablePath()
                p.move(to: start)
                p.addLine(to: end)
                addPathLayer(
                    containerLayer: containerLayer,
                    layerScale: layerScale,
                    path: p,
                    color: glowColor,
                    lineWidth: max(0.8, CGFloat(width) * config.streamerGlowWidthScale),
                    opacity: alphaGlow
                )
                addPathLayer(
                    containerLayer: containerLayer,
                    layerScale: layerScale,
                    path: p,
                    color: coreColor,
                    lineWidth: max(1.5, CGFloat(width) * config.streamerCoreWidthScale),
                    opacity: alphaCore
                )
            case .electric:
                let dx = end.x - start.x
                let dy = end.y - start.y
                let len = sqrt(dx * dx + dy * dy)
                if len < 0.5 {
                    continue
                }
                let invLen = 1.0 / len
                let nx = -dy * invLen
                let ny = dx * invLen
                var jitterA = 0.0
                var jitterB = 0.0
                var glowWidth = 0.0
                var coreWidth = 0.0
                var glowOpacity = 0.0
                var coreOpacity = 0.0
                var emitFork: Int32 = 0
                var forkT = 0.0
                var forkLength = 0.0
                var forkWidth = 0.0
                var forkOpacity = 0.0
                var forkSide: Int32 = 1
                mfxComputeElectricTrailSegmentMetrics(
                    electricFrameBucket,
                    UInt32(i),
                    life,
                    Double(len),
                    Double(config.electricAmplitudeScale),
                    Double(config.electricForkChance),
                    &jitterA,
                    &jitterB,
                    &glowWidth,
                    &coreWidth,
                    &glowOpacity,
                    &coreOpacity,
                    &emitFork,
                    &forkT,
                    &forkLength,
                    &forkWidth,
                    &forkOpacity,
                    &forkSide
                )
                let o1 = CGFloat(jitterA)
                let o2 = CGFloat(jitterB)
                let a = start
                let b = CGPoint(x: start.x + dx * 0.35 + nx * o1, y: start.y + dy * 0.35 + ny * o1)
                let c = CGPoint(x: start.x + dx * 0.70 + nx * o2, y: start.y + dy * 0.70 + ny * o2)
                let d = end
                let coreW = CGFloat(max(1.0, coreWidth))
                let glowW = CGFloat(max(coreW, CGFloat(glowWidth)))
                var cGlow = Self.hslToRgbColor(hueDegrees: 190.0, saturation: 0.9, lightness: 0.62, alpha: 1.0)
                var cCore = NSColor.white
                if config.chromatic {
                    let hue = CGFloat(
                        mfxComputeTrailChromaticHueDeg(
                            nowMs,
                            2,
                            UInt32(i),
                            0
                        )
                    )
                    cGlow = Self.hslToRgbColor(hueDegrees: hue, saturation: 1.0, lightness: 0.60, alpha: 1.0)
                    cCore = Self.hslToRgbColor(hueDegrees: fmod(hue + 30.0, 360.0), saturation: 1.0, lightness: 0.75, alpha: 1.0)
                }
                let path = CGMutablePath()
                path.move(to: a)
                path.addLine(to: b)
                path.addLine(to: c)
                path.addLine(to: d)
                addPathLayer(
                    containerLayer: containerLayer,
                    layerScale: layerScale,
                    path: path,
                    color: cGlow,
                    lineWidth: glowW,
                    opacity: glowOpacity
                )
                addPathLayer(
                    containerLayer: containerLayer,
                    layerScale: layerScale,
                    path: path,
                    color: cCore,
                    lineWidth: coreW,
                    opacity: coreOpacity
                )
                if emitFork != 0 {
                    let t = CGFloat(forkT)
                    let base = CGPoint(x: start.x + dx * t, y: start.y + dy * t)
                    let side: CGFloat = (forkSide < 0) ? -1.0 : 1.0
                    let forkLen = CGFloat(forkLength)
                    let tip = CGPoint(
                        x: base.x + nx * side * forkLen + dx * 0.05,
                        y: base.y + ny * side * forkLen + dy * 0.05
                    )
                    let forkPath = CGMutablePath()
                    forkPath.move(to: base)
                    forkPath.addLine(to: tip)
                    addPathLayer(
                        containerLayer: containerLayer,
                        layerScale: layerScale,
                        path: forkPath,
                        color: cGlow,
                        lineWidth: CGFloat(max(1.0, forkWidth)),
                        opacity: forkOpacity
                    )
                }
            case .meteor:
                let ratio = Double(i) / Double(safeSegmentCount)
                var width = 0.0
                var alpha = 0.0
                var emitCore: Int32 = 0
                var coreWidth = 0.0
                var coreOpacity = 0.0
                mfxComputeMeteorTrailSegmentMetrics(
                    ratio,
                    life,
                    &width,
                    &alpha,
                    &emitCore,
                    &coreWidth,
                    &coreOpacity
                )
                var meteorColor = fillColor
                if config.chromatic {
                    let hue = CGFloat(
                        mfxComputeTrailChromaticHueDeg(
                            nowMs,
                            3,
                            UInt32(i),
                            0
                        )
                    )
                    meteorColor = Self.hslToRgbColor(hueDegrees: hue, saturation: 0.9, lightness: 0.6, alpha: 1.0)
                } else {
                    meteorColor = NSColor(calibratedRed: 1.0, green: 220.0 / 255.0, blue: 160.0 / 255.0, alpha: 1.0)
                }
                let path = CGMutablePath()
                path.move(to: start)
                path.addLine(to: end)
                addPathLayer(
                    containerLayer: containerLayer,
                    layerScale: layerScale,
                    path: path,
                    color: meteorColor,
                    lineWidth: CGFloat(width),
                    opacity: alpha
                )
                if emitCore != 0 {
                    addPathLayer(
                        containerLayer: containerLayer,
                        layerScale: layerScale,
                        path: path,
                        color: NSColor.white,
                        lineWidth: CGFloat(max(1.0, coreWidth)),
                        opacity: coreOpacity
                    )
                }
            case .particle:
                break
            case .tubes:
                break
            }
        }

        if config.style == .meteor {
            for spark in meteorSparks {
                let alpha = max(0.0, min(1.0, Double(spark.life) * idleFactor))
                let sparkColor: NSColor
                if config.chromatic {
                    sparkColor = Self.hslToRgbColor(hueDegrees: spark.hue, saturation: 0.8, lightness: 0.7, alpha: 1.0)
                } else {
                    sparkColor = NSColor(calibratedRed: 1.0, green: 235.0 / 255.0, blue: 170.0 / 255.0, alpha: 1.0)
                }
                let core = CAShapeLayer()
                core.frame = containerLayer.bounds
                core.contentsScale = layerScale
                core.path = CGPath(ellipseIn: CGRect(x: spark.x - spark.size * 0.5, y: spark.y - spark.size * 0.5, width: spark.size, height: spark.size), transform: nil)
                core.fillColor = sparkColor.withAlphaComponent(alpha).cgColor
                core.strokeColor = NSColor.clear.cgColor
                core.opacity = 1.0
                containerLayer.addSublayer(core)
                if spark.life > 0.5 {
                    let halo = CAShapeLayer()
                    halo.frame = containerLayer.bounds
                    halo.contentsScale = layerScale
                    let haloSize = spark.size * 2.5
                    halo.path = CGPath(ellipseIn: CGRect(x: spark.x - haloSize * 0.5, y: spark.y - haloSize * 0.5, width: haloSize, height: haloSize), transform: nil)
                    halo.fillColor = sparkColor.withAlphaComponent(alpha * 0.25).cgColor
                    halo.strokeColor = NSColor.clear.cgColor
                    halo.opacity = 1.0
                    containerLayer.addSublayer(halo)
                }
            }

            if let head = points.last {
                let headPoint = localPoint(head)
                let layers: [(radius: CGFloat, alpha: CGFloat, color: NSColor)] = [
                    (18.0, CGFloat(0.16 * idleFactor), config.chromatic
                        ? Self.hslToRgbColor(hueDegrees: CGFloat(fmod(Double(nowMs) * 0.2, 360.0)), saturation: 0.8, lightness: 0.5, alpha: 1.0)
                        : NSColor(calibratedRed: 1.0, green: 200.0 / 255.0, blue: 120.0 / 255.0, alpha: 1.0)),
                    (10.0, CGFloat(0.42 * idleFactor), config.chromatic
                        ? Self.hslToRgbColor(hueDegrees: CGFloat(fmod(Double(nowMs) * 0.2, 360.0)), saturation: 0.9, lightness: 0.7, alpha: 1.0)
                        : NSColor(calibratedRed: 1.0, green: 240.0 / 255.0, blue: 180.0 / 255.0, alpha: 1.0)),
                    (4.0, 1.0, NSColor.white),
                ]
                for layer in layers {
                    let ring = CAShapeLayer()
                    ring.frame = containerLayer.bounds
                    ring.contentsScale = layerScale
                    ring.path = CGPath(ellipseIn: CGRect(x: headPoint.x - layer.radius, y: headPoint.y - layer.radius, width: layer.radius * 2.0, height: layer.radius * 2.0), transform: nil)
                    ring.fillColor = layer.color.withAlphaComponent(layer.alpha).cgColor
                    ring.strokeColor = NSColor.clear.cgColor
                    ring.opacity = 1.0
                    containerLayer.addSublayer(ring)
                }
            }
        }

        CATransaction.commit()
    }

    private func tickOnMain() {
        let nowMs = Self.nowMs()
        let duration = UInt64(max(1, config.durationMs))

        if config.style == .particle {
            if particleHasPendingPoint, let head = points.last {
                emitParticleBurstIfNeeded(head: CGPoint(x: CGFloat(head.x), y: CGFloat(head.y)))
                points.removeAll(keepingCapacity: true)
                particleHasPendingPoint = false
            }
            updateTrailParticles(nowMs: nowMs)
            clearSegmentSublayers()
            if let containerLayer {
                CATransaction.begin()
                CATransaction.setDisableActions(true)
                renderTrailParticles(
                    containerLayer: containerLayer,
                    layerScale: max(containerLayer.contentsScale, CGFloat(1.0))
                )
                CATransaction.commit()
            }
            if trailParticles.isEmpty {
                if lastInputMs == 0 || nowMs > lastInputMs + duration {
                    resetStateOnMain()
                }
            }
            return
        }

        while !points.isEmpty {
            let age = nowMs > points[0].tickMs ? nowMs - points[0].tickMs : 0
            if age <= duration {
                break
            }
            points.removeFirst()
        }

        let particleActive = (config.style == .particle && !trailParticles.isEmpty)
        if !points.isEmpty || particleActive {
            rebuildPath(nowMs: nowMs)
        } else {
            clearSegmentSublayers()
        }

        if points.count < 2 {
            let canResetParticle = config.style != .particle || trailParticles.isEmpty
            if lastInputMs == 0 || (canResetParticle && nowMs > lastInputMs + duration) {
                resetStateOnMain()
            }
        }
    }

    private func ensureTimerOnMain(x: Int32, y: Int32) {
        let baseIntervalMs = resolveTimerIntervalMs(forX: x, y: y)
        let desiredIntervalMs = (config.style == .particle) ? max(16, baseIntervalMs) : baseIntervalMs
        if let timer {
            if timerIntervalMs != desiredIntervalMs {
                timer.schedule(
                    deadline: .now(),
                    repeating: .milliseconds(desiredIntervalMs),
                    leeway: .milliseconds(max(1, desiredIntervalMs / 8))
                )
                timerIntervalMs = desiredIntervalMs
            }
            return
        }
        let source = DispatchSource.makeTimerSource(queue: DispatchQueue.main)
        source.schedule(
            deadline: .now(),
            repeating: .milliseconds(desiredIntervalMs),
            leeway: .milliseconds(max(1, desiredIntervalMs / 8))
        )
        source.setEventHandler { [weak self] in
            self?.tickOnMain()
        }
        source.resume()
        timer = source
        timerIntervalMs = desiredIntervalMs
    }

    func updateOnMain(
        x: Int32,
        y: Int32,
        durationMs: Int,
        lineWidth: Float,
        strokeArgb: UInt32,
        fillArgb: UInt32,
        styleKind: Int32,
        intensity: Double,
        chromatic: Int32,
        streamerGlowWidthScale: Float,
        streamerCoreWidthScale: Float,
        streamerHeadPower: Float,
        electricAmplitudeScale: Float,
        electricForkChance: Float,
        meteorSparkRateScale: Float,
        meteorSparkSpeedScale: Float,
        idleFadeStartMs: Int,
        idleFadeEndMs: Int
    ) {
        let nextStyle = StyleKind.resolve(styleKind)
        if config.style != nextStyle {
            resetStyleRuntimeStateOnMain()
        }
        config.durationMs = max(1, durationMs)
        config.lineWidth = CGFloat(max(1.0, lineWidth))
        config.strokeArgb = strokeArgb
        config.fillArgb = fillArgb
        config.style = nextStyle
        config.intensity = Self.clamp01(intensity)
        config.chromatic = (chromatic != 0)
        config.streamerGlowWidthScale = max(0.5, min(4.0, CGFloat(streamerGlowWidthScale)))
        config.streamerCoreWidthScale = max(0.2, min(2.0, CGFloat(streamerCoreWidthScale)))
        config.streamerHeadPower = max(0.8, min(3.0, CGFloat(streamerHeadPower)))
        config.electricAmplitudeScale = max(0.2, min(3.0, CGFloat(electricAmplitudeScale)))
        config.electricForkChance = max(0.0, min(0.5, CGFloat(electricForkChance)))
        config.meteorSparkRateScale = max(0.2, min(4.0, CGFloat(meteorSparkRateScale)))
        config.meteorSparkSpeedScale = max(0.2, min(4.0, CGFloat(meteorSparkSpeedScale)))
        config.idleFadeStartMs = idleFadeStartMs
        config.idleFadeEndMs = idleFadeEndMs

        guard ensureWindow(forX: x, y: y) else {
            return
        }

        let nowMs = Self.nowMs()
        lastInputMs = nowMs

        if let last = points.last {
            if Self.isOriginConnector(from: last, toX: x, toY: y) {
                points.removeAll(keepingCapacity: true)
            }
            let dx = Double(x - last.x)
            let dy = Double(y - last.y)
            if (dx * dx + dy * dy) > (1600.0 * 1600.0) {
                points.removeAll(keepingCapacity: true)
            }
            if (dx * dx + dy * dy) < 1.0 {
                return
            }
        }

        if config.style == .particle {
            points.removeAll(keepingCapacity: true)
            points.append(Point(x: x, y: y, tickMs: nowMs))
            particleHasPendingPoint = true
            ensureTimerOnMain(x: x, y: y)
            return
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
        ensureTimerOnMain(x: x, y: y)
    }

    func reset() {
        resetStateOnMain()
    }

    func releaseHandle() {
        resetStateOnMain()
    }

    func isActive() -> Bool {
        return windowHandle != nil && (!points.isEmpty || !trailParticles.isEmpty)
    }

    func pointCount() -> Int {
        return points.count
    }

    func lineWidthPx() -> Double {
        return Double(config.lineWidth)
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
    _ fillArgb: UInt32,
    _ styleKind: Int32,
    _ intensity: Double,
    _ chromatic: Int32,
    _ streamerGlowWidthScale: Float,
    _ streamerCoreWidthScale: Float,
    _ streamerHeadPower: Float,
    _ electricAmplitudeScale: Float,
    _ electricForkChance: Float,
    _ meteorSparkRateScale: Float,
    _ meteorSparkSpeedScale: Float,
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
                fillArgb: fillArgb,
                styleKind: styleKind,
                intensity: intensity,
                chromatic: chromatic,
                streamerGlowWidthScale: streamerGlowWidthScale,
                streamerCoreWidthScale: streamerCoreWidthScale,
                streamerHeadPower: streamerHeadPower,
                electricAmplitudeScale: electricAmplitudeScale,
                electricForkChance: electricForkChance,
                meteorSparkRateScale: meteorSparkRateScale,
                meteorSparkSpeedScale: meteorSparkSpeedScale,
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
                fillArgb: fillArgb,
                styleKind: styleKind,
                intensity: intensity,
                chromatic: chromatic,
                streamerGlowWidthScale: streamerGlowWidthScale,
                streamerCoreWidthScale: streamerCoreWidthScale,
                streamerHeadPower: streamerHeadPower,
                electricAmplitudeScale: electricAmplitudeScale,
                electricForkChance: electricForkChance,
                meteorSparkRateScale: meteorSparkRateScale,
                meteorSparkSpeedScale: meteorSparkSpeedScale,
                idleFadeStartMs: Int(idleFadeStartMs),
                idleFadeEndMs: Int(idleFadeEndMs)
            )
        }
    }
}

@_cdecl("mfx_macos_line_trail_is_active_v1")
public func mfx_macos_line_trail_is_active_v1(_ handle: UnsafeMutableRawPointer?) -> Int32 {
    let bits = UInt(bitPattern: handle)
    if bits == 0 {
        return 0
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else {
        return 0
    }
    let state = Unmanaged<MfxLineTrailState>.fromOpaque(ptr).takeUnretainedValue()

    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            state.isActive() ? 1 : 0
        }
    }

    var active: Int32 = 0
    DispatchQueue.main.sync {
        active = MainActor.assumeIsolated {
            state.isActive() ? 1 : 0
        }
    }
    return active
}

@_cdecl("mfx_macos_line_trail_point_count_v1")
public func mfx_macos_line_trail_point_count_v1(_ handle: UnsafeMutableRawPointer?) -> Int32 {
    let bits = UInt(bitPattern: handle)
    if bits == 0 {
        return 0
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else {
        return 0
    }
    let state = Unmanaged<MfxLineTrailState>.fromOpaque(ptr).takeUnretainedValue()

    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            Int32(max(0, min(Int(Int32.max), state.pointCount())))
        }
    }

    var count: Int32 = 0
    DispatchQueue.main.sync {
        count = MainActor.assumeIsolated {
            Int32(max(0, min(Int(Int32.max), state.pointCount())))
        }
    }
    return count
}

@_cdecl("mfx_macos_line_trail_line_width_px_v1")
public func mfx_macos_line_trail_line_width_px_v1(_ handle: UnsafeMutableRawPointer?) -> Double {
    let bits = UInt(bitPattern: handle)
    if bits == 0 {
        return 0.0
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else {
        return 0.0
    }
    let state = Unmanaged<MfxLineTrailState>.fromOpaque(ptr).takeUnretainedValue()

    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            state.lineWidthPx()
        }
    }

    var width: Double = 0.0
    DispatchQueue.main.sync {
        width = MainActor.assumeIsolated {
            state.lineWidthPx()
        }
    }
    return width
}
