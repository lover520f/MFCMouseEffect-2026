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

private func mfxColorFromArgb(_ argb: UInt32, alphaScale: CGFloat) -> NSColor {
    let color = mfxColorFromArgb(argb)
    return color.withAlphaComponent(
        mfxClamp(color.alphaComponent * alphaScale, min: 0.0, max: 1.0)
    )
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

private func mfxCreateDonutPath(_ outer: CGRect, _ inner: CGRect) -> CGPath {
    let path = CGMutablePath()
    path.addEllipse(in: outer)
    if inner.width > 0.5, inner.height > 0.5 {
        path.addEllipse(in: inner)
    }
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

private func mfxMakeTextFloatAnimationGroup(
    fromOpacity: CGFloat,
    duration: Double,
    floatDistancePx: CGFloat,
    referenceSize: CGFloat
) -> CAAnimationGroup {
    let clampedDuration = max(0.05, duration)
    let maxDistance = max(12.0, referenceSize * 0.72)
    let clampedDistance = mfxClamp(floatDistancePx, min: 4.0, max: maxDistance)
    let horizontalDrift = CGFloat.random(in: -(referenceSize * 0.14)...(referenceSize * 0.14))

    let moveY = CAKeyframeAnimation(keyPath: "transform.translation.y")
    moveY.values = [0.0, clampedDistance * 0.58, clampedDistance]
    moveY.keyTimes = [0.0, 0.72, 1.0]
    moveY.duration = clampedDuration
    moveY.timingFunctions = [
        CAMediaTimingFunction(name: .easeOut),
        CAMediaTimingFunction(name: .easeOut),
    ]

    let moveX = CAKeyframeAnimation(keyPath: "transform.translation.x")
    moveX.values = [0.0, horizontalDrift * 0.42, horizontalDrift]
    moveX.keyTimes = [0.0, 0.72, 1.0]
    moveX.duration = clampedDuration
    moveX.timingFunctions = [
        CAMediaTimingFunction(name: .easeOut),
        CAMediaTimingFunction(name: .easeOut),
    ]

    let scale = CAKeyframeAnimation(keyPath: "transform.scale")
    scale.values = [0.86, 1.10, 1.0]
    scale.keyTimes = [0.0, 0.28, 1.0]
    scale.duration = clampedDuration
    scale.timingFunctions = [
        CAMediaTimingFunction(name: .easeOut),
        CAMediaTimingFunction(name: .easeInEaseOut),
    ]

    let fade = CAKeyframeAnimation(keyPath: "opacity")
    let clampedOpacity = mfxClamp(fromOpacity, min: 0.0, max: 1.0)
    fade.values = [clampedOpacity, clampedOpacity * 0.94, 0.0]
    fade.keyTimes = [0.0, 0.55, 1.0]
    fade.duration = clampedDuration
    fade.timingFunctions = [
        CAMediaTimingFunction(name: .easeOut),
        CAMediaTimingFunction(name: .easeIn),
    ]

    let group = CAAnimationGroup()
    group.animations = [moveY, moveX, scale, fade]
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

private func mfxResolveTextFont(_ fontFamily: String, _ size: CGFloat, _ emoji: Bool) -> NSFont {
    if emoji {
        if let emojiFont = NSFont(name: "Apple Color Emoji", size: size) {
            return emojiFont
        }
        return NSFont.systemFont(ofSize: size, weight: .regular)
    }
    if !fontFamily.isEmpty, let custom = NSFont(name: fontFamily, size: size) {
        return custom
    }
    return NSFont.boldSystemFont(ofSize: size)
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
    glowArgb: UInt32,
    baseOpacity: Double,
    animationDurationSec: Double,
    startRadiusPx: Double,
    endRadiusPx: Double,
    strokeWidthPx: Double,
    textLabel: String,
    textFontSizePx: Double,
    textFloatDistancePx: Double,
    textFontFamily: String,
    textEmoji: Bool
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
    let baseOpacityCGFloat = CGFloat(baseOpacity)
    let geometryStartRadius = CGFloat(max(0.0, startRadiusPx))
    let geometryEndRadius = CGFloat(max(1.0, endRadiusPx))
    let geometryStrokeWidth = CGFloat(max(0.1, strokeWidthPx))

    if !textMode && !starMode {
        let maxRadius = max(1.0, (sizeCGFloat - safeInset * 2.0) * 0.5)
        let endRadius = mfxClamp(geometryEndRadius, min: 1.0, max: maxRadius)
        let startRadius = mfxClamp(geometryStartRadius, min: 0.0, max: max(0.0, endRadius - 1.0))
        let ringWidth = mfxClamp(
            max(geometryStrokeWidth * 1.34, endRadius * 0.18),
            min: max(geometryStrokeWidth + 1.25, 2.6),
            max: max(3.4, endRadius * 0.42)
        )
        let innerRadius = max(0.0, endRadius - ringWidth)
        let ringFrame = CGRect(
            x: (sizeCGFloat - endRadius * 2.0) * 0.5,
            y: (sizeCGFloat - endRadius * 2.0) * 0.5,
            width: endRadius * 2.0,
            height: endRadius * 2.0
        )
        let innerFrame = CGRect(
            x: (sizeCGFloat - innerRadius * 2.0) * 0.5,
            y: (sizeCGFloat - innerRadius * 2.0) * 0.5,
            width: innerRadius * 2.0,
            height: innerRadius * 2.0
        )
        let ringPath = CGPath(ellipseIn: ringFrame, transform: nil)
        let donutPath = mfxCreateDonutPath(ringFrame, innerFrame)
        let ringStrokeWidth = mfxClamp(
            geometryStrokeWidth,
            min: 0.6,
            max: max(0.8, endRadius * 0.55)
        )
        let startScale = mfxClamp(
            startRadius / max(1.0, endRadius),
            min: 0.03,
            max: 1.0
        )

        let base = CAShapeLayer()
        base.frame = content.bounds
        base.path = donutPath
        base.fillRule = .evenOdd
        base.fillColor = mfxColorFromArgb(
            fillArgb,
            alphaScale: mfxClamp(baseOpacityCGFloat * 0.66, min: 0.0, max: 1.0)
        ).cgColor
        base.strokeColor = mfxColorFromArgb(
            strokeArgb,
            alphaScale: mfxClamp(baseOpacityCGFloat, min: 0.0, max: 1.0)
        ).cgColor
        base.lineWidth = ringStrokeWidth
        base.lineCap = .round
        base.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.0, 0.0))
        contentLayer.addSublayer(base)

        let coreGroup = mfxMakeScaleFadeAnimationGroup(
            fromScale: startScale,
            toScale: 1.0,
            fromOpacity: baseOpacityCGFloat,
            duration: animationDurationSec
        )
        base.add(coreGroup, forKey: "mfx_click_pulse")

        let secondaryRadius = max(startRadius, endRadius * 0.72)
        if secondaryRadius > innerRadius + 1.0 {
            let secondaryFrame = CGRect(
                x: (sizeCGFloat - secondaryRadius * 2.0) * 0.5,
                y: (sizeCGFloat - secondaryRadius * 2.0) * 0.5,
                width: secondaryRadius * 2.0,
                height: secondaryRadius * 2.0
            )
            let secondary = CAShapeLayer()
            secondary.frame = content.bounds
            secondary.path = CGPath(ellipseIn: secondaryFrame, transform: nil)
            secondary.fillColor = NSColor.clear.cgColor
            secondary.strokeColor = mfxColorFromArgb(
                strokeArgb,
                alphaScale: mfxClamp(baseOpacityCGFloat * 0.42, min: 0.0, max: 1.0)
            ).cgColor
            secondary.lineWidth = max(0.9, geometryStrokeWidth * 0.78)
            secondary.lineCap = .round
            secondary.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, -0.14, 0.0))
            contentLayer.addSublayer(secondary)
            let secondaryGroup = mfxMakeScaleFadeAnimationGroup(
                fromScale: startScale,
                toScale: 1.0,
                fromOpacity: mfxResolveOpacity(baseOpacityCGFloat, -0.18, 0.02),
                duration: animationDurationSec
            )
            secondary.add(secondaryGroup, forKey: "mfx_click_pulse_secondary")
        }

        for index in 0..<3 {
            let glow = CAShapeLayer()
            glow.frame = content.bounds
            glow.path = ringPath
            glow.fillColor = NSColor.clear.cgColor
            glow.strokeColor = mfxColorFromArgb(
                glowArgb,
                alphaScale: mfxClamp(
                    baseOpacityCGFloat * max(0.04, 0.15 - CGFloat(index) * 0.03),
                    min: 0.0,
                    max: 1.0
                )
            ).cgColor
            glow.lineWidth = ringWidth + mfxScaleMetric(
                sizeCGFloat,
                CGFloat(5 + index * 3),
                160.0,
                3.0,
                14.0
            )
            glow.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, -0.36, 0.02))
            glow.lineCap = .round
            contentLayer.addSublayer(glow)

            let glowGroup = mfxMakeScaleFadeAnimationGroup(
                fromScale: startScale,
                toScale: 1.0,
                fromOpacity: mfxResolveOpacity(baseOpacityCGFloat, -0.1, 0.04),
                duration: animationDurationSec
            )
            glow.add(glowGroup, forKey: "mfx_click_pulse_glow_\(index)")
        }
    }

    if starMode {
        let maxRadius = max(1.0, (sizeCGFloat - safeInset * 2.0) * 0.5)
        let endRadius = mfxClamp(geometryEndRadius, min: 1.0, max: maxRadius)
        let startRadius = mfxClamp(geometryStartRadius, min: 0.0, max: max(0.0, endRadius - 1.0))
        let star = CAShapeLayer()
        star.frame = content.bounds
        let starBounds = CGRect(
            x: (sizeCGFloat - endRadius * 2.0) * 0.5,
            y: (sizeCGFloat - endRadius * 2.0) * 0.5,
            width: endRadius * 2.0,
            height: endRadius * 2.0
        )
        star.path = mfxCreateClickStarPath(starBounds, points: 5)
        star.fillColor = mfxColorFromArgb(
            fillArgb,
            alphaScale: mfxClamp(baseOpacityCGFloat, min: 0.0, max: 1.0)
        ).cgColor
        star.strokeColor = mfxColorFromArgb(
            strokeArgb,
            alphaScale: mfxClamp(baseOpacityCGFloat, min: 0.0, max: 1.0)
        ).cgColor
        star.lineWidth = mfxClamp(geometryStrokeWidth, min: 0.6, max: max(1.0, endRadius * 0.48))
        star.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.03, 0.0))
        star.lineJoin = .round
        contentLayer.addSublayer(star)

        let starGroup = mfxMakeScaleFadeAnimationGroup(
            fromScale: mfxClamp(
                startRadius / max(1.0, endRadius),
                min: 0.03,
                max: 1.0
            ),
            toScale: 1.0,
            fromOpacity: baseOpacityCGFloat,
            duration: animationDurationSec
        )
        star.add(starGroup, forKey: "mfx_click_star")
    }

    if textMode {
        let text = CATextLayer()
        text.alignmentMode = .center
        text.isWrapped = false
        text.truncationMode = .none
        text.foregroundColor = mfxColorFromArgb(strokeArgb).cgColor
        text.contentsScale = max(1.0, contentLayer.contentsScale)
        let fontSize = mfxClamp(
            CGFloat(textFontSizePx),
            min: 8.0,
            max: max(10.0, sizeCGFloat * 0.92)
        )
        text.fontSize = fontSize
        let textFont = mfxResolveTextFont(textFontFamily, fontSize, textEmoji)
        text.font = textFont.fontName as CFTypeRef
        let attributed = NSAttributedString(
            string: textLabel,
            attributes: [.font: textFont]
        )
        var bounds = attributed.boundingRect(
            with: CGSize(width: max(8.0, sizeCGFloat * 0.94), height: max(8.0, sizeCGFloat * 0.92)),
            options: [.usesLineFragmentOrigin, .usesFontLeading],
            context: nil
        ).integral
        if bounds.width <= 0.0 {
            bounds.size.width = fontSize
        }
        if bounds.height <= 0.0 {
            bounds.size.height = fontSize * 1.2
        }
        let textWidth = mfxClamp(
            bounds.size.width + fontSize * 0.18,
            min: max(10.0, fontSize * 0.92),
            max: max(12.0, sizeCGFloat * 0.96)
        )
        let textHeight = mfxClamp(
            bounds.size.height + fontSize * 0.18,
            min: max(10.0, fontSize * 0.88),
            max: max(12.0, sizeCGFloat * 0.96)
        )
        text.frame = CGRect(
            x: (sizeCGFloat - textWidth) * 0.5,
            y: (sizeCGFloat - textHeight) * 0.5,
            width: textWidth,
            height: textHeight
        )
        text.string = textLabel
        text.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.0, 0.0))
        contentLayer.addSublayer(text)

        let textGroup = mfxMakeTextFloatAnimationGroup(
            fromOpacity: baseOpacityCGFloat,
            duration: animationDurationSec,
            floatDistancePx: CGFloat(textFloatDistancePx),
            referenceSize: sizeCGFloat
        )
        text.add(textGroup, forKey: "mfx_click_text_float")
    }

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
    _ glowArgb: UInt32,
    _ baseOpacity: Double,
    _ animationDurationSec: Double,
    _ startRadiusPx: Double,
    _ endRadiusPx: Double,
    _ strokeWidthPx: Double,
    _ textLabelUtf8: UnsafePointer<CChar>?,
    _ textFontSizePx: Double,
    _ textFloatDistancePx: Double,
    _ textFontFamilyUtf8: UnsafePointer<CChar>?,
    _ textEmoji: Int32
) -> UnsafeMutableRawPointer? {
    let normalizedType = mfxNormalizeClickType(normalizedTypeUtf8)
    let textLabel = mfxNormalizeTextLabel(textLabelUtf8)
    let textFontFamily = textFontFamilyUtf8.map(String.init(cString:)) ?? ""
    let textEmojiEnabled = (textEmoji != 0)

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
                    glowArgb: glowArgb,
                    baseOpacity: baseOpacity,
                    animationDurationSec: animationDurationSec,
                    startRadiusPx: startRadiusPx,
                    endRadiusPx: endRadiusPx,
                    strokeWidthPx: strokeWidthPx,
                    textLabel: textLabel,
                    textFontSizePx: textFontSizePx,
                    textFloatDistancePx: textFloatDistancePx,
                    textFontFamily: textFontFamily,
                    textEmoji: textEmojiEnabled
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
                    glowArgb: glowArgb,
                    baseOpacity: baseOpacity,
                    animationDurationSec: animationDurationSec,
                    startRadiusPx: startRadiusPx,
                    endRadiusPx: endRadiusPx,
                    strokeWidthPx: strokeWidthPx,
                    textLabel: textLabel,
                    textFontSizePx: textFontSizePx,
                    textFloatDistancePx: textFloatDistancePx,
                    textFontFamily: textFontFamily,
                    textEmoji: textEmojiEnabled
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}
