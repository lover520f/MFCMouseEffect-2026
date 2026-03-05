/// MacosScrollHelixAnimator.swift
/// Thin per-frame helix renderer for macOS.
/// Animation math delegates to shared C++ Core via opaque-handle C bridge.
/// Swift side: Timer driving + CGContext drawing.

@preconcurrency import AppKit
@preconcurrency import QuartzCore
@preconcurrency import Foundation

// MARK: - C bridge declarations (opaque-handle pattern)

@_silgen_name("mfx_scroll_helix_state_create")
private func mfx_helix_state_create() -> UnsafeMutableRawPointer?

@_silgen_name("mfx_scroll_helix_state_destroy")
private func mfx_helix_state_destroy(_ state: UnsafeMutableRawPointer?)

@_silgen_name("mfx_scroll_helix_compute")
private func mfx_helix_compute(
    _ state: UnsafeMutableRawPointer?,
    _ t: Float, _ elapsedMs: UInt64, _ sizePx: Int32,
    _ dirRad: Float, _ intensity: Float,
    _ startRadius: Float, _ endRadius: Float, _ strokeWidth: Float,
    _ strokeR: Float, _ strokeG: Float, _ strokeB: Float,
    _ glowR: Float, _ glowG: Float, _ glowB: Float)

@_silgen_name("mfx_scroll_helix_segment_count")
private func mfx_helix_segment_count(_ state: UnsafeMutableRawPointer?) -> Int32

@_silgen_name("mfx_scroll_helix_get_segment")
private func mfx_helix_get_segment(
    _ state: UnsafeMutableRawPointer?, _ i: Int32,
    _ x1: UnsafeMutablePointer<Float>, _ y1: UnsafeMutablePointer<Float>,
    _ x2: UnsafeMutablePointer<Float>, _ y2: UnsafeMutablePointer<Float>,
    _ width: UnsafeMutablePointer<Float>,
    _ r: UnsafeMutablePointer<Float>, _ g: UnsafeMutablePointer<Float>,
    _ b: UnsafeMutablePointer<Float>, _ a: UnsafeMutablePointer<Float>)

@_silgen_name("mfx_scroll_helix_head_count")
private func mfx_helix_head_count(_ state: UnsafeMutableRawPointer?) -> Int32

@_silgen_name("mfx_scroll_helix_get_head")
private func mfx_helix_get_head(
    _ state: UnsafeMutableRawPointer?, _ i: Int32,
    _ x: UnsafeMutablePointer<Float>, _ y: UnsafeMutablePointer<Float>,
    _ alpha: UnsafeMutablePointer<Float>,
    _ strokeR: UnsafeMutablePointer<Float>, _ strokeG: UnsafeMutablePointer<Float>,
    _ strokeB: UnsafeMutablePointer<Float>)

@_silgen_name("mfx_macos_overlay_timer_interval_ms_v1")
private func mfx_macos_overlay_timer_interval_ms_v1(_ x: Int32, _ y: Int32) -> Int32

// MARK: - Swift draw data (value types, safe for deferred CA draw)

private struct HelixSegmentDraw {
    var x1: Float; var y1: Float; var x2: Float; var y2: Float
    var width: Float; var r: Float; var g: Float; var b: Float; var a: Float
}
private struct HelixHeadDraw {
    var x: Float; var y: Float; var alpha: Float
    var strokeR: Float; var strokeG: Float; var strokeB: Float
}

// MARK: - Drawing layer

private final class MfxHelixDrawLayer: CALayer {
    nonisolated(unsafe) var segments: [HelixSegmentDraw] = []
    nonisolated(unsafe) var heads: [HelixHeadDraw] = []

