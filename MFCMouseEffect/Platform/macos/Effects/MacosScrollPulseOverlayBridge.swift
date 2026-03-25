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

private func mfxNormalizedDirection(dx: Double, dy: Double) -> CGVector {
    let length = sqrt(dx * dx + dy * dy)
    if length <= 0.0001 {
        return CGVector(dx: 0.0, dy: -1.0)
    }
    return CGVector(dx: dx / length, dy: dy / length)
}

private func mfxOffsetPoint(_ origin: CGPoint, direction: CGVector, distance: CGFloat) -> CGPoint {
    CGPoint(
        x: origin.x + direction.dx * distance,
        y: origin.y + direction.dy * distance
    )
}

private func mfxColorComponents(_ color: NSColor) -> (r: CGFloat, g: CGFloat, b: CGFloat, a: CGFloat) {
    let converted = color.usingColorSpace(.deviceRGB) ?? color
    return (converted.redComponent, converted.greenComponent, converted.blueComponent, converted.alphaComponent)
}

private func mfxMixColor(
    _ lhs: NSColor,
    _ rhs: NSColor,
    t: CGFloat,
    alphaScale: CGFloat
) -> CGColor {
    let safeT = mfxClamp(t, min: 0.0, max: 1.0)
    let lhsComp = mfxColorComponents(lhs)
    let rhsComp = mfxColorComponents(rhs)
    let mixed = NSColor(
        calibratedRed: lhsComp.r * (1.0 - safeT) + rhsComp.r * safeT,
        green: lhsComp.g * (1.0 - safeT) + rhsComp.g * safeT,
        blue: lhsComp.b * (1.0 - safeT) + rhsComp.b * safeT,
        alpha: mfxClamp((lhsComp.a * (1.0 - safeT) + rhsComp.a * safeT) * alphaScale, min: 0.0, max: 1.0)
    )
    return mixed.cgColor
}

private func mfxScaledColor(_ argb: UInt32, alphaScale: CGFloat) -> CGColor {
    let source = mfxColorFromArgb(argb)
    let comp = mfxColorComponents(source)
    return NSColor(
        calibratedRed: comp.r,
        green: comp.g,
        blue: comp.b,
        alpha: mfxClamp(comp.a * alphaScale, min: 0.0, max: 1.0)
    ).cgColor
}

private func mfxBuildChevronPath(
    center: CGPoint,
    direction: CGVector,
    index: Int,
    size: CGFloat,
    startRadius: CGFloat,
    endRadius: CGFloat,
    strokeWidth: CGFloat,
    intensity: CGFloat
) -> CGPath {
    let perpendicular = CGVector(dx: -direction.dy, dy: direction.dx)
    let safeStart = max(0.0, startRadius)
    let safeEnd = max(safeStart + 1.0, endRadius)
    let safeIntensity = mfxClamp(intensity, min: 0.0, max: 1.0)
    let radius = safeStart + (safeEnd - safeStart) * 0.68
    let drift = radius * 0.10 * (1.0 - safeIntensity * 0.25)
    let spacing = max(2.0, radius * 0.26)
    let length = max(size * 0.26, radius * 1.12)
    let offset = CGFloat(index) * spacing + drift
    let halfWidth = max(1.0, strokeWidth * (3.1 - CGFloat(index) * 0.65))

    let tip = mfxOffsetPoint(center, direction: direction, distance: length * 0.5 - offset)
    let tail = mfxOffsetPoint(center, direction: direction, distance: -(length * 0.5 + offset))
    let left = CGPoint(
        x: tail.x + perpendicular.dx * halfWidth,
        y: tail.y + perpendicular.dy * halfWidth
    )
    let right = CGPoint(
        x: tail.x - perpendicular.dx * halfWidth,
        y: tail.y - perpendicular.dy * halfWidth
    )

    let path = CGMutablePath()
    path.move(to: tip)
    path.addLine(to: left)
    path.move(to: tip)
    path.addLine(to: right)
    return path
}

