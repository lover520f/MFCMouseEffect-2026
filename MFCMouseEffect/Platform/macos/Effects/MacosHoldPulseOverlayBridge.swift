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

private func mfxCreateLightningPath(_ bounds: CGRect, phase: CGFloat = 0.0) -> CGPath {
    let cx = bounds.midX
    let cy = bounds.midY
    let h = bounds.height * 0.40
    let jitter = sin(phase) * 3.6
    let path = CGMutablePath()
    path.move(to: CGPoint(x: cx - 6.0 + jitter, y: cy + h * 0.45))
    path.addLine(to: CGPoint(x: cx + 2.0 - jitter * 0.35, y: cy + h * 0.10))
    path.addLine(to: CGPoint(x: cx - 1.5 + jitter * 0.20, y: cy + h * 0.10))
    path.addLine(to: CGPoint(x: cx + 6.0 - jitter, y: cy - h * 0.45))
    path.addLine(to: CGPoint(x: cx - 2.0 + jitter * 0.30, y: cy - h * 0.05))
    path.addLine(to: CGPoint(x: cx + 1.5 - jitter * 0.20, y: cy - h * 0.05))
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
    accent.fillColor = NSColor.clear.cgColor
    accent.strokeColor = baseColor.cgColor
    accent.lineJoin = .round
    accent.lineCap = .round
    accent.lineDashPattern = nil

    switch holdStyle {
    case .hex:
        accent.path = mfxCreateHexPath(bounds.insetBy(dx: 38.0, dy: 38.0))
        accent.lineWidth = 1.8
        accent.lineDashPattern = [3, 5]
    case .lightning:
        accent.path = mfxCreateLightningPath(bounds.insetBy(dx: 36.0, dy: 36.0))
        accent.fillColor = baseColor.cgColor
        accent.lineWidth = 1.0
        accent.shadowColor = baseColor.cgColor
        accent.shadowRadius = 8.0
        accent.shadowOpacity = 0.8
    case .fluxField:
        accent.path = mfxCreateFluxFieldPath(bounds.insetBy(dx: 36.0, dy: 36.0))
        accent.lineWidth = 2.0
    case .quantumHalo:
        accent.path = CGPath(ellipseIn: bounds.insetBy(dx: 36.0, dy: 36.0), transform: nil)
        accent.lineWidth = 2.2
        accent.lineDashPattern = [2, 5]
    case .techRing:
        accent.path = CGPath(ellipseIn: bounds.insetBy(dx: 32.0, dy: 32.0), transform: nil)
        accent.lineWidth = 2.0
        accent.lineDashPattern = [10, 6]
    case .hologram:
        let path = CGMutablePath()
        path.addEllipse(in: bounds.insetBy(dx: 30.0, dy: 30.0))
        path.addEllipse(in: bounds.insetBy(dx: 42.0, dy: 42.0))
        accent.path = path
        accent.lineWidth = 1.6
        accent.lineDashPattern = [14, 9]
    case .neon:
        let path = CGMutablePath()
        path.addEllipse(in: bounds.insetBy(dx: 28.0, dy: 28.0))
        path.addEllipse(in: bounds.insetBy(dx: 40.0, dy: 40.0))
        accent.path = path
        accent.lineWidth = 2.0
        accent.lineDashPattern = [4, 7]
    case .charge:
        accent.path = CGPath(ellipseIn: bounds.insetBy(dx: 40.0, dy: 40.0), transform: nil)
        accent.lineWidth = 1.6
    }
}

private func mfxFindShapeLayer(_ root: CALayer, named name: String) -> CAShapeLayer? {
    return root.sublayers?.first(where: { $0.name == name }) as? CAShapeLayer
}

private func mfxFindLayer(_ root: CALayer, named name: String) -> CALayer? {
    return root.sublayers?.first(where: { $0.name == name })
}

private func mfxResolveSpinDuration(
    style: HoldStyleCode,
    rotateDurationSec: Double,
    rotateDurationFastSec: Double
) -> Double {
    switch style {
    case .lightning:
        return max(0.05, rotateDurationFastSec * 0.65)
    case .hex:
        return max(0.05, rotateDurationSec * 0.8)
    case .techRing, .hologram, .neon:
        return max(0.05, rotateDurationFastSec)
    case .quantumHalo, .fluxField:
        return max(0.05, rotateDurationFastSec)
    case .charge:
        return max(0.05, rotateDurationSec * 1.1)
    }
}

// MARK: - Per-type layer builders

