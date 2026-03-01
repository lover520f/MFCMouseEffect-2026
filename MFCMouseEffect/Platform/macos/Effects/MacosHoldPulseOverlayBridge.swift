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

private enum HoldStyleCode: Int32 {
    case charge = 0
    case lightning = 1
    case hex = 2
    case techRing = 3
    case hologram = 4
    case neon = 5
    case quantumHalo = 6
    case fluxField = 7
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

private func mfxResolveOpacity(_ baseOpacity: CGFloat, _ delta: CGFloat, _ minOpacity: CGFloat) -> CGFloat {
    let clamped = mfxClamp(baseOpacity + delta, min: 0.0, max: 1.0)
    let floor = mfxClamp(minOpacity, min: 0.0, max: 1.0)
    return max(clamped, floor)
}

private func mfxScaleMetric(
    _ referenceSize: CGFloat,
    _ baseValue: CGFloat,
    _ baseReference: CGFloat,
    _ minValue: CGFloat,
    _ maxValue: CGFloat
) -> CGFloat {
    let safeReference = max(1.0, baseReference)
    let safeSize = max(1.0, referenceSize)
    let scaled = baseValue * (safeSize / safeReference)
    return mfxClamp(scaled, min: minValue, max: maxValue)
}

private func mfxColorFromArgb(_ argb: UInt32) -> NSColor {
    let alpha = CGFloat(Double((argb >> 24) & 0xFF) / 255.0)
    let red = CGFloat(Double((argb >> 16) & 0xFF) / 255.0)
    let green = CGFloat(Double((argb >> 8) & 0xFF) / 255.0)
    let blue = CGFloat(Double(argb & 0xFF) / 255.0)
    return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
}

private func mfxCreateHexPath(_ bounds: CGRect) -> CGPath {
    let path = CGMutablePath()
    let cx = bounds.midX
    let cy = bounds.midY
    let radius = min(bounds.width, bounds.height) * 0.42
    for index in 0..<6 {
        let angle = CGFloat.pi / 3.0 * CGFloat(index) - CGFloat.pi * 0.5
        let x = cx + cos(angle) * radius
        let y = cy + sin(angle) * radius
        let point = CGPoint(x: x, y: y)
        if index == 0 {
            path.move(to: point)
        } else {
            path.addLine(to: point)
        }
    }
    path.closeSubpath()
    return path
}

private func mfxCreateLightningPath(_ bounds: CGRect) -> CGPath {
    let cx = bounds.midX
    let cy = bounds.midY
    let h = bounds.height * 0.40
    let path = CGMutablePath()
    path.move(to: CGPoint(x: cx - 6.0, y: cy + h * 0.45))
    path.addLine(to: CGPoint(x: cx + 2.0, y: cy + h * 0.10))
    path.addLine(to: CGPoint(x: cx - 1.5, y: cy + h * 0.10))
    path.addLine(to: CGPoint(x: cx + 6.0, y: cy - h * 0.45))
    path.addLine(to: CGPoint(x: cx - 2.0, y: cy - h * 0.05))
    path.addLine(to: CGPoint(x: cx + 1.5, y: cy - h * 0.05))
    path.closeSubpath()
    return path
}

private func mfxCreateFluxFieldPath(_ bounds: CGRect) -> CGPath {
    let cx = bounds.midX
    let cy = bounds.midY
    let r = min(bounds.width, bounds.height) * 0.36
    let path = CGMutablePath()
    path.move(to: CGPoint(x: cx - r, y: cy))
    path.addLine(to: CGPoint(x: cx + r, y: cy))
    path.move(to: CGPoint(x: cx, y: cy - r))
    path.addLine(to: CGPoint(x: cx, y: cy + r))
    return path
}

private func mfxConfigureHoldAccentLayer(
    _ accent: CAShapeLayer,
    bounds: CGRect,
    holdStyle: HoldStyleCode,
    baseColor: NSColor
) {
    switch holdStyle {
    case .hex:
        accent.path = mfxCreateHexPath(bounds.insetBy(dx: 38.0, dy: 38.0))
        accent.fillColor = NSColor.clear.cgColor
        accent.strokeColor = baseColor.cgColor
        accent.lineWidth = 1.8
    case .lightning:
        accent.path = mfxCreateLightningPath(bounds.insetBy(dx: 36.0, dy: 36.0))
        accent.fillColor = baseColor.cgColor
        accent.strokeColor = baseColor.cgColor
        accent.lineWidth = 1.0
    case .fluxField:
        accent.path = mfxCreateFluxFieldPath(bounds.insetBy(dx: 36.0, dy: 36.0))
        accent.fillColor = NSColor.clear.cgColor
        accent.strokeColor = baseColor.cgColor
        accent.lineWidth = 2.0
    case .quantumHalo:
        accent.path = CGPath(ellipseIn: bounds.insetBy(dx: 36.0, dy: 36.0), transform: nil)
        accent.fillColor = NSColor.clear.cgColor
        accent.strokeColor = baseColor.cgColor
        accent.lineWidth = 2.2
    default:
        accent.path = CGPath(ellipseIn: bounds.insetBy(dx: 44.0, dy: 44.0), transform: nil)
        accent.fillColor = NSColor.clear.cgColor
        accent.strokeColor = baseColor.withAlphaComponent(0.85).cgColor
        accent.lineWidth = 1.4
        accent.lineDashPattern = [6, 6]
    }
}

@MainActor
private func mfxCreateHoldPulseOverlayOnMainThread(
    frameX: Double,
    frameY: Double,
    frameSize: Double,
    overlayX: Int32,
    overlayY: Int32,
    baseStrokeArgb: UInt32,
    holdStyleCode: Int32,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationSec: Double,
    rotateDurationFastSec: Double
) -> (UInt, UInt, UInt) {
    let size = max(1.0, frameSize)
    guard let windowHandle = mfx_macos_overlay_create_window_v1(frameX, frameY, size, size) else {
        return (0, 0, 0)
    }
    let window = Unmanaged<NSWindow>.fromOpaque(windowHandle).takeUnretainedValue()
    guard let content = window.contentView else {
        return (UInt(bitPattern: windowHandle), 0, 0)
    }
    content.wantsLayer = true
    mfx_macos_overlay_apply_content_scale_v1(
        Unmanaged.passUnretained(content).toOpaque(),
        overlayX,
        overlayY
    )

    guard let contentLayer = content.layer else {
        return (UInt(bitPattern: windowHandle), 0, 0)
    }

    let style = HoldStyleCode(rawValue: holdStyleCode) ?? .charge
    let baseColor = mfxColorFromArgb(baseStrokeArgb)
    let sizeCGFloat = CGFloat(size)
    let ringInset = mfxScaleMetric(sizeCGFloat, 24.0, 160.0, 10.0, 44.0)
    let ringLineWidth = mfxScaleMetric(sizeCGFloat, 2.4, 160.0, 1.2, 4.8)

    let ring = CAShapeLayer()
    ring.frame = content.bounds
    ring.path = CGPath(ellipseIn: content.bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    ring.fillColor = baseColor.withAlphaComponent(0.16).cgColor
    ring.strokeColor = baseColor.cgColor
    ring.lineWidth = ringLineWidth
    ring.opacity = Float(mfxResolveOpacity(CGFloat(baseOpacity), 0.0, 0.0))
    contentLayer.addSublayer(ring)

    let accent = CAShapeLayer()
    accent.frame = content.bounds
    mfxConfigureHoldAccentLayer(accent, bounds: content.bounds, holdStyle: style, baseColor: baseColor)
    accent.opacity = Float(mfxResolveOpacity(CGFloat(baseOpacity), -0.06, 0.1))
    contentLayer.addSublayer(accent)

    let breathe = CABasicAnimation(keyPath: "opacity")
    breathe.fromValue = 0.35
    breathe.toValue = mfxResolveOpacity(CGFloat(baseOpacity), 0.03, 0.0)
    breathe.duration = max(0.05, breatheDurationSec)
    breathe.autoreverses = true
    breathe.repeatCount = .greatestFiniteMagnitude
    ring.add(breathe, forKey: "mfx_hold_breathe")

    let spin = CABasicAnimation(keyPath: "transform.rotation")
    spin.fromValue = 0.0
    spin.toValue = Double.pi * 2.0
    let spinDuration = (style == .quantumHalo || style == .fluxField)
        ? rotateDurationFastSec
        : rotateDurationSec
    spin.duration = max(0.05, spinDuration)
    spin.repeatCount = .greatestFiniteMagnitude
    accent.add(spin, forKey: "mfx_hold_spin")

    return (
        UInt(bitPattern: windowHandle),
        UInt(bitPattern: Unmanaged.passUnretained(ring).toOpaque()),
        UInt(bitPattern: Unmanaged.passUnretained(accent).toOpaque())
    )
}

@MainActor
private func mfxUpdateHoldPulseOverlayOnMainThread(
    windowHandle: UnsafeMutableRawPointer?,
    ringLayerHandle: UnsafeMutableRawPointer?,
    accentLayerHandle: UnsafeMutableRawPointer?,
    frameOriginX: Double,
    frameOriginY: Double,
    overlayX: Int32,
    overlayY: Int32,
    baseOpacity: Double,
    progressFullMs: UInt32,
    holdMs: UInt32
) {
    guard
        let windowHandle,
        let ringLayerHandle
    else {
        return
    }
    let window = Unmanaged<NSWindow>.fromOpaque(windowHandle).takeUnretainedValue()
    let ring = Unmanaged<CAShapeLayer>.fromOpaque(ringLayerHandle).takeUnretainedValue()
    let accent = accentLayerHandle.map { Unmanaged<CAShapeLayer>.fromOpaque($0).takeUnretainedValue() }

    window.setFrameOrigin(NSPoint(x: frameOriginX, y: frameOriginY))
    if let content = window.contentView {
        mfx_macos_overlay_apply_content_scale_v1(
            Unmanaged.passUnretained(content).toOpaque(),
            overlayX,
            overlayY
        )
    }

    let frame = window.frame
    let width = frame.width
    let progressDenominator = max(1.0, CGFloat(progressFullMs))
    let progress = min(1.0, CGFloat(holdMs) / progressDenominator)

    let scale = 1.0 + progress * 0.20
    ring.transform = CATransform3DMakeScale(scale, scale, 1.0)
    let baseLineWidth = mfxScaleMetric(width, 2.4, 160.0, 1.2, 4.8)
    let progressLineDelta = mfxScaleMetric(width, 1.4, 160.0, 0.7, 3.0)
    ring.lineWidth = baseLineWidth + progress * progressLineDelta

    let baseOpacityValue = CGFloat(baseOpacity)
    ring.opacity = Float(mfxResolveOpacity(baseOpacityValue, -0.18 + progress * 0.20, 0.2))
    if let accent {
        accent.opacity = Float(mfxResolveOpacity(baseOpacityValue, -0.35 + progress * 0.35, 0.15))
    }
}

@_cdecl("mfx_macos_hold_pulse_overlay_create_v1")
public func mfx_macos_hold_pulse_overlay_create_v1(
    _ frameX: Double,
    _ frameY: Double,
    _ frameSize: Double,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ baseStrokeArgb: UInt32,
    _ holdStyleCode: Int32,
    _ baseOpacity: Double,
    _ breatheDurationSec: Double,
    _ rotateDurationSec: Double,
    _ rotateDurationFastSec: Double,
    _ ringLayerOut: UnsafeMutablePointer<UnsafeMutableRawPointer?>?,
    _ accentLayerOut: UnsafeMutablePointer<UnsafeMutableRawPointer?>?
) -> UnsafeMutableRawPointer? {
    let bits: (UInt, UInt, UInt)
    if Thread.isMainThread {
        bits = MainActor.assumeIsolated {
            mfxCreateHoldPulseOverlayOnMainThread(
                frameX: frameX,
                frameY: frameY,
                frameSize: frameSize,
                overlayX: overlayX,
                overlayY: overlayY,
                baseStrokeArgb: baseStrokeArgb,
                holdStyleCode: holdStyleCode,
                baseOpacity: baseOpacity,
                breatheDurationSec: breatheDurationSec,
                rotateDurationSec: rotateDurationSec,
                rotateDurationFastSec: rotateDurationFastSec
            )
        }
    } else {
        var mainBits: (UInt, UInt, UInt) = (0, 0, 0)
        DispatchQueue.main.sync {
            mainBits = MainActor.assumeIsolated {
                mfxCreateHoldPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    baseStrokeArgb: baseStrokeArgb,
                    holdStyleCode: holdStyleCode,
                    baseOpacity: baseOpacity,
                    breatheDurationSec: breatheDurationSec,
                    rotateDurationSec: rotateDurationSec,
                    rotateDurationFastSec: rotateDurationFastSec
                )
            }
        }
        bits = mainBits
    }

    if let ringLayerOut {
        ringLayerOut.pointee = UnsafeMutableRawPointer(bitPattern: bits.1)
    }
    if let accentLayerOut {
        accentLayerOut.pointee = UnsafeMutableRawPointer(bitPattern: bits.2)
    }
    return UnsafeMutableRawPointer(bitPattern: bits.0)
}

@_cdecl("mfx_macos_hold_pulse_overlay_update_v1")
public func mfx_macos_hold_pulse_overlay_update_v1(
    _ windowHandle: UnsafeMutableRawPointer?,
    _ ringLayerHandle: UnsafeMutableRawPointer?,
    _ accentLayerHandle: UnsafeMutableRawPointer?,
    _ frameOriginX: Double,
    _ frameOriginY: Double,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ baseOpacity: Double,
    _ progressFullMs: UInt32,
    _ holdMs: UInt32
) {
    let windowBits = UInt(bitPattern: windowHandle)
    let ringBits = UInt(bitPattern: ringLayerHandle)
    let accentBits = UInt(bitPattern: accentLayerHandle)

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxUpdateHoldPulseOverlayOnMainThread(
                windowHandle: UnsafeMutableRawPointer(bitPattern: windowBits),
                ringLayerHandle: UnsafeMutableRawPointer(bitPattern: ringBits),
                accentLayerHandle: UnsafeMutableRawPointer(bitPattern: accentBits),
                frameOriginX: frameOriginX,
                frameOriginY: frameOriginY,
                overlayX: overlayX,
                overlayY: overlayY,
                baseOpacity: baseOpacity,
                progressFullMs: progressFullMs,
                holdMs: holdMs
            )
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxUpdateHoldPulseOverlayOnMainThread(
                windowHandle: UnsafeMutableRawPointer(bitPattern: windowBits),
                ringLayerHandle: UnsafeMutableRawPointer(bitPattern: ringBits),
                accentLayerHandle: UnsafeMutableRawPointer(bitPattern: accentBits),
                frameOriginX: frameOriginX,
                frameOriginY: frameOriginY,
                overlayX: overlayX,
                overlayY: overlayY,
                baseOpacity: baseOpacity,
                progressFullMs: progressFullMs,
                holdMs: holdMs
            )
        }
    }
}