private func mfxBuildHelixPath(
    center: CGPoint,
    direction: CGVector,
    perpendicular: CGVector,
    length: CGFloat,
    radius: CGFloat,
    turns: CGFloat,
    phase: CGFloat,
    segments: Int
) -> CGPath {
    let safeSegments = max(6, segments)
    let path = CGMutablePath()
    for idx in 0..<safeSegments {
        let u = CGFloat(idx) / CGFloat(safeSegments - 1)
        let dist = (u - 0.5) * length
        let angle = u * turns * CGFloat.pi * 2.0 + phase
        let offset = sin(angle) * radius
        let point = CGPoint(
            x: center.x + direction.dx * dist + perpendicular.dx * offset,
            y: center.y + direction.dy * dist + perpendicular.dy * offset
        )
        if idx == 0 {
            path.move(to: point)
        } else {
            path.addLine(to: point)
        }
    }
    return path
}

@MainActor
private var mfxTwinkleDotImageCache: CGImage? = nil

@MainActor
private func mfxResolveTwinkleDotImage() -> CGImage? {
    if let cached = mfxTwinkleDotImageCache {
        return cached
    }
    let image = NSImage(size: NSSize(width: 12.0, height: 12.0))
    image.lockFocus()
    NSColor.white.setFill()
    NSBezierPath(ovalIn: NSRect(x: 1.0, y: 1.0, width: 10.0, height: 10.0)).fill()
    image.unlockFocus()
    let cgImage = image.cgImage(forProposedRect: nil, context: nil, hints: nil)
    mfxTwinkleDotImageCache = cgImage
    return cgImage
}

private func mfxAddChevronLayers(
    parent: CALayer,
    center: CGPoint,
    direction: CGVector,
    size: CGFloat,
    startRadius: CGFloat,
    endRadius: CGFloat,
    strokeWidth: CGFloat,
    intensity: CGFloat,
    fillArgb: UInt32,
    strokeArgb: UInt32,
    baseOpacity: CGFloat,
    durationSec: Double
) {
    let safeStrokeWidth = max(0.8, strokeWidth)
    let glowColor = mfxColorFromArgb(fillArgb)
    let strokeColor = mfxColorFromArgb(strokeArgb)
    let safeIntensity = mfxClamp(intensity, min: 0.0, max: 1.0)
    let clampedDuration = max(0.08, durationSec)
    let safeStart = max(0.0, startRadius)
    let safeEnd = max(safeStart + 1.0, endRadius)
    // Win drift: radius * 0.10 * (1 - eased), animate from full drift to zero
    let driftDistance = (safeStart + (safeEnd - safeStart) * 0.68) * 0.10

    for index in 0..<3 {
        let fade = max(0.18, (1.0 - CGFloat(index) * 0.2) * (0.84 + safeIntensity * 0.16))
        let path = mfxBuildChevronPath(
            center: center,
            direction: direction,
            index: index,
            size: size,
            startRadius: startRadius,
            endRadius: endRadius,
            strokeWidth: safeStrokeWidth,
            intensity: safeIntensity
        )

        // Group layer per chevron for independent drift + opacity animation
        let chevronGroup = CALayer()
        chevronGroup.frame = parent.bounds

        let outerGlow = CAShapeLayer()
        outerGlow.path = path
        outerGlow.fillColor = nil
        outerGlow.lineCap = .round
        outerGlow.lineJoin = .round
        outerGlow.lineWidth = safeStrokeWidth + max(2.2, size * 0.032)
        outerGlow.strokeColor = mfxMixColor(glowColor, strokeColor, t: 0.30, alphaScale: baseOpacity * fade * 0.38)
        chevronGroup.addSublayer(outerGlow)

        let innerGlow = CAShapeLayer()
        innerGlow.path = path
        innerGlow.fillColor = nil
        innerGlow.lineCap = .round
        innerGlow.lineJoin = .round
        innerGlow.lineWidth = safeStrokeWidth + max(1.4, size * 0.016)
        innerGlow.strokeColor = mfxMixColor(glowColor, strokeColor, t: 0.45, alphaScale: baseOpacity * fade * 0.72)
        chevronGroup.addSublayer(innerGlow)

        let core = CAShapeLayer()
        core.path = path
        core.fillColor = nil
        core.lineCap = .round
        core.lineJoin = .round
        core.lineWidth = safeStrokeWidth + max(0.4, size * 0.004)
        core.strokeColor = mfxScaledColor(strokeArgb, alphaScale: baseOpacity * fade)
        chevronGroup.addSublayer(core)

        // Forward-drift animation: chevron moves along scroll direction then settles
        let driftAnim = CABasicAnimation(keyPath: "position")
        let startOffset = CGPoint(
            x: chevronGroup.position.x - direction.dx * driftDistance,
            y: chevronGroup.position.y - direction.dy * driftDistance
        )
        driftAnim.fromValue = NSValue(point: NSPoint(x: startOffset.x, y: startOffset.y))
        driftAnim.toValue = NSValue(point: NSPoint(x: chevronGroup.position.x, y: chevronGroup.position.y))
        driftAnim.duration = clampedDuration * 0.85
        driftAnim.timingFunction = CAMediaTimingFunction(controlPoints: 0.22, 1.0, 0.36, 1.0)
        driftAnim.fillMode = .forwards
        driftAnim.isRemovedOnCompletion = false

        // Staggered per-chevron opacity: back chevrons fade faster (Win: 1-i*0.20)
        let stagger = Double(index) * 0.03
        let opacityAnim = CABasicAnimation(keyPath: "opacity")
        opacityAnim.fromValue = Float(fade)
        opacityAnim.toValue = Float(0.0)
        opacityAnim.beginTime = CACurrentMediaTime() + stagger
        opacityAnim.duration = clampedDuration * (1.0 - stagger / clampedDuration)
        opacityAnim.timingFunction = CAMediaTimingFunction(name: .easeIn)
        opacityAnim.fillMode = .forwards
        opacityAnim.isRemovedOnCompletion = false

        chevronGroup.add(driftAnim, forKey: "mfx_chevron_drift_\(index)")
        chevronGroup.add(opacityAnim, forKey: "mfx_chevron_fade_\(index)")
        parent.addSublayer(chevronGroup)
    }
}