/// Charge: background ring (stroke-only) + glow arc + progress arc + orbiting dot head.
/// Matches Windows ChargeRenderer visual contract.
@MainActor
private func mfxBuildChargeLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double
) {
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let ringLineWidth = mfxScaleMetric(size, 2.4, 160.0, 1.2, 4.8)
    let arcRect = bounds.insetBy(dx: ringInset, dy: ringInset)

    // 1) Background ring – stroke only, low alpha (matches Windows bgPen 25% alpha)
    let ring = CAShapeLayer()
    ring.name = "mfx_hold_ring"
    ring.frame = bounds
    ring.path = CGPath(ellipseIn: arcRect, transform: nil)
    ring.fillColor = NSColor.clear.cgColor
    ring.strokeColor = baseColor.withAlphaComponent(0.25).cgColor
    ring.lineWidth = ringLineWidth
    ring.opacity = Float(mfxResolveOpacity(CGFloat(baseOpacity), 0.0, 0.0))
    contentLayer.addSublayer(ring)

    // 2) Glow arc – wider stroke, lower alpha, sweeps with progress
    let glowArc = CAShapeLayer()
    glowArc.name = "mfx_hold_accent"
    glowArc.frame = bounds
    glowArc.path = CGPath(ellipseIn: arcRect, transform: nil)
    glowArc.fillColor = NSColor.clear.cgColor
    glowArc.strokeColor = baseColor.withAlphaComponent(0.36).cgColor
    glowArc.lineWidth = ringLineWidth + 8.0
    glowArc.lineCap = .round
    glowArc.strokeStart = 0.0
    glowArc.strokeEnd = 0.0
    glowArc.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(glowArc)

    // 3) Progress arc – main stroke
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: arcRect, transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.92).cgColor
    progress.lineWidth = ringLineWidth + 1.2
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)

    // 4) Orbiting dot head
    let head = CALayer()
    head.name = "mfx_hold_head"
    let headSize = max(4.0, ringLineWidth * 2.2)
    head.bounds = CGRect(x: 0.0, y: 0.0, width: headSize, height: headSize)
    head.cornerRadius = headSize * 0.5
    head.backgroundColor = baseColor.withAlphaComponent(0.96).cgColor
    head.shadowColor = baseColor.cgColor
    head.shadowOpacity = 0.9
    head.shadowRadius = max(5.0, ringLineWidth * 2.2)
    head.shadowOffset = .zero
    // Start at 12 o'clock
    let radius = max(1.0, size * 0.5 - ringInset)
    head.position = CGPoint(x: bounds.midX, y: bounds.midY - radius)
    contentLayer.addSublayer(head)

    // 5) Breathing animation on background ring
    let breathe = CABasicAnimation(keyPath: "opacity")
    breathe.fromValue = 0.15
    breathe.toValue = 0.35
    breathe.duration = max(0.05, breatheDurationSec)
    breathe.autoreverses = true
    breathe.repeatCount = .greatestFiniteMagnitude
    ring.add(breathe, forKey: "mfx_hold_breathe")
}

/// Lightning: central energy orb + 24 bolt particle lines radiating outward.
/// Matches Windows LightningRenderer visual contract.
@MainActor
private func mfxBuildLightningLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationFastSec: Double
) {
    let cx = bounds.midX
    let cy = bounds.midY
    let radius = size * 0.38

    // 1) Core glow – radial gradient layer
    let coreGlowSize = radius * 1.2
    let coreGlow = CAGradientLayer()
    coreGlow.name = "mfx_hold_ring"
    coreGlow.type = .radial
    coreGlow.startPoint = CGPoint(x: 0.5, y: 0.5)
    coreGlow.endPoint = CGPoint(x: 1.0, y: 1.0)
    coreGlow.colors = [
        baseColor.withAlphaComponent(0.45).cgColor,
        baseColor.withAlphaComponent(0.0).cgColor
    ]
    coreGlow.frame = CGRect(
        x: cx - coreGlowSize,
        y: cy - coreGlowSize,
        width: coreGlowSize * 2.0,
        height: coreGlowSize * 2.0)
    coreGlow.cornerRadius = coreGlowSize
    contentLayer.addSublayer(coreGlow)

    // 2) Energy orb core – smaller, brighter
    let coreSize = size * 0.06
    let core = CALayer()
    core.name = "mfx_hold_core"
    core.frame = CGRect(x: cx - coreSize, y: cy - coreSize, width: coreSize * 2.0, height: coreSize * 2.0)
    core.cornerRadius = coreSize
    core.backgroundColor = baseColor.withAlphaComponent(0.85).cgColor
    core.shadowColor = baseColor.cgColor
    core.shadowOpacity = 0.9
    core.shadowRadius = coreSize * 2.0
    core.shadowOffset = .zero
    contentLayer.addSublayer(core)

    // 3) 24 bolt lines radiating from center
    let boltContainer = CALayer()
    boltContainer.name = "mfx_hold_accent"
    boltContainer.frame = bounds
    contentLayer.addSublayer(boltContainer)

    let boltCount = 24
    for i in 0..<boltCount {
        let angle = CGFloat(i) * (CGFloat.pi * 2.0 / CGFloat(boltCount))
        let boltLength = radius * CGFloat(0.3 + 0.5 * Double((i * 37 + 13) % 100) / 100.0)
        let startDist = radius * 0.15

        let x1 = cx + cos(angle) * startDist
        let y1 = cy + sin(angle) * startDist
        let x2 = cx + cos(angle) * (startDist + boltLength)
        let y2 = cy + sin(angle) * (startDist + boltLength)

        let bolt = CAShapeLayer()
        bolt.name = "mfx_bolt_\(i)"
        let path = CGMutablePath()
        path.move(to: CGPoint(x: x1, y: y1))
        path.addLine(to: CGPoint(x: x2, y: y2))
        bolt.path = path
        bolt.strokeColor = baseColor.withAlphaComponent(0.55).cgColor
        bolt.lineWidth = 1.5
        bolt.lineCap = .round
        bolt.fillColor = NSColor.clear.cgColor

        // Staggered opacity animation per bolt
        let flash = CABasicAnimation(keyPath: "opacity")
        flash.fromValue = 0.15
        flash.toValue = 0.75
        let phase = Double(i) * 0.12
        flash.duration = max(0.08, rotateDurationFastSec * 0.28)
        flash.beginTime = CACurrentMediaTime() + phase
        flash.autoreverses = true
        flash.repeatCount = .greatestFiniteMagnitude
        bolt.add(flash, forKey: "mfx_bolt_flash")

        boltContainer.addSublayer(bolt)
    }

    // 4) Progress layer (shared name for update path)
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.6).cgColor
    progress.lineWidth = 2.0
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)

    // 5) Core pulse animation
    let pulse = CABasicAnimation(keyPath: "opacity")
    pulse.fromValue = 0.5
    pulse.toValue = 1.0
    pulse.duration = max(0.05, breatheDurationSec * 0.5)
    pulse.autoreverses = true
    pulse.repeatCount = .greatestFiniteMagnitude
    core.add(pulse, forKey: "mfx_core_pulse")
}