    override func draw(in ctx: CGContext) {
        ctx.setShouldAntialias(true)
        ctx.setLineCap(.round)
        ctx.setLineJoin(.round)

        for s in segments {
            guard s.a > 0.002 else { continue }
            let w = max(0.5, CGFloat(s.width))
            let auraA = s.a * 0.24
            if auraA > 0.002 {
                ctx.setStrokeColor(red: CGFloat(s.r), green: CGFloat(s.g), blue: CGFloat(s.b), alpha: CGFloat(auraA))
                ctx.setLineWidth(w + 2.1)
                ctx.move(to: CGPoint(x: CGFloat(s.x1), y: CGFloat(s.y1)))
                ctx.addLine(to: CGPoint(x: CGFloat(s.x2), y: CGFloat(s.y2)))
                ctx.strokePath()
            }
            ctx.setStrokeColor(red: CGFloat(s.r), green: CGFloat(s.g), blue: CGFloat(s.b), alpha: CGFloat(s.a))
            ctx.setLineWidth(w)
            ctx.move(to: CGPoint(x: CGFloat(s.x1), y: CGFloat(s.y1)))
            ctx.addLine(to: CGPoint(x: CGFloat(s.x2), y: CGFloat(s.y2)))
            ctx.strokePath()
        }

        for h in heads {
            guard h.alpha > 0.01 else { continue }
            let r: CGFloat = 2.0
            ctx.setFillColor(red: 1, green: 1, blue: 1, alpha: CGFloat(h.alpha * 0.82))
            ctx.fillEllipse(in: CGRect(x: CGFloat(h.x) - r, y: CGFloat(h.y) - r, width: r * 2, height: r * 2))
            let r2 = r * 1.85
            ctx.setFillColor(red: CGFloat(h.strokeR), green: CGFloat(h.strokeG), blue: CGFloat(h.strokeB), alpha: CGFloat(h.alpha * 0.18))
            ctx.fillEllipse(in: CGRect(x: CGFloat(h.x) - r2, y: CGFloat(h.y) - r2, width: r2 * 2, height: r2 * 2))
        }
    }
}

// MARK: - Animator

@MainActor
final class MfxScrollHelixAnimator {

    private let directionRad: Float
    private let intensity: Float
    private let startRadius: Float
    private let endRadius: Float
    private let strokeWidth: Float
    private let strokeR: Float, strokeG: Float, strokeB: Float
    private let glowR: Float, glowG: Float, glowB: Float
    private let sizePx: Int32
    private let durationSec: Double

    private let drawLayer: MfxHelixDrawLayer
    private nonisolated(unsafe) var timer: Timer?
    private var startTime: CFTimeInterval = 0.0
    private nonisolated(unsafe) var cppState: UnsafeMutableRawPointer?
    private let timerIntervalSec: Double

    init(
        parent: CALayer,
        center: CGPoint,
        direction: CGVector,
        size: CGFloat,
        startRadius: CGFloat,
        endRadius: CGFloat,
        strokeWidth: CGFloat,
        strokeArgb: UInt32,
        fillArgb: UInt32,
        baseOpacity: CGFloat,
        intensity: CGFloat,
        durationSec: Double,
        overlayX: Int32,
        overlayY: Int32
    ) {
        self.directionRad = Float(atan2(direction.dy, direction.dx)) + Float.pi
        self.intensity = Float(max(0, min(1, intensity)))
        self.startRadius = Float(max(0, startRadius))
        self.endRadius = Float(max(CGFloat(self.startRadius) + 1, endRadius))
        self.strokeWidth = Float(max(0.6, strokeWidth))
        self.sizePx = Int32(max(1, size))
        self.durationSec = max(0.08, durationSec)
        let intervalMs = max(4, min(1000, Int(mfx_macos_overlay_timer_interval_ms_v1(overlayX, overlayY))))
        self.timerIntervalSec = Double(intervalMs) / 1000.0

        let sc = Self.colorComponents(strokeArgb)
        self.strokeR = Float(sc.r); self.strokeG = Float(sc.g); self.strokeB = Float(sc.b)
        let gc = Self.colorComponents(fillArgb)
        self.glowR = Float(gc.r); self.glowG = Float(gc.g); self.glowB = Float(gc.b)

        self.drawLayer = MfxHelixDrawLayer()
        drawLayer.frame = parent.bounds
        drawLayer.contentsScale = NSScreen.main?.backingScaleFactor ?? 2.0
        drawLayer.isOpaque = false
        drawLayer.backgroundColor = nil
        parent.addSublayer(drawLayer)

        self.cppState = mfx_helix_state_create()
    }

    func start() {
        startTime = CACurrentMediaTime()
        let t = Timer(timeInterval: timerIntervalSec, repeats: true) { [weak self] _ in
            MainActor.assumeIsolated { self?.tick() }
        }
        RunLoop.main.add(t, forMode: .common)
        timer = t
    }

    func stop() {
        timer?.invalidate()
        timer = nil
        drawLayer.removeFromSuperlayer()
    }