private func mfxAddHelixLayers(
    parent: CALayer,
    center: CGPoint,
    direction: CGVector,
    size: CGFloat,
    startRadius: CGFloat,
    endRadius: CGFloat,
    strokeWidth: CGFloat,
    strokeArgb: UInt32,
    baseOpacity: CGFloat,
    durationSec: Double
) {
    let backDirection = CGVector(dx: -direction.dx, dy: -direction.dy)
    let perpendicular = CGVector(dx: -backDirection.dy, dy: backDirection.dx)
    let length = max(24.0, (endRadius - startRadius) * 2.05)
    let radius = max(2.0, strokeWidth * 2.2 + 6.2)
    let turns = CGFloat(1.7)
    let segments = 34
    let white = NSColor.white
    let stroke = mfxColorFromArgb(strokeArgb)

    let strandA = mfxBuildHelixPath(
        center: center,
        direction: backDirection,
        perpendicular: perpendicular,
        length: length,
        radius: radius,
        turns: turns,
        phase: 0.0,
        segments: segments)
    let strandB = mfxBuildHelixPath(
        center: center,
        direction: backDirection,
        perpendicular: perpendicular,
        length: length,
        radius: radius,
        turns: turns,
        phase: .pi,
        segments: segments)

    let strandWidth = max(0.9, strokeWidth + 0.9)

    let auraA = CAShapeLayer()
    auraA.path = strandA
    auraA.fillColor = nil
    auraA.lineCap = .round
    auraA.lineJoin = .round
    auraA.lineWidth = strandWidth + max(1.2, size * 0.018)
    auraA.strokeColor = mfxMixColor(stroke, white, t: 0.25, alphaScale: baseOpacity * 0.35)
    parent.addSublayer(auraA)

    let coreA = CAShapeLayer()
    coreA.path = strandA
    coreA.fillColor = nil
    coreA.lineCap = .round
    coreA.lineJoin = .round
    coreA.lineWidth = strandWidth
    coreA.strokeColor = mfxMixColor(stroke, white, t: 0.40, alphaScale: baseOpacity * 0.85)
    parent.addSublayer(coreA)

    let auraB = CAShapeLayer()
    auraB.path = strandB
    auraB.fillColor = nil
    auraB.lineCap = .round
    auraB.lineJoin = .round
    auraB.lineWidth = strandWidth + max(1.0, size * 0.015)
    auraB.strokeColor = mfxMixColor(stroke, white, t: 0.16, alphaScale: baseOpacity * 0.30)
    parent.addSublayer(auraB)

    let coreB = CAShapeLayer()
    coreB.path = strandB
    coreB.fillColor = nil
    coreB.lineCap = .round
    coreB.lineJoin = .round
    coreB.lineWidth = strandWidth
    coreB.strokeColor = mfxMixColor(stroke, white, t: 0.26, alphaScale: baseOpacity * 0.70)
    parent.addSublayer(coreB)

    let rungPath = CGMutablePath()
    let rungCount = 6
    for idx in 0..<rungCount {
        let u = CGFloat(idx + 1) / CGFloat(rungCount + 1)
        let dist = (u - 0.5) * length
        let angle = u * turns * CGFloat.pi * 2.0
        let offset = sin(angle) * radius
        let opposite = sin(angle + .pi) * radius
        let p1 = CGPoint(
            x: center.x + backDirection.dx * dist + perpendicular.dx * offset,
            y: center.y + backDirection.dy * dist + perpendicular.dy * offset
        )
        let p2 = CGPoint(
            x: center.x + backDirection.dx * dist + perpendicular.dx * opposite,
            y: center.y + backDirection.dy * dist + perpendicular.dy * opposite
        )
        rungPath.move(to: p1)
        rungPath.addLine(to: p2)
    }
    let rung = CAShapeLayer()
    rung.path = rungPath
    rung.fillColor = nil
    rung.lineCap = .round
    rung.lineJoin = .round
    rung.lineWidth = max(0.7, size * 0.006)
    rung.strokeColor = mfxMixColor(stroke, white, t: 0.5, alphaScale: baseOpacity * 0.32)
    parent.addSublayer(rung)

    let spin = CABasicAnimation(keyPath: "transform.rotation.z")
    spin.fromValue = 0.0
    spin.toValue = Double.pi * 1.5
    spin.duration = max(0.2, min(0.9, durationSec * 0.55))
    spin.repeatCount = 1
    spin.timingFunction = CAMediaTimingFunction(name: .easeOut)
    parent.add(spin, forKey: "mfx_scroll_helix_spin")
}

