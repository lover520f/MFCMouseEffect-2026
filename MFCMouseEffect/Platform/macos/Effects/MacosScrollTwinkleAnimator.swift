/// MacosScrollTwinkleAnimator.swift
/// Thin per-frame twinkle particle renderer for macOS.
/// Particle physics delegates to shared C++ Core via opaque-handle C bridge.
/// Swift side: Timer driving + CGContext drawing.

@preconcurrency import AppKit
@preconcurrency import QuartzCore
@preconcurrency import Foundation

// MARK: - C bridge declarations (opaque-handle pattern)

@_silgen_name("mfx_scroll_twinkle_state_create")
private func mfx_twinkle_state_create() -> UnsafeMutableRawPointer?

@_silgen_name("mfx_scroll_twinkle_state_destroy")
private func mfx_twinkle_state_destroy(_ state: UnsafeMutableRawPointer?)

@_silgen_name("mfx_scroll_twinkle_state_start")
private func mfx_twinkle_state_start(
    _ state: UnsafeMutableRawPointer?, _ cx: Float, _ cy: Float, _ dirRad: Float, _ intensity: Float)

@_silgen_name("mfx_scroll_twinkle_compute")
private func mfx_twinkle_compute(
    _ state: UnsafeMutableRawPointer?,
    _ t: Float, _ elapsedMs: UInt64, _ sizePx: Int32,
    _ strokeR: Float, _ strokeG: Float, _ strokeB: Float,
    _ glowR: Float, _ glowG: Float, _ glowB: Float)

@_silgen_name("mfx_scroll_twinkle_particle_count")
private func mfx_twinkle_particle_count(_ state: UnsafeMutableRawPointer?) -> Int32

@_silgen_name("mfx_scroll_twinkle_get_particle")
private func mfx_twinkle_get_particle(
    _ state: UnsafeMutableRawPointer?, _ i: Int32,
    _ x: UnsafeMutablePointer<Float>, _ y: UnsafeMutablePointer<Float>,
    _ prevX: UnsafeMutablePointer<Float>, _ prevY: UnsafeMutablePointer<Float>,
    _ drawSize: UnsafeMutablePointer<Float>,
    _ coreR: UnsafeMutablePointer<Float>, _ coreG: UnsafeMutablePointer<Float>,
    _ coreB: UnsafeMutablePointer<Float>, _ coreAlpha: UnsafeMutablePointer<Float>,
    _ trailR: UnsafeMutablePointer<Float>, _ trailG: UnsafeMutablePointer<Float>,
    _ trailB: UnsafeMutablePointer<Float>, _ trailAlpha: UnsafeMutablePointer<Float>,
    _ glowAlpha: UnsafeMutablePointer<Float>)

@_silgen_name("mfx_macos_overlay_timer_interval_ms_v1")
private func mfx_macos_overlay_timer_interval_ms_v1(_ x: Int32, _ y: Int32) -> Int32

// MARK: - Swift draw data

private struct TwinkleParticleDraw {
    var x: Float; var y: Float; var prevX: Float; var prevY: Float; var drawSize: Float
    var coreR: Float; var coreG: Float; var coreB: Float; var coreAlpha: Float
    var trailR: Float; var trailG: Float; var trailB: Float; var trailAlpha: Float
    var glowAlpha: Float
}

// MARK: - Drawing layer

private final class MfxTwinkleDrawLayer: CALayer {
    nonisolated(unsafe) var particles: [TwinkleParticleDraw] = []

