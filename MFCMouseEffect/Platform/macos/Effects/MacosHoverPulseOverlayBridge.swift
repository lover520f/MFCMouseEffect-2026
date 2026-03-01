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
    tubesMode: Bool
) -> UnsafeMutableRawPointer? {
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
    mfx_macos_overlay_apply_content_scale_v1(
        Unmanaged.passUnretained(content).toOpaque(),
        overlayX,
        overlayY
    )

    let contentBounds = content.bounds
    let sizeCGFloat = CGFloat(size)
    let baseOpacityCGFloat = CGFloat(baseOpacity)

    let ring = CAShapeLayer()
    ring.frame = contentBounds
    let ringInset = mfxScaleMetric(sizeCGFloat, 20.0, 160.0, 10.0, 40.0)
    ring.path = CGPath(ellipseIn: contentBounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    ring.fillColor = mfxColorFromArgb(glowFillArgb).cgColor
    ring.strokeColor = mfxColorFromArgb(glowStrokeArgb).cgColor
    ring.lineWidth = mfxScaleMetric(sizeCGFloat, 2.0, 160.0, 1.0, 4.2)
    ring.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.0, 0.0))
    content.layer?.addSublayer(ring)

    let breathe = CABasicAnimation(keyPath: "opacity")
    breathe.fromValue = 0.25
    breathe.toValue = mfxResolveOpacity(baseOpacityCGFloat, 0.05, 0.0)
    breathe.duration = max(0.05, breatheDurationSec)
    breathe.autoreverses = true
    breathe.repeatCount = .greatestFiniteMagnitude
    ring.add(breathe, forKey: "mfx_hover_breathe")

    if tubesMode {
        let ring2 = CAShapeLayer()
        ring2.frame = contentBounds
        let ringInset2 = mfxScaleMetric(sizeCGFloat, 28.0, 160.0, 14.0, 52.0)
        ring2.path = CGPath(ellipseIn: contentBounds.insetBy(dx: ringInset2, dy: ringInset2), transform: nil)
        ring2.fillColor = NSColor.clear.cgColor
        ring2.strokeColor = mfxColorFromArgb(tubesStrokeArgb).cgColor
        ring2.lineWidth = mfxScaleMetric(sizeCGFloat, 1.6, 160.0, 0.8, 2.8)
        ring2.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, -0.12, 0.0))
        content.layer?.addSublayer(ring2)

        let spin = CABasicAnimation(keyPath: "transform.rotation")
        spin.fromValue = 0.0
        spin.toValue = Double.pi * 2.0
        spin.duration = max(0.05, tubesSpinDurationSec)
        spin.repeatCount = .greatestFiniteMagnitude
        ring2.add(spin, forKey: "mfx_hover_spin")
    }

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
    _ tubesMode: Int32
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
                    tubesMode: tubesMode != 0
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
                    tubesMode: tubesMode != 0
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}