/// Hex: 3 concentric rotating hexagons with vertex glow dots.
/// Matches Windows HexRenderer visual contract.
@MainActor
private func mfxBuildHexLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationSec: Double
) {
    let cx = bounds.midX
    let cy = bounds.midY
    let baseRadius = size * 0.36

    // Container for ring handle
    let container = CALayer()
    container.name = "mfx_hold_ring"
    container.frame = bounds
    contentLayer.addSublayer(container)

    // Config: (radius fraction, rotation speed multiplier, line width, alpha, hasFill)
    let hexConfigs: [(CGFloat, Double, CGFloat, CGFloat, Bool)] = [
        (0.4,  2.0, 2.4, 1.0, true),   // inner – filled
        (0.7, -0.5, 1.8, 0.7, false),  // middle
        (1.0,  1.0, 1.2, 0.4, false),  // outer
    ]

    for (i, config) in hexConfigs.enumerated() {
        let (rFrac, speedMul, lineWidth, alpha, hasFill) = config
        let r = baseRadius * rFrac

        // Hex path
        let hexPath = CGMutablePath()
        for v in 0..<6 {
            let angle = CGFloat(v) * (CGFloat.pi / 3.0)
            let x = cx + r * cos(angle)
            let y = cy + r * sin(angle)
            if v == 0 { hexPath.move(to: CGPoint(x: x, y: y)) }
            else { hexPath.addLine(to: CGPoint(x: x, y: y)) }
        }
        hexPath.closeSubpath()

        let hex = CAShapeLayer()
        hex.name = "mfx_hex_\(i)"
        hex.frame = bounds
        hex.path = hexPath
        hex.strokeColor = baseColor.withAlphaComponent(alpha).cgColor
        hex.lineWidth = lineWidth
        hex.lineJoin = .miter
        hex.fillColor = hasFill ? baseColor.withAlphaComponent(0.12).cgColor : NSColor.clear.cgColor

        // Vertex glow dots
        let dotRadius = lineWidth * 1.5
        let dotLayer = CALayer()
        dotLayer.name = "mfx_hex_dots_\(i)"
        dotLayer.frame = bounds
        for v in 0..<6 {
            let angle = CGFloat(v) * (CGFloat.pi / 3.0)
            let x = cx + r * cos(angle)
            let y = cy + r * sin(angle)
            let dot = CALayer()
            dot.frame = CGRect(x: x - dotRadius, y: y - dotRadius, width: dotRadius * 2, height: dotRadius * 2)
            dot.cornerRadius = dotRadius
            dot.backgroundColor = baseColor.withAlphaComponent(alpha * 0.6).cgColor
            dot.shadowColor = baseColor.cgColor
            dot.shadowOpacity = 0.5
            dot.shadowRadius = dotRadius
            dot.shadowOffset = .zero
            dotLayer.addSublayer(dot)
        }

        // Container for hex + dots so rotation applies to both
        let group = CALayer()
        group.name = "mfx_hex_group_\(i)"
        group.frame = bounds
        group.addSublayer(hex)
        group.addSublayer(dotLayer)

        // Rotation animation
        let spin = CABasicAnimation(keyPath: "transform.rotation")
        spin.fromValue = 0.0
        spin.toValue = Double.pi * 2.0
        spin.duration = max(0.1, rotateDurationSec * abs(1.0 / speedMul))
        spin.repeatCount = .greatestFiniteMagnitude
        if speedMul < 0 {
            spin.toValue = -Double.pi * 2.0
        }
        group.add(spin, forKey: "mfx_hex_spin")

        container.addSublayer(group)
    }

    // Accent layer (for handle compatibility)
    let accent = CALayer()
    accent.name = "mfx_hold_accent"
    accent.frame = bounds
    contentLayer.addSublayer(accent)

    // Progress arc
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.7).cgColor
    progress.lineWidth = 2.0
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)

    // Breathe animation on container
    let breathe = CABasicAnimation(keyPath: "opacity")
    breathe.fromValue = 0.5
    breathe.toValue = Float(mfxResolveOpacity(CGFloat(baseOpacity), 0.0, 0.0))
    breathe.duration = max(0.05, breatheDurationSec)
    breathe.autoreverses = true
    breathe.repeatCount = .greatestFiniteMagnitude
    container.add(breathe, forKey: "mfx_hold_breathe")
}