    override func draw(in ctx: CGContext) {
        ctx.setShouldAntialias(true)
        ctx.setLineCap(.round)
        ctx.setLineJoin(.round)

        for p in particles {
            guard p.coreAlpha > 0.005 else { continue }

            // Trail line
            let tw = max(0.75, CGFloat(p.drawSize * 0.26))
            ctx.setStrokeColor(red: CGFloat(p.trailR), green: CGFloat(p.trailG), blue: CGFloat(p.trailB), alpha: CGFloat(p.trailAlpha))
            ctx.setLineWidth(tw)
            ctx.move(to: CGPoint(x: CGFloat(p.prevX), y: CGFloat(p.prevY)))
            ctx.addLine(to: CGPoint(x: CGFloat(p.x), y: CGFloat(p.y)))
            ctx.strokePath()

            // Glow circle
            if p.glowAlpha > 0.005 {
                let gs = CGFloat(p.drawSize * 1.22)
                ctx.setFillColor(red: CGFloat(p.trailR), green: CGFloat(p.trailG), blue: CGFloat(p.trailB), alpha: CGFloat(p.glowAlpha))
                ctx.fillEllipse(in: CGRect(x: CGFloat(p.x) - gs * 0.5, y: CGFloat(p.y) - gs * 0.5, width: gs, height: gs))
            }

            // Core dot
            ctx.setFillColor(red: CGFloat(p.coreR), green: CGFloat(p.coreG), blue: CGFloat(p.coreB), alpha: CGFloat(p.coreAlpha))
            let ds = CGFloat(p.drawSize)
            ctx.fillEllipse(in: CGRect(x: CGFloat(p.x) - ds * 0.5, y: CGFloat(p.y) - ds * 0.5, width: ds, height: ds))
        }
    }
}

// MARK: - Animator

@MainActor
final class MfxScrollTwinkleAnimator {

    private let dirRad: Float
    private let intensity: Float
    private let strokeR: Float, strokeG: Float, strokeB: Float
    private let glowR: Float, glowG: Float, glowB: Float
    private let sizePx: Int32
    private let durationSec: Double
    private let cx: Float, cy: Float

    private nonisolated(unsafe) var cppState: UnsafeMutableRawPointer?
    private let drawLayer: MfxTwinkleDrawLayer
    private nonisolated(unsafe) var timer: Timer?
    private var startTime: CFTimeInterval = 0.0
    private let timerIntervalSec: Double

    init(
        parent: CALayer,
        center: CGPoint,
        direction: CGVector,
        size: CGFloat,
        strokeArgb: UInt32,
        fillArgb: UInt32,
        baseOpacity: CGFloat,
        intensity: CGFloat,
        strokeWidth: CGFloat,
        durationSec: Double,
        overlayX: Int32,
        overlayY: Int32
    ) {
        self.dirRad = Float(atan2(direction.dy, direction.dx))
        self.intensity = Float(max(0, min(1, intensity)))
        self.sizePx = Int32(max(1, size))
        self.durationSec = max(0.08, durationSec)
        self.cx = Float(center.x)
        self.cy = Float(center.y)
        let intervalMs = max(4, min(1000, Int(mfx_macos_overlay_timer_interval_ms_v1(overlayX, overlayY))))
        self.timerIntervalSec = Double(intervalMs) / 1000.0

        let sc = Self.colorComponents(strokeArgb)
        self.strokeR = Float(sc.r); self.strokeG = Float(sc.g); self.strokeB = Float(sc.b)
        let gc = Self.colorComponents(fillArgb)
        self.glowR = Float(gc.r); self.glowG = Float(gc.g); self.glowB = Float(gc.b)

        self.drawLayer = MfxTwinkleDrawLayer()
        drawLayer.frame = parent.bounds
        drawLayer.contentsScale = NSScreen.main?.backingScaleFactor ?? 2.0
        drawLayer.isOpaque = false
        drawLayer.backgroundColor = nil
        parent.addSublayer(drawLayer)

        self.cppState = mfx_twinkle_state_create()
        mfx_twinkle_state_start(self.cppState, self.cx, self.cy, self.dirRad, self.intensity)
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
        mfx_twinkle_compute(cppState,
            t, UInt64(elapsed * 1000.0), sizePx,
            strokeR, strokeG, strokeB,
            glowR, glowG, glowB)

        let count = Int(mfx_twinkle_particle_count(cppState))
        var pts = [TwinkleParticleDraw]()
        pts.reserveCapacity(count)
        var x: Float = 0, y: Float = 0, px: Float = 0, py: Float = 0, ds: Float = 0
        var cr: Float = 0, cg: Float = 0, cb: Float = 0, ca: Float = 0
        var tr: Float = 0, tg: Float = 0, tb: Float = 0, ta: Float = 0
        var ga: Float = 0
        for i in 0..<Int32(count) {
            mfx_twinkle_get_particle(cppState, i,
                &x, &y, &px, &py, &ds,
                &cr, &cg, &cb, &ca,
                &tr, &tg, &tb, &ta, &ga)
            pts.append(TwinkleParticleDraw(
                x: x, y: y, prevX: px, prevY: py, drawSize: ds,
                coreR: cr, coreG: cg, coreB: cb, coreAlpha: ca,
                trailR: tr, trailG: tg, trailB: tb, trailAlpha: ta,
                glowAlpha: ga))
        }
        drawLayer.particles = pts
        drawLayer.setNeedsDisplay()
    }