@MainActor
private func mfxAddTwinkleEmitter(
    parent: CALayer,
    center: CGPoint,
    direction: CGVector,
    size: CGFloat,
    strokeArgb: UInt32,
    baseOpacity: CGFloat,
    intensity: CGFloat,
    strokeWidth: CGFloat,
    durationSec: Double
) {
    guard let particleImage = mfxResolveTwinkleDotImage() else {
        return
    }
    let emitter = CAEmitterLayer()
    emitter.emitterPosition = center
    emitter.emitterShape = .point
    emitter.emitterMode = .points
    emitter.renderMode = .additive
    emitter.birthRate = 1.0

    let cell = CAEmitterCell()
    cell.contents = particleImage
    cell.color = mfxScaledColor(strokeArgb, alphaScale: baseOpacity)
    cell.alphaRange = 0.25
    cell.alphaSpeed = -1.8
    cell.lifetime = Float(max(0.14, min(0.60, durationSec * 0.52)))
    cell.lifetimeRange = cell.lifetime * 0.25
    cell.birthRate = Float(20.0 + 28.0 * intensity)
    cell.velocity = max(18.0, size * 0.16)
    cell.velocityRange = cell.velocity * 0.45
    cell.emissionLongitude = atan2(direction.dy, direction.dx)
    cell.emissionRange = .pi / 3.0
    let baseScale = max(0.04, min(0.11, size / 2100.0))
    cell.scale = baseScale * max(0.7, min(1.5, strokeWidth / 2.0))
    cell.scaleRange = cell.scale * 0.8
    cell.scaleSpeed = -0.04
    cell.spin = .pi
    cell.spinRange = .pi * 0.6
    emitter.emitterCells = [cell]
    parent.addSublayer(emitter)

    let burst = CABasicAnimation(keyPath: "birthRate")
    burst.fromValue = 1.0
    burst.toValue = 0.0
    burst.duration = max(0.08, min(0.35, durationSec * 0.45))
    burst.fillMode = .forwards
    burst.isRemovedOnCompletion = false
    emitter.add(burst, forKey: "mfx_scroll_twinkle_burst")
}