/// TechRing: 3 tilted gyro ellipse rings with 3D perspective + orbit particles.
/// Matches Windows TechRingRenderer visual contract.
@MainActor
private func mfxBuildTechRingLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationSec: Double,
    rotateDurationFastSec: Double
) {
    let cx = bounds.midX
    let cy = bounds.midY
    let baseRadius = size * 0.38

    // Container with 3D perspective
    let container = CALayer()
    container.name = "mfx_hold_ring"
    container.frame = bounds
    var perspective = CATransform3DIdentity
    perspective.m34 = -1.0 / 500.0
    container.sublayerTransform = perspective
    contentLayer.addSublayer(container)

    // Ring configs: (radius fraction, tiltX°, tiltY°, spinDuration, alpha)
    let ringConfigs: [(CGFloat, CGFloat, CGFloat, Double, CGFloat)] = [
        (0.6, 60.0, 0.0, rotateDurationSec * 0.67, 0.5),   // inner gyro
        (0.8, 0.0, 45.0, rotateDurationSec * 0.83, 0.4),    // middle gyro
        (1.1, 0.0, 0.0, rotateDurationFastSec * 1.25, 0.7), // outer scanner
    ]

    for (i, config) in ringConfigs.enumerated() {
        let (rFrac, tiltX, tiltY, spinDur, alpha) = config
        let r = baseRadius * rFrac
        let ringRect = CGRect(x: cx - r, y: cy - r, width: r * 2, height: r * 2)

        let ring = CAShapeLayer()
        ring.name = "mfx_tech_ring_\(i)"
        ring.frame = bounds
        ring.path = CGPath(ellipseIn: ringRect, transform: nil)
        ring.fillColor = NSColor.clear.cgColor
        ring.strokeColor = baseColor.withAlphaComponent(alpha).cgColor
        ring.lineWidth = i == 2 ? 3.0 : 2.0

        // Apply 3D tilt
        let tiltXRad = tiltX * CGFloat.pi / 180.0
        let tiltYRad = tiltY * CGFloat.pi / 180.0
        var tilt = CATransform3DIdentity
        tilt = CATransform3DRotate(tilt, tiltXRad, 1, 0, 0)
        tilt = CATransform3DRotate(tilt, tiltYRad, 0, 1, 0)
        ring.transform = tilt

        // Spin animation
        let spin = CABasicAnimation(keyPath: "transform.rotation.z")
        spin.fromValue = 0.0
        spin.toValue = i == 1 ? -Double.pi * 2.0 : Double.pi * 2.0
        spin.duration = max(0.1, spinDur)
        spin.repeatCount = .greatestFiniteMagnitude
        ring.add(spin, forKey: "mfx_tech_spin")

        container.addSublayer(ring)
    }

    // Orbit particles (20 dots scattered in 3D-like positions)
    for i in 0..<20 {
        let angle = CGFloat(i) * (CGFloat.pi * 2.0 / 20.0)
        let r = baseRadius * CGFloat(0.3 + 0.5 * Double((i * 41 + 7) % 100) / 100.0)
        let x = cx + cos(angle) * r
        let y = cy + sin(angle) * r * 0.7 // flattened by perspective
        let dotSize: CGFloat = 3.0

        let dot = CALayer()
        dot.name = "mfx_tech_dot_\(i)"
        dot.frame = CGRect(x: x - dotSize * 0.5, y: y - dotSize * 0.5, width: dotSize, height: dotSize)
        dot.cornerRadius = dotSize * 0.5
        dot.backgroundColor = baseColor.withAlphaComponent(0.35).cgColor
        container.addSublayer(dot)
    }

    // Accent (handle compat)
    let accent = CALayer()
    accent.name = "mfx_hold_accent"
    accent.frame = bounds
    contentLayer.addSublayer(accent)

    // Progress arc
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.6).cgColor
    progress.lineWidth = 2.0
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)

    // Breathe
    let breathe = CABasicAnimation(keyPath: "opacity")
    breathe.fromValue = 0.5
    breathe.toValue = Float(mfxResolveOpacity(CGFloat(baseOpacity), 0.0, 0.0))
    breathe.duration = max(0.05, breatheDurationSec)
    breathe.autoreverses = true
    breathe.repeatCount = .greatestFiniteMagnitude
    container.add(breathe, forKey: "mfx_hold_breathe")
}

