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

private func mfxCreateClickStarPath(_ bounds: CGRect, points: Int) -> CGPath {
    let safePoints = max(4, points)
    let cx = bounds.midX
    let cy = bounds.midY
    let outerRadius = min(bounds.width, bounds.height) * 0.42
    let innerRadius = outerRadius * 0.46
    let startAngle = -CGFloat.pi * 0.5

    let path = CGMutablePath()
    for index in 0..<(safePoints * 2) {
        let radius = (index % 2 == 0) ? outerRadius : innerRadius
        let angle = startAngle + CGFloat(index) * CGFloat.pi / CGFloat(safePoints)
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

private func mfxMakeScaleFadeAnimationGroup(
    fromScale: CGFloat,
    toScale: CGFloat,
    fromOpacity: CGFloat,
    duration: Double
) -> CAAnimationGroup {
    let clampedDuration = max(0.05, duration)

    let scale = CABasicAnimation(keyPath: "transform.scale")
    scale.fromValue = fromScale
    scale.toValue = toScale
    scale.duration = clampedDuration
    scale.timingFunction = CAMediaTimingFunction(name: .easeOut)

    let fade = CABasicAnimation(keyPath: "opacity")
    fade.fromValue = mfxClamp(fromOpacity, min: 0.0, max: 1.0)
    fade.toValue = 0.0
    fade.duration = clampedDuration
    fade.timingFunction = CAMediaTimingFunction(name: .easeIn)

    let group = CAAnimationGroup()
    group.animations = [scale, fade]
    group.duration = clampedDuration
    group.fillMode = .forwards
    group.isRemovedOnCompletion = false
    return group
}

private func mfxNormalizeClickType(_ normalizedTypeUtf8: UnsafePointer<CChar>?) -> String {
    guard
        let normalizedTypeUtf8,
        let value = String(validatingCString: normalizedTypeUtf8)
    else {
        return "ripple"
    }
    let lowered = value.lowercased()
    if lowered == "text" || lowered == "star" {
        return lowered
    }
    return "ripple"
}

private func mfxNormalizeTextLabel(_ textLabelUtf8: UnsafePointer<CChar>?) -> String {
    guard
        let textLabelUtf8,
        let label = String(validatingCString: textLabelUtf8),
        !label.isEmpty
    else {
        return "LEFT"
    }
    return label
}

@MainActor
private func mfxCreateClickPulseOverlayOnMainThread(
    frameX: Double,
    frameY: Double,
    frameSize: Double,
    inset: Double,
    overlayX: Int32,
    overlayY: Int32,
    normalizedType: String,
    fillArgb: UInt32,
    strokeArgb: UInt32,
    baseOpacity: Double,
    animationDurationSec: Double,
    textLabel: String
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

    guard let contentLayer = content.layer else {
        return windowHandle
    }

    let textMode = normalizedType == "text"
    let starMode = normalizedType == "star"
    let sizeCGFloat = CGFloat(size)
    let safeInset = mfxClamp(CGFloat(inset), min: 0.0, max: max(0.0, sizeCGFloat * 0.48))
    let safeRingSize = max(1.0, sizeCGFloat - safeInset * 2.0)
    let baseOpacityCGFloat = CGFloat(baseOpacity)

    let base = CAShapeLayer()
    base.frame = content.bounds
    base.path = CGPath(
        ellipseIn: CGRect(x: safeInset, y: safeInset, width: safeRingSize, height: safeRingSize),
        transform: nil
    )
    base.fillColor = mfxColorFromArgb(fillArgb).cgColor
    base.strokeColor = mfxColorFromArgb(strokeArgb).cgColor
    base.lineWidth = mfxScaleMetric(
        sizeCGFloat,
        textMode ? 2.1 : 2.4,
        160.0,
        1.2,
        4.8
    )
    base.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.0, 0.0))
    contentLayer.addSublayer(base)

    if starMode {
        let star = CAShapeLayer()
        star.frame = content.bounds
        let starInset = mfxScaleMetric(sizeCGFloat, 38.0, 160.0, 18.0, 74.0)
        let starBounds = content.bounds.insetBy(dx: starInset, dy: starInset)
        star.path = mfxCreateClickStarPath(starBounds, points: 5)
        star.fillColor = mfxColorFromArgb(strokeArgb).cgColor
        star.strokeColor = mfxColorFromArgb(strokeArgb).cgColor
        star.lineWidth = mfxScaleMetric(sizeCGFloat, 1.0, 160.0, 0.8, 2.2)
        star.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.03, 0.0))
        contentLayer.addSublayer(star)
    }

    if textMode {
        let text = CATextLayer()
        let textHeight = mfxScaleMetric(sizeCGFloat, 36.0, 160.0, 24.0, 60.0)
        text.frame = CGRect(x: 0.0, y: sizeCGFloat * 0.30, width: sizeCGFloat, height: textHeight)
        text.alignmentMode = .center
        text.foregroundColor = mfxColorFromArgb(strokeArgb).cgColor
        text.contentsScale = max(1.0, contentLayer.contentsScale)
        let fontSize = mfxScaleMetric(sizeCGFloat, 24.0, 160.0, 14.0, 42.0)
        text.fontSize = fontSize
        text.font = NSFont.boldSystemFont(ofSize: fontSize).fontName as CFTypeRef
        text.string = textLabel
        text.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.03, 0.0))
        contentLayer.addSublayer(text)
    }

    let group = mfxMakeScaleFadeAnimationGroup(
        fromScale: textMode ? 0.75 : 0.15,
        toScale: 1.0,
        fromOpacity: baseOpacityCGFloat,
        duration: animationDurationSec
    )
    base.add(group, forKey: "mfx_click_pulse")

    return windowHandle
}

@_cdecl("mfx_macos_click_pulse_overlay_create_v1")
public func mfx_macos_click_pulse_overlay_create_v1(
    _ frameX: Double,
    _ frameY: Double,
    _ frameSize: Double,
    _ inset: Double,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ normalizedTypeUtf8: UnsafePointer<CChar>?,
    _ fillArgb: UInt32,
    _ strokeArgb: UInt32,
    _ baseOpacity: Double,
    _ animationDurationSec: Double,
    _ textLabelUtf8: UnsafePointer<CChar>?
) -> UnsafeMutableRawPointer? {
    let normalizedType = mfxNormalizeClickType(normalizedTypeUtf8)
    let textLabel = mfxNormalizeTextLabel(textLabelUtf8)

    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateClickPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    inset: inset,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    normalizedType: normalizedType,
                    fillArgb: fillArgb,
                    strokeArgb: strokeArgb,
                    baseOpacity: baseOpacity,
                    animationDurationSec: animationDurationSec,
                    textLabel: textLabel
                )
            )
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateClickPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    inset: inset,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    normalizedType: normalizedType,
                    fillArgb: fillArgb,
                    strokeArgb: strokeArgb,
                    baseOpacity: baseOpacity,
                    animationDurationSec: animationDurationSec,
                    textLabel: textLabel
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}