@MainActor
private func mfxCreateScrollPulseOverlayOnMainThread(
    frameX: Double,
    frameY: Double,
    frameSize: Double,
    bodyX: Double,
    bodyY: Double,
    bodyWidth: Double,
    bodyHeight: Double,
    overlayX: Int32,
    overlayY: Int32,
    directionDx: Double,
    directionDy: Double,
    strengthLevel: Int32,
    intensity: Double,
    startRadiusPx: Double,
    endRadiusPx: Double,
    strokeWidthPx: Double,
    helixMode: Bool,
    twinkleMode: Bool,
    fillArgb: UInt32,
    strokeArgb: UInt32,
    baseOpacity: Double,
    durationSec: Double
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

    let contentBounds = content.bounds
    let bodyRect = CGRect(x: bodyX, y: bodyY, width: bodyWidth, height: bodyHeight)
    let sizeCGFloat = max(1.0, CGFloat(size))
    let baseOpacityCGFloat = CGFloat(baseOpacity)
    let commandIntensity = mfxClamp(CGFloat(intensity), min: 0.0, max: 1.0)
    let intensityFloor = mfxClamp(CGFloat(abs(strengthLevel)) / 6.0, min: 0.16, max: 1.0)
    let renderIntensity = max(commandIntensity, intensityFloor)
    let startRadius = CGFloat(max(0.0, startRadiusPx))
    let endRadius = CGFloat(max(startRadius + 1.0, endRadiusPx))
    let strokeWidth = CGFloat(max(0.6, strokeWidthPx))
    let center: CGPoint
    if bodyRect.isNull || bodyRect.isEmpty {
        center = CGPoint(x: contentBounds.midX, y: contentBounds.midY)
    } else {
        center = CGPoint(x: bodyRect.midX, y: bodyRect.midY)
    }
    let direction = mfxNormalizedDirection(dx: directionDx, dy: directionDy)

    let renderRoot = CALayer()
    renderRoot.frame = contentBounds
    renderRoot.opacity = Float(mfxResolveOpacity(baseOpacityCGFloat, 0.0, 0.0))
    contentLayer.addSublayer(renderRoot)

    // Chevron only in Arrow mode; Helix and Twinkle have their own visuals
    if !helixMode && !twinkleMode {
        mfxAddChevronLayers(
            parent: renderRoot,
            center: center,
            direction: direction,
            size: sizeCGFloat,
            startRadius: startRadius,
            endRadius: endRadius,
            strokeWidth: strokeWidth,
            intensity: renderIntensity,
            fillArgb: fillArgb,
            strokeArgb: strokeArgb,
            baseOpacity: baseOpacityCGFloat,
            durationSec: durationSec)
    }

    if helixMode {
        let parentPtr = Unmanaged.passUnretained(renderRoot).toOpaque()
        let animatorHandle = mfx_scroll_helix_create(
            parentPtr,
            Double(center.x), Double(center.y),
            Double(direction.dx), Double(direction.dy),
            overlayX, overlayY,
            Double(sizeCGFloat),
            Double(startRadius), Double(endRadius), Double(strokeWidth),
            strokeArgb, fillArgb,
            Double(baseOpacityCGFloat),
            Double(renderIntensity),
            durationSec)
        if let animatorHandle {
            mfxScheduleAnimatorRelease(animatorHandle, afterMs: Int(durationSec * 1000.0) + 100)
        }
    }

    if twinkleMode {
        let parentPtr = Unmanaged.passUnretained(renderRoot).toOpaque()
        let animatorHandle = mfx_scroll_twinkle_create(
            parentPtr,
            Double(center.x), Double(center.y),
            Double(direction.dx), Double(direction.dy),
            overlayX, overlayY,
            Double(sizeCGFloat),
            strokeArgb, fillArgb,
            Double(baseOpacityCGFloat),
            Double(renderIntensity),
            Double(strokeWidth),
            durationSec)
        if let animatorHandle {
            mfxScheduleAnimatorRelease(animatorHandle, afterMs: Int(durationSec * 1000.0) + 100)
        }
    }

    let group = mfxCreateScaleFadeAnimationGroup(
        fromScale: 0.84,
        toScale: 1.02,
        fromOpacity: CGFloat(baseOpacity + 0.04),
        duration: durationSec)
    renderRoot.add(group, forKey: "mfx_scroll_group")

    return windowHandle
}