/// Hologram: tilted segmented rings + rising particle dots + central core.
/// Matches Windows HologramHudRenderer visual contract.
@MainActor
private func mfxBuildHologramLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationSec: Double,
    rotateDurationFastSec: Double
) {
    let cx = bounds.midX
    let cy = bounds.midY
    let baseRadius = size * 0.36

    let container = CALayer()
    container.name = "mfx_hold_ring"
    container.frame = bounds
    contentLayer.addSublayer(container)

    // Segmented ring 1 – 3 arc segments, rotating CW
    let arcCount1 = 3
    let sweep1 = (CGFloat.pi * 2.0 / CGFloat(arcCount1)) * 0.8
    for i in 0..<arcCount1 {
        let startAngle = CGFloat(i) * (CGFloat.pi * 2.0 / CGFloat(arcCount1))
        let arc = CAShapeLayer()
        arc.name = "mfx_holo_arc1_\(i)"
        arc.frame = bounds
        let path = CGMutablePath()
        path.addArc(center: CGPoint(x: cx, y: cy), radius: baseRadius,
                    startAngle: startAngle, endAngle: startAngle + sweep1, clockwise: false)
        arc.path = path
        arc.fillColor = NSColor.clear.cgColor
        arc.strokeColor = baseColor.withAlphaComponent(0.55).cgColor
        arc.lineWidth = 3.5
        arc.lineCap = .round
        container.addSublayer(arc)
    }

    // Segmented ring 2 – 4 arc segments, smaller, rotating CCW
    let arcCount2 = 4
    let sweep2 = (CGFloat.pi * 2.0 / CGFloat(arcCount2)) * 0.6
    let innerR = baseRadius * 0.65
    for i in 0..<arcCount2 {
        let startAngle = CGFloat(i) * (CGFloat.pi * 2.0 / CGFloat(arcCount2))
        let arc = CAShapeLayer()
        arc.name = "mfx_holo_arc2_\(i)"
        arc.frame = bounds
        let path = CGMutablePath()
        path.addArc(center: CGPoint(x: cx, y: cy), radius: innerR,
                    startAngle: startAngle, endAngle: startAngle + sweep2, clockwise: false)
        arc.path = path
        arc.fillColor = NSColor.clear.cgColor
        arc.strokeColor = baseColor.withAlphaComponent(0.4).cgColor
        arc.lineWidth = 2.0
        arc.lineCap = .round
        container.addSublayer(arc)
    }

    // Spin animations on container
    let spin = CABasicAnimation(keyPath: "transform.rotation")
    spin.fromValue = 0.0
    spin.toValue = Double.pi * 2.0
    spin.duration = max(0.1, rotateDurationSec * 2.0)
    spin.repeatCount = .greatestFiniteMagnitude
    container.add(spin, forKey: "mfx_holo_spin")

    // Rising particles (15 dots with vertical drift animation)
    let particleContainer = CALayer()
    particleContainer.name = "mfx_hold_accent"
    particleContainer.frame = bounds
    contentLayer.addSublayer(particleContainer)

    for i in 0..<15 {
        let angle = CGFloat(i) * (CGFloat.pi * 2.0 / 15.0)
        let r = baseRadius * CGFloat(0.2 + 0.6 * Double((i * 31 + 17) % 100) / 100.0)
        let x = cx + cos(angle) * r
        let y = cy

        let dot = CALayer()
        dot.frame = CGRect(x: x - 1.5, y: y - 1.5, width: 3.0, height: 3.0)
        dot.cornerRadius = 1.5
        dot.backgroundColor = baseColor.withAlphaComponent(0.5).cgColor

        // Rise animation
        let rise = CABasicAnimation(keyPath: "position.y")
        rise.fromValue = y
        rise.toValue = y - baseRadius * 1.2
        rise.duration = 1.5 + Double(i % 5) * 0.3
        rise.repeatCount = .greatestFiniteMagnitude
        dot.add(rise, forKey: "mfx_holo_rise")

        let fade = CABasicAnimation(keyPath: "opacity")
        fade.fromValue = 0.6
        fade.toValue = 0.0
        fade.duration = rise.duration
        fade.repeatCount = .greatestFiniteMagnitude
        dot.add(fade, forKey: "mfx_holo_fade")

        particleContainer.addSublayer(dot)
    }

    // Core glow
    let coreSize = size * 0.04
    let core = CALayer()
    core.frame = CGRect(x: cx - coreSize, y: cy - coreSize, width: coreSize * 2, height: coreSize * 2)
    core.cornerRadius = coreSize
    core.backgroundColor = baseColor.withAlphaComponent(0.0).cgColor
    contentLayer.addSublayer(core)

    // Progress arc
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.6).cgColor
    progress.lineWidth = 2.0
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)
}

