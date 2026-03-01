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

private func mfxCreateScaleFadeAnimationGroup(
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

private func mfxNormalizeTrailType(_ normalizedTypeUtf8: UnsafePointer<CChar>?) -> String {
    guard
        let normalizedTypeUtf8,
        let value = String(validatingCString: normalizedTypeUtf8)
    else {
        return "none"
    }
    let lowered = value.lowercased()
    if lowered.isEmpty {
        return "none"
    }
    if lowered == "none" {
        return "none"
    }
    if lowered == "line" ||
        lowered == "meteor" ||
        lowered == "streamer" ||
        lowered == "electric" ||
        lowered == "tubes" ||
        lowered == "particle" {
        return lowered
    }
    return "none"
}

private func mfxCreateTrailLinePath(
    bounds: CGRect,
    deltaX: CGFloat,
    deltaY: CGFloat,
    trailType: String
) -> CGPath {
    if trailType == "none" {
        return CGMutablePath()
    }
    let cx = bounds.midX
    let cy = bounds.midY
    var dx = deltaX
    var dy = deltaY
    let distance = max(0.0, sqrt(dx * dx + dy * dy))

    let lineTrail = (trailType == "line")
    let electricTrail = (trailType == "electric")
    let meteorTrail = (trailType == "meteor")
    let streamerTrail = (trailType == "streamer")
    let minSegment: CGFloat = lineTrail ? 1.0 : (meteorTrail ? 18.0 : (streamerTrail ? 4.0 : 10.0))
    let maxSegment: CGFloat = lineTrail ? .greatestFiniteMagnitude : (meteorTrail ? 240.0 : (streamerTrail ? 90.0 : 160.0))

    if distance < 0.0001 {
        dx = minSegment
        dy = 0.0
    } else {
        let targetLength = lineTrail ? distance : mfxClamp(distance, min: minSegment, max: maxSegment)
        let scale = targetLength / distance
        dx *= scale
        dy *= scale
    }

    let path = CGMutablePath()
    path.move(to: CGPoint(x: cx - dx, y: cy - dy))
    if electricTrail {
        let scaledLength = max(sqrt(dx * dx + dy * dy), 1.0)
        let nx = -dy / scaledLength
        let ny = dx / scaledLength
        let kink = mfxClamp(scaledLength * 0.20, min: 3.0, max: 14.0)
        path.addLine(
            to: CGPoint(
                x: cx - dx * 0.56 + nx * kink,
                y: cy - dy * 0.56 + ny * kink
            )
        )
        path.addLine(
            to: CGPoint(
                x: cx - dx * 0.28 - nx * (kink * 0.85),
                y: cy - dy * 0.28 - ny * (kink * 0.85)
            )
        )
    }
    path.addLine(to: CGPoint(x: cx, y: cy))
    return path
}

@MainActor
private func mfxCreateTrailPulseOverlayOnMainThread(
    frameX: Double,
    frameY: Double,
    frameWidth: Double,
    frameHeight: Double,
    overlayX: Int32,
    overlayY: Int32,
    normalizedType: String,
    tubesMode: Bool,
    particleMode: Bool,
    glowMode: Bool,
    deltaX: Double,
    deltaY: Double,
    intensity: Double,
    sizePx: Int32,
    lineWidthPx: Double,
    durationSec: Double,
    baseOpacity: Double,
    fillArgb: UInt32,
    strokeArgb: UInt32
) -> UnsafeMutableRawPointer? {
    if normalizedType == "none" {
        return nil
    }
    let width = max(1.0, frameWidth)
    let height = max(1.0, frameHeight)
    let windowHandle = mfx_macos_overlay_create_window_v1(frameX, frameY, width, height)
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

    let contentBounds = content.bounds
    let size = CGFloat(max(sizePx, 1))
    let intensityValue = CGFloat(max(0.0, intensity))
    let baseOpacityValue = CGFloat(baseOpacity)
    let meteorMode = (normalizedType == "meteor")

    let core = CAShapeLayer()
    core.frame = contentBounds
    if tubesMode {
        let outerInset = mfxScaleMetric(size, 9.0, 160.0, 5.0, 20.0)
        core.path = CGPath(ellipseIn: contentBounds.insetBy(dx: outerInset, dy: outerInset), transform: nil)
        core.fillColor = NSColor.clear.cgColor
        core.strokeColor = mfxColorFromArgb(strokeArgb).cgColor
        core.lineWidth = mfxScaleMetric(size, 3.2 + intensityValue * 1.4, 160.0, 1.2, 7.8)
    } else if particleMode {
        let dotInset = mfxScaleMetric(size, 16.0, 160.0, 8.0, 34.0)
        core.path = CGPath(ellipseIn: contentBounds.insetBy(dx: dotInset, dy: dotInset), transform: nil)
        core.fillColor = mfxColorFromArgb(strokeArgb).cgColor
        core.strokeColor = mfxColorFromArgb(strokeArgb).cgColor
        core.lineWidth = mfxScaleMetric(size, 1.2 + intensityValue * 0.8, 160.0, 0.8, 3.0)
    } else if meteorMode {
        let cx = contentBounds.midX
        let cy = contentBounds.midY
        let speed = CGFloat(sqrt(deltaX * deltaX + deltaY * deltaY))
        let coreRadius = mfxScaleMetric(size, 14.0 + intensityValue * 4.0, 160.0, 8.0, 28.0)
        let stretch = mfxClamp(1.0 + speed * 0.02, min: 1.0, max: 1.8)
        let semiMajor = coreRadius * stretch
        let semiMinor = coreRadius
        let angle = speed > 0.5 ? CGFloat(atan2(deltaY, deltaX)) : 0.0

        var transform = CGAffineTransform.identity
        transform = transform.translatedBy(x: cx, y: cy)
        transform = transform.rotated(by: angle)
        let path = CGMutablePath()
        path.addEllipse(
            in: CGRect(x: -semiMajor, y: -semiMinor, width: semiMajor * 2.0, height: semiMinor * 2.0),
            transform: transform
        )
        core.path = path
        core.fillColor = mfxColorFromArgb(fillArgb).cgColor
        core.strokeColor = NSColor.clear.cgColor
        core.lineWidth = 0.0
    } else {
        core.path = mfxCreateTrailLinePath(
            bounds: contentBounds,
            deltaX: CGFloat(deltaX),
            deltaY: CGFloat(deltaY),
            trailType: normalizedType
        )
        core.fillColor = NSColor.clear.cgColor
        core.strokeColor = mfxColorFromArgb(strokeArgb).cgColor
        core.lineCap = .round
        core.lineJoin = .round
        if normalizedType == "line", lineWidthPx > 0.0 {
            core.lineWidth = CGFloat(mfxClamp(lineWidthPx, min: 1.0, max: 18.0))
        } else {
            core.lineWidth = mfxScaleMetric(size, 3.0 + intensityValue * 1.6, 160.0, 1.2, 8.6)
        }
    }
    core.opacity = Float(mfxResolveOpacity(baseOpacityValue, intensityValue * 0.05, 0.0))
    contentLayer.addSublayer(core)

    if glowMode {
        let glow = CAShapeLayer()
        glow.frame = contentBounds
        let glowInset = mfxScaleMetric(
            size,
            meteorMode ? 12.0 : 18.0,
            160.0,
            meteorMode ? 5.0 : 8.0,
            meteorMode ? 28.0 : 36.0
        )
        glow.path = CGPath(ellipseIn: contentBounds.insetBy(dx: glowInset, dy: glowInset), transform: nil)
        glow.fillColor = mfxColorFromArgb(fillArgb).cgColor
        glow.strokeColor = NSColor.clear.cgColor
        glow.opacity = Float(mfxResolveOpacity(baseOpacityValue, -0.08 + intensityValue * 0.05, 0.0))
        contentLayer.addSublayer(glow)

        if meteorMode {
            let outerGlow = CAShapeLayer()
            outerGlow.frame = contentBounds
            let outerInset = mfxScaleMetric(size, 6.0, 160.0, 2.0, 16.0)
            outerGlow.path = CGPath(ellipseIn: contentBounds.insetBy(dx: outerInset, dy: outerInset), transform: nil)
            outerGlow.fillColor = mfxColorFromArgb(0x18FFDCA0).cgColor
            outerGlow.strokeColor = NSColor.clear.cgColor
            outerGlow.opacity = Float(mfxClamp(baseOpacityValue * 0.6 + intensityValue * 0.15, min: 0.1, max: 0.7))
            contentLayer.insertSublayer(outerGlow, below: glow)
        }
    }

    let group = mfxCreateScaleFadeAnimationGroup(
        fromScale: 0.65,
        toScale: 1.0,
        fromOpacity: baseOpacityValue,
        duration: durationSec
    )
    core.add(group, forKey: "mfx_trail_pulse")

    return windowHandle
}

@_cdecl("mfx_macos_trail_pulse_overlay_create_v1")
public func mfx_macos_trail_pulse_overlay_create_v1(
    _ frameX: Double,
    _ frameY: Double,
    _ frameWidth: Double,
    _ frameHeight: Double,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ normalizedTypeUtf8: UnsafePointer<CChar>?,
    _ tubesMode: Int32,
    _ particleMode: Int32,
    _ glowMode: Int32,
    _ deltaX: Double,
    _ deltaY: Double,
    _ intensity: Double,
    _ sizePx: Int32,
    _ lineWidthPx: Double,
    _ durationSec: Double,
    _ baseOpacity: Double,
    _ fillArgb: UInt32,
    _ strokeArgb: UInt32
) -> UnsafeMutableRawPointer? {
    let normalizedType = mfxNormalizeTrailType(normalizedTypeUtf8)
    let tubesEnabled = (tubesMode != 0)
    let particleEnabled = (particleMode != 0)
    let glowEnabled = (glowMode != 0)

    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTrailPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameWidth: frameWidth,
                    frameHeight: frameHeight,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    normalizedType: normalizedType,
                    tubesMode: tubesEnabled,
                    particleMode: particleEnabled,
                    glowMode: glowEnabled,
                    deltaX: deltaX,
                    deltaY: deltaY,
                    intensity: intensity,
                    sizePx: sizePx,
                    lineWidthPx: lineWidthPx,
                    durationSec: durationSec,
                    baseOpacity: baseOpacity,
                    fillArgb: fillArgb,
                    strokeArgb: strokeArgb
                )
            )
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTrailPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameWidth: frameWidth,
                    frameHeight: frameHeight,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    normalizedType: normalizedType,
                    tubesMode: tubesEnabled,
                    particleMode: particleEnabled,
                    glowMode: glowEnabled,
                    deltaX: deltaX,
                    deltaY: deltaY,
                    intensity: intensity,
                    sizePx: sizePx,
                    lineWidthPx: lineWidthPx,
                    durationSec: durationSec,
                    baseOpacity: baseOpacity,
                    fillArgb: fillArgb,
                    strokeArgb: strokeArgb
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}