    private func tick() {
        let elapsed = CACurrentMediaTime() - startTime
        if elapsed >= durationSec { stop(); return }

        let t = Float(elapsed / durationSec)
        mfx_helix_compute(cppState,
            t, UInt64(elapsed * 1000.0), sizePx,
            directionRad, intensity,
            startRadius, endRadius, strokeWidth,
            strokeR, strokeG, strokeB,
            glowR, glowG, glowB)

        // Pull segment data into Swift-owned arrays
        let segCount = Int(mfx_helix_segment_count(cppState))
        var segs = [HelixSegmentDraw]()
        segs.reserveCapacity(segCount)
        var x1: Float = 0, y1: Float = 0, x2: Float = 0, y2: Float = 0
        var w: Float = 0, r: Float = 0, g: Float = 0, b: Float = 0, a: Float = 0
        for i in 0..<Int32(segCount) {
            mfx_helix_get_segment(cppState, i, &x1, &y1, &x2, &y2, &w, &r, &g, &b, &a)
            segs.append(HelixSegmentDraw(x1: x1, y1: y1, x2: x2, y2: y2, width: w, r: r, g: g, b: b, a: a))
        }
        drawLayer.segments = segs

        let headCount = Int(mfx_helix_head_count(cppState))
        var hds = [HelixHeadDraw]()
        hds.reserveCapacity(headCount)
        var hx: Float = 0, hy: Float = 0, ha: Float = 0
        var sr: Float = 0, sg: Float = 0, sb: Float = 0
        for i in 0..<Int32(headCount) {
            mfx_helix_get_head(cppState, i, &hx, &hy, &ha, &sr, &sg, &sb)
            hds.append(HelixHeadDraw(x: hx, y: hy, alpha: ha, strokeR: sr, strokeG: sg, strokeB: sb))
        }
        drawLayer.heads = hds

        drawLayer.setNeedsDisplay()
    }

    deinit {
        timer?.invalidate()
        if let s = cppState { mfx_helix_state_destroy(s) }
    }

    private static func colorComponents(_ argb: UInt32) -> (r: CGFloat, g: CGFloat, b: CGFloat, a: CGFloat) {
        (CGFloat((argb >> 16) & 0xFF) / 255.0,
         CGFloat((argb >> 8) & 0xFF) / 255.0,
         CGFloat(argb & 0xFF) / 255.0,
         CGFloat((argb >> 24) & 0xFF) / 255.0)
    }
}

// MARK: - C-callable factory

@_cdecl("mfx_macos_scroll_helix_animator_create_v1")
public func mfx_macos_scroll_helix_animator_create_v1(
    _ parentHandle: UnsafeMutableRawPointer,
    _ centerX: Double, _ centerY: Double,
    _ dirDx: Double, _ dirDy: Double,
    _ overlayX: Int32, _ overlayY: Int32,
    _ size: Double,
    _ startRadius: Double, _ endRadius: Double, _ strokeWidth: Double,
    _ strokeArgb: UInt32, _ fillArgb: UInt32,
    _ baseOpacity: Double, _ intensity: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer? {
    let pBits = Int(bitPattern: parentHandle)
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateHelixAnimator(
                pBits, centerX, centerY, dirDx, dirDy,
                overlayX, overlayY,
                size, startRadius, endRadius, strokeWidth,
                strokeArgb, fillArgb, baseOpacity, intensity, durationSec))
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }
    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateHelixAnimator(
                pBits, centerX, centerY, dirDx, dirDy,
                overlayX, overlayY,
                size, startRadius, endRadius, strokeWidth,
                strokeArgb, fillArgb, baseOpacity, intensity, durationSec))
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@MainActor
private func mfxCreateHelixAnimator(
    _ parentBits: Int,
    _ centerX: Double, _ centerY: Double,
    _ dirDx: Double, _ dirDy: Double,
    _ overlayX: Int32, _ overlayY: Int32,
    _ size: Double,
    _ startRadius: Double, _ endRadius: Double, _ strokeWidth: Double,
    _ strokeArgb: UInt32, _ fillArgb: UInt32,
    _ baseOpacity: Double, _ intensity: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer? {
    guard let parentPtr = UnsafeMutableRawPointer(bitPattern: parentBits) else { return nil }
    let parentLayer = Unmanaged<CALayer>.fromOpaque(parentPtr).takeUnretainedValue()
    let animator = MfxScrollHelixAnimator(
        parent: parentLayer,
        center: CGPoint(x: centerX, y: centerY),
        direction: CGVector(dx: dirDx, dy: dirDy),
        size: CGFloat(size),
        startRadius: CGFloat(startRadius),
        endRadius: CGFloat(endRadius),
        strokeWidth: CGFloat(strokeWidth),
        strokeArgb: strokeArgb,
        fillArgb: fillArgb,
        baseOpacity: CGFloat(baseOpacity),
        intensity: CGFloat(intensity),
        durationSec: durationSec,
        overlayX: overlayX,
        overlayY: overlayY)
    animator.start()
    return Unmanaged.passRetained(animator).toOpaque()
}

@_cdecl("mfx_macos_scroll_animator_release_v1")
public func mfx_macos_scroll_animator_release_v1(_ handle: UnsafeMutableRawPointer?) {
    guard let handle else { return }
    Unmanaged<AnyObject>.fromOpaque(handle).release()
}