/// FluxField: 6 concentric pulsing rings + 4 rotating arc bands + orbit particles + center glow.
/// Matches Windows FluxFieldHudCpuRenderer visual contract.
@MainActor
private func mfxBuildFluxFieldLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationSec: Double,
    rotateDurationFastSec: Double
) {
    let cx = bounds.midX
    let cy = bounds.midY
    let baseRadius = size * 0.38

    let container = CALayer()
    container.name = "mfx_hold_ring"
    container.frame = bounds
    contentLayer.addSublayer(container)

    // 6 concentric rings with staggered pulse
    for i in 0..<6 {
        let frac = CGFloat(i + 1) / 6.0
        let r = baseRadius * (0.28 + frac * 0.78)
        let ringRect = CGRect(x: cx - r, y: cy - r, width: r * 2, height: r * 2)
        let ring = CAShapeLayer()
        ring.name = "mfx_flux_ring_\(i)"
        ring.frame = bounds
        ring.path = CGPath(ellipseIn: ringRect, transform: nil)
        ring.fillColor = NSColor.clear.cgColor
        ring.strokeColor = baseColor.withAlphaComponent(0.08 + 0.11 * frac).cgColor
        ring.lineWidth = 1.3 + frac * 2.4

        let pulse = CABasicAnimation(keyPath: "opacity")
        pulse.fromValue = 0.3
        pulse.toValue = 0.8
        pulse.duration = max(0.1, 0.8 + Double(frac) * 0.5)
        pulse.beginTime = CACurrentMediaTime() + Double(frac) * 0.3
        pulse.autoreverses = true
        pulse.repeatCount = .greatestFiniteMagnitude
        ring.add(pulse, forKey: "mfx_flux_pulse")

        container.addSublayer(ring)
    }

    // 4 rotating segmented arc bands
    for band in 0..<4 {
        let frac = CGFloat(band + 1) / 4.0
        let arcR = baseRadius * (0.34 + frac * 0.62)
        let segCount = 8
        let sweep = CGFloat.pi * 2.0 / CGFloat(segCount) * 0.6

        let bandLayer = CALayer()
        bandLayer.name = "mfx_flux_band_\(band)"
        bandLayer.frame = bounds

        for s in 0..<segCount {
            let startAngle = CGFloat(s) * (CGFloat.pi * 2.0 / CGFloat(segCount))
            let arc = CAShapeLayer()
            let path = CGMutablePath()
            path.addArc(center: CGPoint(x: cx, y: cy), radius: arcR,
                        startAngle: startAngle, endAngle: startAngle + sweep, clockwise: false)
            arc.path = path
            arc.fillColor = NSColor.clear.cgColor
            arc.strokeColor = baseColor.withAlphaComponent(0.16 + 0.12 * frac).cgColor
            arc.lineWidth = 1.2 + frac * 1.8
            arc.lineCap = .round
            bandLayer.addSublayer(arc)
        }

        let spin = CABasicAnimation(keyPath: "transform.rotation")
        spin.fromValue = 0.0
        spin.toValue = (band % 2 == 0) ? Double.pi * 2.0 : -Double.pi * 2.0
        spin.duration = max(0.1, rotateDurationSec * (1.5 + Double(frac) * 0.5))
        spin.repeatCount = .greatestFiniteMagnitude
        bandLayer.add(spin, forKey: "mfx_flux_spin")

        container.addSublayer(bandLayer)
    }

    // Orbit particles (20 dots)
    let particleContainer = CALayer()
    particleContainer.name = "mfx_hold_accent"
    particleContainer.frame = bounds
    contentLayer.addSublayer(particleContainer)

    for i in 0..<20 {
        let angle = CGFloat(i) * (CGFloat.pi * 2.0 / 20.0)
        let r = baseRadius * CGFloat(0.18 + 0.78 * Double((i * 37 + 11) % 100) / 100.0)
        let x = cx + cos(angle) * r
        let y = cy + sin(angle * 1.15) * r * 0.84

        let dot = CALayer()
        dot.frame = CGRect(x: x - 1.5, y: y - 1.5, width: 3.0, height: 3.0)
        dot.cornerRadius = 1.5
        dot.backgroundColor = baseColor.withAlphaComponent(0.36).cgColor

        let orbit = CABasicAnimation(keyPath: "transform.rotation")
        orbit.fromValue = 0.0
        orbit.toValue = (i % 2 == 0) ? Double.pi * 2.0 : -Double.pi * 2.0
        orbit.duration = 3.0 + Double(i % 5) * 0.5
        orbit.repeatCount = .greatestFiniteMagnitude
        dot.add(orbit, forKey: "mfx_flux_orbit")

        particleContainer.addSublayer(dot)
    }

    // Center glow
    let coreSize = size * 0.05
    let core = CAGradientLayer()
    core.type = .radial
    core.startPoint = CGPoint(x: 0.5, y: 0.5)
    core.endPoint = CGPoint(x: 1.0, y: 1.0)
    core.colors = [
        baseColor.withAlphaComponent(0.4).cgColor,
        baseColor.withAlphaComponent(0.0).cgColor
    ]
    core.frame = CGRect(x: cx - coreSize * 3, y: cy - coreSize * 3, width: coreSize * 6, height: coreSize * 6)
    core.cornerRadius = coreSize * 3
    contentLayer.addSublayer(core)

    // Progress arc
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.6).cgColor
    progress.lineWidth = 2.0
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)
}