    deinit {
        timer?.invalidate()
        if let s = cppState { mfx_twinkle_state_destroy(s) }
    }

    private static func colorComponents(_ argb: UInt32) -> (r: CGFloat, g: CGFloat, b: CGFloat, a: CGFloat) {
        (CGFloat((argb >> 16) & 0xFF) / 255.0,
         CGFloat((argb >> 8) & 0xFF) / 255.0,
         CGFloat(argb & 0xFF) / 255.0,
         CGFloat((argb >> 24) & 0xFF) / 255.0)
    }
}

// MARK: - C-callable factory

@_cdecl("mfx_macos_scroll_twinkle_animator_create_v1")
public func mfx_macos_scroll_twinkle_animator_create_v1(
    _ parentHandle: UnsafeMutableRawPointer,
    _ centerX: Double, _ centerY: Double,
    _ dirDx: Double, _ dirDy: Double,
    _ overlayX: Int32, _ overlayY: Int32,
    _ size: Double,
    _ strokeArgb: UInt32, _ fillArgb: UInt32,
    _ baseOpacity: Double, _ intensity: Double,
    _ strokeWidth: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer? {
    let pBits = Int(bitPattern: parentHandle)
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateTwinkleAnimator(
                pBits, centerX, centerY, dirDx, dirDy,
                overlayX, overlayY,
                size, strokeArgb, fillArgb, baseOpacity, intensity,
                strokeWidth, durationSec))
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }
    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateTwinkleAnimator(
                pBits, centerX, centerY, dirDx, dirDy,
                overlayX, overlayY,
                size, strokeArgb, fillArgb, baseOpacity, intensity,
                strokeWidth, durationSec))
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@MainActor
private func mfxCreateTwinkleAnimator(
    _ parentBits: Int,
    _ centerX: Double, _ centerY: Double,
    _ dirDx: Double, _ dirDy: Double,
    _ overlayX: Int32, _ overlayY: Int32,
    _ size: Double,
    _ strokeArgb: UInt32, _ fillArgb: UInt32,
    _ baseOpacity: Double, _ intensity: Double,
    _ strokeWidth: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer? {
    guard let parentPtr = UnsafeMutableRawPointer(bitPattern: parentBits) else { return nil }
    let parentLayer = Unmanaged<CALayer>.fromOpaque(parentPtr).takeUnretainedValue()
    let animator = MfxScrollTwinkleAnimator(
        parent: parentLayer,
        center: CGPoint(x: centerX, y: centerY),
        direction: CGVector(dx: dirDx, dy: dirDy),
        size: CGFloat(size),
        strokeArgb: strokeArgb,
        fillArgb: fillArgb,
        baseOpacity: CGFloat(baseOpacity),
        intensity: CGFloat(intensity),
        strokeWidth: CGFloat(strokeWidth),
        durationSec: durationSec,
        overlayX: overlayX,
        overlayY: overlayY)
    animator.start()
    return Unmanaged.passRetained(animator).toOpaque()
}