@_cdecl("mfx_macos_scroll_pulse_overlay_create_v1")
public func mfx_macos_scroll_pulse_overlay_create_v1(
    _ frameX: Double,
    _ frameY: Double,
    _ frameSize: Double,
    _ bodyX: Double,
    _ bodyY: Double,
    _ bodyWidth: Double,
    _ bodyHeight: Double,
    _ overlayX: Int32,
    _ overlayY: Int32,
    _ directionDx: Double,
    _ directionDy: Double,
    _ strengthLevel: Int32,
    _ intensity: Double,
    _ startRadiusPx: Double,
    _ endRadiusPx: Double,
    _ strokeWidthPx: Double,
    _ helixMode: Int32,
    _ twinkleMode: Int32,
    _ fillArgb: UInt32,
    _ strokeArgb: UInt32,
    _ baseOpacity: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer? {
    let helixEnabled = (helixMode != 0)
    let twinkleEnabled = (twinkleMode != 0)

    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateScrollPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    bodyX: bodyX,
                    bodyY: bodyY,
                    bodyWidth: bodyWidth,
                    bodyHeight: bodyHeight,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    directionDx: directionDx,
                    directionDy: directionDy,
                    strengthLevel: strengthLevel,
                    intensity: intensity,
                    startRadiusPx: startRadiusPx,
                    endRadiusPx: endRadiusPx,
                    strokeWidthPx: strokeWidthPx,
                    helixMode: helixEnabled,
                    twinkleMode: twinkleEnabled,
                    fillArgb: fillArgb,
                    strokeArgb: strokeArgb,
                    baseOpacity: baseOpacity,
                    durationSec: durationSec
                )
            )
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateScrollPulseOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    bodyX: bodyX,
                    bodyY: bodyY,
                    bodyWidth: bodyWidth,
                    bodyHeight: bodyHeight,
                    overlayX: overlayX,
                    overlayY: overlayY,
                    directionDx: directionDx,
                    directionDy: directionDy,
                    strengthLevel: strengthLevel,
                    intensity: intensity,
                    startRadiusPx: startRadiusPx,
                    endRadiusPx: endRadiusPx,
                    strokeWidthPx: strokeWidthPx,
                    helixMode: helixEnabled,
                    twinkleMode: twinkleEnabled,
                    fillArgb: fillArgb,
                    strokeArgb: strokeArgb,
                    baseOpacity: baseOpacity,
                    durationSec: durationSec
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

// MARK: - Cross-file animator factory declarations

@_silgen_name("mfx_macos_scroll_helix_animator_create_v1")
private func mfx_scroll_helix_create(
    _ parentHandle: UnsafeMutableRawPointer,
    _ centerX: Double, _ centerY: Double,
    _ dirDx: Double, _ dirDy: Double,
    _ overlayX: Int32, _ overlayY: Int32,
    _ size: Double,
    _ startRadius: Double, _ endRadius: Double, _ strokeWidth: Double,
    _ strokeArgb: UInt32, _ fillArgb: UInt32,
    _ baseOpacity: Double, _ intensity: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer?

@_silgen_name("mfx_macos_scroll_twinkle_animator_create_v1")
private func mfx_scroll_twinkle_create(
    _ parentHandle: UnsafeMutableRawPointer,
    _ centerX: Double, _ centerY: Double,
    _ dirDx: Double, _ dirDy: Double,
    _ overlayX: Int32, _ overlayY: Int32,
    _ size: Double,
    _ strokeArgb: UInt32, _ fillArgb: UInt32,
    _ baseOpacity: Double, _ intensity: Double,
    _ strokeWidth: Double,
    _ durationSec: Double
) -> UnsafeMutableRawPointer?

@_silgen_name("mfx_macos_scroll_animator_release_v1")
private func mfx_scroll_animator_release(_ handle: UnsafeMutableRawPointer?)

// MARK: - Animator lifecycle management

/// Schedule retained animator release after duration + padding.
private func mfxScheduleAnimatorRelease(_ handle: UnsafeMutableRawPointer, afterMs: Int) {
    let safeMs = max(50, afterMs)
    let bits = Int(bitPattern: handle)
    DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(safeMs)) {
        guard let ptr = UnsafeMutableRawPointer(bitPattern: bits) else { return }
        mfx_scroll_animator_release(ptr)
    }
}