/// Neon3D: glass ring + inner scanner + progress arc + crystal seed.
/// CoreAnimation approximation of the Windows Neon3D HUD renderer.
@MainActor
private func mfxBuildNeonLayers(
    contentLayer: CALayer,
    bounds: CGRect,
    size: CGFloat,
    baseColor: NSColor,
    baseOpacity: Double,
    breatheDurationSec: Double,
    rotateDurationSec: Double
) {
    let cx = bounds.midX
    let cy = bounds.midY
    let baseRadius = size * 0.38
    let thick = max(12.0, baseRadius * 0.18)

    // 1) Glass ring – gradient fill + stroke
    let glassRing = CAShapeLayer()
    glassRing.name = "mfx_hold_ring"
    glassRing.frame = bounds
    let ringRect = CGRect(x: cx - baseRadius, y: cy - baseRadius, width: baseRadius * 2, height: baseRadius * 2)
    glassRing.path = CGPath(ellipseIn: ringRect, transform: nil)
    glassRing.fillColor = baseColor.withAlphaComponent(0.06).cgColor
    glassRing.strokeColor = baseColor.withAlphaComponent(0.5).cgColor
    glassRing.lineWidth = thick
    contentLayer.addSublayer(glassRing)

    // 2) Inner scanner ring – thinner, pulsing
    let innerR = baseRadius - thick * 0.5
    let innerRect = CGRect(x: cx - innerR, y: cy - innerR, width: innerR * 2, height: innerR * 2)
    let scanner = CAShapeLayer()
    scanner.name = "mfx_hold_accent"
    scanner.frame = bounds
    scanner.path = CGPath(ellipseIn: innerRect, transform: nil)
    scanner.fillColor = NSColor.clear.cgColor
    scanner.strokeColor = baseColor.withAlphaComponent(0.35).cgColor
    scanner.lineWidth = 1.5
    contentLayer.addSublayer(scanner)

    let scanSpin = CABasicAnimation(keyPath: "transform.rotation")
    scanSpin.fromValue = 0.0
    scanSpin.toValue = Double.pi * 2.0
    scanSpin.duration = max(0.1, rotateDurationSec * 0.8)
    scanSpin.repeatCount = .greatestFiniteMagnitude
    scanner.add(scanSpin, forKey: "mfx_neon_scan_spin")

    // 3) Progress arc
    let ringInset = mfxScaleMetric(size, 24.0, 160.0, 10.0, 44.0)
    let progress = CAShapeLayer()
    progress.name = "mfx_hold_progress"
    progress.frame = bounds
    progress.path = CGPath(ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset), transform: nil)
    progress.fillColor = NSColor.clear.cgColor
    progress.strokeColor = baseColor.withAlphaComponent(0.85).cgColor
    progress.lineWidth = 3.0
    progress.lineCap = .round
    progress.strokeStart = 0.0
    progress.strokeEnd = 0.0
    progress.transform = CATransform3DMakeRotation(-CGFloat.pi / 2.0, 0.0, 0.0, 1.0)
    contentLayer.addSublayer(progress)

    // 4) Crystal seed dot at center
    let seedSize: CGFloat = 8.0
    let seed = CALayer()
    seed.name = "mfx_neon_seed"
    seed.frame = CGRect(x: cx - seedSize * 0.5, y: cy - seedSize * 0.5, width: seedSize, height: seedSize)
    seed.cornerRadius = seedSize * 0.5
    seed.backgroundColor = baseColor.withAlphaComponent(0.8).cgColor
    seed.shadowColor = baseColor.cgColor
    seed.shadowOpacity = 0.8
    seed.shadowRadius = seedSize * 2.0
    seed.shadowOffset = .zero
    contentLayer.addSublayer(seed)

    // 5) Head dot
    let head = CALayer()
    head.name = "mfx_hold_head"
    let headSize = max(4.0, thick * 0.3)
    head.bounds = CGRect(x: 0, y: 0, width: headSize, height: headSize)
    head.cornerRadius = headSize * 0.5
    head.backgroundColor = baseColor.withAlphaComponent(0.96).cgColor
    head.shadowColor = baseColor.cgColor
    head.shadowOpacity = 0.9
    head.shadowRadius = headSize * 2
    head.shadowOffset = .zero
    head.position = CGPoint(x: cx, y: cy - baseRadius + ringInset)
    contentLayer.addSublayer(head)

    // Breathe
    let breathe = CABasicAnimation(keyPath: "opacity")
    breathe.fromValue = 0.55
    breathe.toValue = Float(mfxResolveOpacity(CGFloat(baseOpacity), 0.0, 0.0))
    breathe.duration = max(0.05, breatheDurationSec)
    breathe.autoreverses = true
    breathe.repeatCount = .greatestFiniteMagnitude
    glassRing.add(breathe, forKey: "mfx_hold_breathe")
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

    // --- Per-type dispatch ---
    switch style {
    case .charge:
        mfxBuildChargeLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec)

        let ring = mfxFindShapeLayer(contentLayer, named: "mfx_hold_ring")
        let accent = mfxFindShapeLayer(contentLayer, named: "mfx_hold_accent")
        return (
            UInt(bitPattern: windowHandle),
            ring.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accent.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )

    case .lightning:
        mfxBuildLightningLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec,
            rotateDurationFastSec: rotateDurationFastSec)

        // ring handle → core glow (CAGradientLayer found by name), accent → bolt container
        let ringLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_ring" })
        let accentLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_accent" })
        return (
            UInt(bitPattern: windowHandle),
            ringLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accentLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )

    case .hex:
        mfxBuildHexLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec,
            rotateDurationSec: rotateDurationSec)

        let ringLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_ring" })
        let accentLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_accent" })
        return (
            UInt(bitPattern: windowHandle),
            ringLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accentLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )

    case .techRing:
        mfxBuildTechRingLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec,
            rotateDurationSec: rotateDurationSec,
            rotateDurationFastSec: rotateDurationFastSec)

        let ringLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_ring" })
        let accentLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_accent" })
        return (
            UInt(bitPattern: windowHandle),
            ringLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accentLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )

    case .hologram:
        mfxBuildHologramLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec,
            rotateDurationSec: rotateDurationSec,
            rotateDurationFastSec: rotateDurationFastSec)

        let ringLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_ring" })
        let accentLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_accent" })
        return (
            UInt(bitPattern: windowHandle),
            ringLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accentLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )

    case .neon:
        mfxBuildNeonLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec,
            rotateDurationSec: rotateDurationSec)

        let ring = mfxFindShapeLayer(contentLayer, named: "mfx_hold_ring")
        let accent = mfxFindShapeLayer(contentLayer, named: "mfx_hold_accent")
        return (
            UInt(bitPattern: windowHandle),
            ring.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accent.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )

    case .quantumHalo, .fluxField:
        mfxBuildFluxFieldLayers(
            contentLayer: contentLayer,
            bounds: content.bounds,
            size: sizeCGFloat,
            baseColor: baseColor,
            baseOpacity: baseOpacity,
            breatheDurationSec: breatheDurationSec,
            rotateDurationSec: rotateDurationSec,
            rotateDurationFastSec: rotateDurationFastSec)

        let ringLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_ring" })
        let accentLayer = contentLayer.sublayers?.first(where: { $0.name == "mfx_hold_accent" })
        return (
            UInt(bitPattern: windowHandle),
            ringLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0,
            accentLayer.map { UInt(bitPattern: Unmanaged.passUnretained($0).toOpaque()) } ?? 0
        )
    }
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
    holdMs: UInt32,
    holdStyleCode: Int32
) {
    guard
        let windowHandle,
        let ringLayerHandle
    else {
        return
    }
    let window = Unmanaged<NSWindow>.fromOpaque(windowHandle).takeUnretainedValue()
    let ring = Unmanaged<CALayer>.fromOpaque(ringLayerHandle).takeUnretainedValue()
    let accent = accentLayerHandle.map { Unmanaged<CALayer>.fromOpaque($0).takeUnretainedValue() }
    let style = HoldStyleCode(rawValue: holdStyleCode) ?? .charge

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
    let bounds = CGRect(origin: .zero, size: frame.size)
    let progressDenominator = max(1.0, CGFloat(progressFullMs))
    let progress = min(1.0, CGFloat(holdMs) / progressDenominator)

    let baseOpacityValue = CGFloat(baseOpacity)

    // --- Per-type update dispatch ---
    switch style {
    case .charge:
        // Charge: no ring scaling. Pulse ring opacity (matches Windows sine pulse).
        let pulse = 0.5 + 0.5 * sin(Double(holdMs) * 0.0045)
        ring.opacity = Float(0.15 + 0.20 * pulse)
        // Glow arc sweeps with progress
        if let accent, let accentShape = accent as? CAShapeLayer {
            accentShape.strokeEnd = progress
            accentShape.opacity = Float(0.36 * (0.55 + 0.45 * pulse))
        }

    case .lightning:
        // Lightning: pulse core glow, grow bolt opacity with progress
        let fastPulse = 0.5 + 0.5 * sin(Double(holdMs) * 0.05)
        ring.opacity = Float(0.4 + 0.4 * fastPulse)
        // Core orb scales with progress
        if let content = window.contentView, let root = content.layer {
            if let coreLayer = mfxFindLayer(root, named: "mfx_hold_core") {
                let coreScale = 1.0 + progress * 1.5
                coreLayer.transform = CATransform3DMakeScale(coreScale, coreScale, 1.0)
            }
        }

    case .hex:
        // Hex: scale container with progress (Windows multiplies hex radius by progress)
        let hexScale = 0.3 + progress * 0.7
        ring.transform = CATransform3DMakeScale(hexScale, hexScale, 1.0)
        let hexPulse = 0.5 + 0.5 * sin(Double(holdMs) * 0.003)
        ring.opacity = Float(0.5 + 0.4 * hexPulse)

    case .techRing:
        // TechRing: slight expansion + opacity pulse
        let techScale = 0.85 + progress * 0.15
        ring.transform = CATransform3DMakeScale(techScale, techScale, 1.0)
        ring.opacity = Float(0.5 + progress * 0.4)

    case .hologram:
        // Hologram: opacity grows with progress, ring container stays stable
        ring.opacity = Float(0.4 + progress * 0.5)

    case .neon:
        // Neon: progress sweep + head orbit + glow pulse (similar to charge but with glass ring)
        let neonPulse = 0.5 + 0.5 * sin(Double(holdMs) * 0.004)
        if let ringShape = ring as? CAShapeLayer {
            ringShape.opacity = Float(0.45 + 0.35 * neonPulse)
        }

    case .quantumHalo, .fluxField:
        // FluxField/QuantumHalo: container opacity grows with progress
        ring.opacity = Float(0.4 + progress * 0.5)
    }

    if let content = window.contentView, let root = content.layer {
        let ringInset = mfxScaleMetric(width, 24.0, 160.0, 10.0, 44.0)
        if let progressLayer = mfxFindShapeLayer(root, named: "mfx_hold_progress") {
            progressLayer.path = CGPath(
                ellipseIn: bounds.insetBy(dx: ringInset, dy: ringInset),
                transform: nil)
            progressLayer.strokeEnd = progress
            let progressAlpha = mfxResolveOpacity(baseOpacityValue, -0.12 + progress * 0.32, 0.16)
            progressLayer.opacity = Float(progressAlpha)
        }

        if let headLayer = mfxFindLayer(root, named: "mfx_hold_head") {
            let radius = max(1.0, width * 0.5 - ringInset)
            let angle = -CGFloat.pi / 2.0 + progress * CGFloat.pi * 2.0
            let cx = width * 0.5
            let cy = frame.height * 0.5
            headLayer.position = CGPoint(
                x: cx + cos(angle) * radius,
                y: cy + sin(angle) * radius)
            headLayer.opacity = Float(mfxResolveOpacity(baseOpacityValue, -0.08 + progress * 0.38, 0.2))
        }
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
    _ holdMs: UInt32,
    _ holdStyleCode: Int32
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
                holdMs: holdMs,
                holdStyleCode: holdStyleCode
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
                holdMs: holdMs,
                holdStyleCode: holdStyleCode
            )
        }
    }
}
