@preconcurrency import AppKit
@preconcurrency import Foundation
@preconcurrency import QuartzCore
@preconcurrency import SceneKit
import simd

private enum MfxMouseCompanionActionCode: Int32 {
    case idle = 0
    case follow = 1
    case clickReact = 2
    case drag = 3
    case holdReact = 4
    case scrollReact = 5
}

private enum MfxMouseCompanionPositionMode: Int32 {
    case follow = 0
    case fixedBottomLeft = 1
}

private struct MfxActionKeyframe {
    let t: CGFloat
    let rotation: simd_quatf?
    let scale: SIMD3<Float>?
}

private struct MfxActionTrack {
    let bone: String
    let keyframes: [MfxActionKeyframe]
}

private struct MfxActionClip {
    let action: String
    let duration: CGFloat
    let loop: Bool
    let tracks: [MfxActionTrack]
}

private struct MfxActionTrackSample {
    let rotation: simd_quatf?
    let scale: SIMD3<Float>?
}

private struct MfxGlbNodeMetadata {
    let name: String?
    let children: [Int]
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

private func mfxImpulseProfile(_ t: CGFloat, inRatio: CGFloat, holdRatio: CGFloat) -> CGFloat {
    let x = mfxClamp(t, min: 0.0, max: 1.0)
    let inPart = mfxClamp(inRatio, min: 0.05, max: 0.9)
    let holdPart = mfxClamp(holdRatio, min: 0.0, max: 0.7)
    let outStart = mfxClamp(inPart + holdPart, min: inPart, max: 0.98)
    if x < inPart {
        let p = x / inPart
        let eased = 1.0 - pow(1.0 - p, 3.0)
        return mfxClamp(eased, min: 0.0, max: 1.0)
    }
    if x < outStart {
        return 1.0
    }
    let p = (x - outStart) / max(0.001, 1.0 - outStart)
    let eased = 1.0 - p * p * (3.0 - 2.0 * p)
    return mfxClamp(eased, min: 0.0, max: 1.0)
}

private func mfxYawFromQuaternion(_ x: CGFloat, _ y: CGFloat, _ z: CGFloat, _ w: CGFloat) -> CGFloat {
    let siny = 2.0 * (w * z + x * y)
    let cosy = 1.0 - 2.0 * (y * y + z * z)
    return atan2(siny, cosy)
}

@MainActor
private final class MfxMouseCompanionPanelView: NSView {
    private var actionCode: MfxMouseCompanionActionCode = .idle
    private var actionIntensity: CGFloat = 0.0
    private var headTintAmount: CGFloat = 0.0
    private var clickPulse: CGFloat = 0.0
    private var holdPulse: CGFloat = 0.0
    private var scrollPulse: CGFloat = 0.0
    private var poseBindingConfigured = false
    private var poseEarSpread: CGFloat = 0.0
    private var poseEarLift: CGFloat = 0.0
    private var poseHandLift: CGFloat = 0.0
    private var poseHandSpread: CGFloat = 0.0
    private var poseLegSpread: CGFloat = 0.0
    private var poseLegKick: CGFloat = 0.0
    private var bobTime: CGFloat = 0.0
    private var frameTimer: Timer?
    private var lastTick: CFTimeInterval = CACurrentMediaTime()

    override var isOpaque: Bool {
        false
    }

    func start() {
        if frameTimer != nil {
            return
        }
        frameTimer = Timer.scheduledTimer(
            timeInterval: 1.0 / 60.0,
            target: self,
            selector: #selector(onFrame(_:)),
            userInfo: nil,
            repeats: true)
    }

    func stop() {
        frameTimer?.invalidate()
        frameTimer = nil
    }

    func updateState(actionCode: Int32, actionIntensity: Float, headTintAmount: Float) {
        let resolvedAction = MfxMouseCompanionActionCode(rawValue: actionCode) ?? .idle
        let resolvedIntensity = mfxClamp(CGFloat(actionIntensity), min: 0.0, max: 1.0)
        let resolvedTint = mfxClamp(CGFloat(headTintAmount), min: 0.0, max: 1.0)

        if resolvedAction == .clickReact {
            clickPulse = max(clickPulse, 1.0)
        }
        if resolvedAction == .holdReact {
            holdPulse = max(holdPulse, max(0.35, resolvedIntensity))
        }
        if resolvedAction == .scrollReact {
            scrollPulse = max(scrollPulse, max(0.30, resolvedIntensity))
        }

        self.actionCode = resolvedAction
        self.actionIntensity = resolvedIntensity
        self.headTintAmount = resolvedTint
        needsDisplay = true
    }

    func configurePoseBinding(names: [String]) -> Bool {
        poseBindingConfigured = !names.isEmpty
        if !poseBindingConfigured {
            poseEarSpread = 0.0
            poseEarLift = 0.0
            poseHandLift = 0.0
            poseHandSpread = 0.0
            poseLegSpread = 0.0
            poseLegKick = 0.0
        }
        return poseBindingConfigured
    }

    func applyPose(indices: [Int32], positions: [Float], rotations: [Float], scales: [Float]) {
        guard poseBindingConfigured else {
            return
        }
        let count = indices.count
        if positions.count < count * 3 || rotations.count < count * 4 || scales.count < count * 3 {
            return
        }

        var leftEarX: CGFloat = 0.0
        var leftEarY: CGFloat = 0.0
        var rightEarX: CGFloat = 0.0
        var rightEarY: CGFloat = 0.0
        var leftEarYaw: CGFloat = 0.0
        var rightEarYaw: CGFloat = 0.0
        var leftHandY: CGFloat = 0.0
        var rightHandY: CGFloat = 0.0
        var leftHandYaw: CGFloat = 0.0
        var rightHandYaw: CGFloat = 0.0
        var leftLegYaw: CGFloat = 0.0
        var rightLegYaw: CGFloat = 0.0

        for i in 0..<count {
            let idx = Int(indices[i])
            if idx < 0 {
                continue
            }
            let pBase = i * 3
            let rBase = i * 4
            let px = CGFloat(positions[pBase])
            let py = CGFloat(positions[pBase + 1])
            let qx = CGFloat(rotations[rBase])
            let qy = CGFloat(rotations[rBase + 1])
            let qz = CGFloat(rotations[rBase + 2])
            let qw = CGFloat(rotations[rBase + 3])
            let yawZ = mfxYawFromQuaternion(qx, qy, qz, qw)

            switch idx {
            case 0:
                leftEarX = px
                leftEarY = py
                leftEarYaw = yawZ
            case 1:
                rightEarX = px
                rightEarY = py
                rightEarYaw = yawZ
            case 2:
                leftHandY = py
                leftHandYaw = yawZ
            case 3:
                rightHandY = py
                rightHandYaw = yawZ
            case 4:
                leftLegYaw = yawZ
            case 5:
                rightLegYaw = yawZ
            default:
                break
            }
        }

        let earSpread = mfxClamp((abs(leftEarYaw) + abs(rightEarYaw)) * 0.7 + (abs(leftEarX) + abs(rightEarX)) * 1.8, min: 0.0, max: 1.0)
        let earLift = mfxClamp((leftEarY + rightEarY) * 2.4, min: 0.0, max: 1.0)
        let handLift = mfxClamp((leftHandY + rightHandY) * 2.0, min: 0.0, max: 1.0)
        let handSpread = mfxClamp((abs(leftHandYaw) + abs(rightHandYaw)) * 0.9, min: 0.0, max: 1.0)
        let legSpread = mfxClamp((abs(leftLegYaw) + abs(rightLegYaw)) * 1.0, min: 0.0, max: 1.0)
        let legKick = mfxClamp((abs(leftLegYaw) + abs(rightLegYaw)) * 0.9, min: 0.0, max: 1.0)

        poseEarSpread = earSpread
        poseEarLift = earLift
        poseHandLift = handLift
        poseHandSpread = handSpread
        poseLegSpread = legSpread
        poseLegKick = legKick
        needsDisplay = true
    }

    @objc private func onFrame(_ timer: Timer) {
        let now = CACurrentMediaTime()
        let dt = CGFloat(max(0.0, min(0.08, now - lastTick)))
        lastTick = now

        bobTime += dt
        clickPulse = max(0.0, clickPulse - dt * 3.6)
        holdPulse = max(0.0, holdPulse - dt * 1.2)
        scrollPulse = max(0.0, scrollPulse - dt * 1.8)

        needsDisplay = true
    }

    override func draw(_ dirtyRect: NSRect) {
        NSColor.clear.setFill()
        dirtyRect.fill()

        let side = min(bounds.width, bounds.height)
        if side <= 1.0 {
            return
        }

        let clickT = mfxClamp(1.0 - clickPulse, min: 0.0, max: 1.0)
        let clickProfileBase = mfxImpulseProfile(clickT, inRatio: 0.42, holdRatio: 0.16)
        let holdProfileBase = mfxClamp(max(holdPulse, (actionCode == .holdReact ? actionIntensity : 0.0)), min: 0.0, max: 1.0)
        let scrollProfileBase = mfxClamp(scrollPulse, min: 0.0, max: 1.0)
        let clickProfile = max(clickProfileBase, poseHandSpread, poseLegSpread, poseEarSpread * 0.9)
        let holdProfile = max(holdProfileBase, poseHandLift * 0.7)
        let scrollProfile = max(scrollProfileBase, poseEarSpread * 0.8, poseLegKick * 0.7)

        let bobOffset = sin(Double(bobTime * 2.1)) * Double(2.0 + holdProfile * 3.0)
        let followTilt = (actionCode == .follow || actionCode == .drag) ? (actionIntensity * 8.0) : 0.0
        let clickScale = 1.0 + clickProfile * 0.10
        let holdScale = 1.0 + holdProfile * 0.05
        let scrollScale = 1.0 + scrollProfile * 0.04
        let actionScale = clickScale * holdScale * scrollScale

        NSGraphicsContext.saveGraphicsState()
        let transform = NSAffineTransform()
        transform.translateX(by: bounds.midX, yBy: bounds.midY + CGFloat(bobOffset))
        transform.rotate(byDegrees: followTilt * (actionCode == .drag ? -1.0 : 1.0))
        transform.scale(by: actionScale)
        transform.concat()

        drawShadow(side: side, clickProfile: clickProfile)
        drawLimbs(side: side, clickProfile: clickProfile, holdProfile: holdProfile, scrollProfile: scrollProfile)
        drawBody(side: side, clickProfile: clickProfile, holdProfile: holdProfile)
        drawHead(side: side, clickProfile: clickProfile, holdProfile: holdProfile, scrollProfile: scrollProfile)
        drawFace(side: side, clickProfile: clickProfile, scrollProfile: scrollProfile, holdProfile: holdProfile)
        drawActionAura(side: side, clickProfile: clickProfile, holdProfile: holdProfile, scrollProfile: scrollProfile)

        NSGraphicsContext.restoreGraphicsState()
    }

    private func drawShadow(side: CGFloat, clickProfile: CGFloat) {
        let shadowRect = NSRect(x: -side * 0.20, y: -side * 0.40, width: side * 0.40, height: side * 0.11)
        let alpha = 0.22 + 0.08 * clickProfile
        NSColor(calibratedWhite: 0.0, alpha: alpha).setFill()
        NSBezierPath(ovalIn: shadowRect).fill()
    }

    private func drawLimbs(side: CGFloat, clickProfile: CGFloat, holdProfile: CGFloat, scrollProfile: CGFloat) {
        let handSpread = side * (0.17 + clickProfile * 0.03 + poseHandSpread * 0.03)
        let handBaseY = side * (0.01 + holdProfile * 0.01)
        let handLift = side * (clickProfile * 0.10 + scrollProfile * 0.06 + holdProfile * 0.04 + poseHandLift * 0.08)
        let handTwist = 34.0 * clickProfile + 18.0 * scrollProfile + 12.0 * holdProfile + 24.0 * poseHandSpread
        let handRect = NSRect(x: -side * 0.06, y: -side * 0.05, width: side * 0.12, height: side * 0.14)

        drawLimbOval(centerX: -handSpread, centerY: handBaseY + handLift, degrees: handTwist, rect: handRect)
        drawLimbOval(centerX: handSpread, centerY: handBaseY + handLift, degrees: -handTwist, rect: handRect)

        let legSpread = side * (0.10 + clickProfile * 0.03 + scrollProfile * 0.02 + poseLegSpread * 0.03)
        let legY = -side * 0.20
        let legKick = 16.0 * clickProfile + 8.0 * scrollProfile + 14.0 * poseLegKick
        let legRect = NSRect(x: -side * 0.055, y: -side * 0.025, width: side * 0.11, height: side * 0.09)
        drawLimbOval(centerX: -legSpread, centerY: legY, degrees: -legKick, rect: legRect)
        drawLimbOval(centerX: legSpread, centerY: legY, degrees: legKick, rect: legRect)
    }

    private func drawLimbOval(centerX: CGFloat, centerY: CGFloat, degrees: CGFloat, rect: NSRect) {
        NSGraphicsContext.saveGraphicsState()
        let tf = NSAffineTransform()
        tf.translateX(by: centerX, yBy: centerY)
        tf.rotate(byDegrees: degrees)
        tf.concat()
        let path = NSBezierPath(roundedRect: rect, xRadius: rect.width * 0.5, yRadius: rect.width * 0.5)
        NSColor(calibratedRed: 0.96, green: 0.98, blue: 1.0, alpha: 0.98).setFill()
        path.fill()
        NSColor(calibratedRed: 0.74, green: 0.79, blue: 0.90, alpha: 0.92).setStroke()
        path.lineWidth = 1.1
        path.stroke()
        NSGraphicsContext.restoreGraphicsState()
    }

    private func drawBody(side: CGFloat, clickProfile: CGFloat, holdProfile: CGFloat) {
        let bodyRect = NSRect(x: -side * 0.20, y: -side * 0.30, width: side * 0.40, height: side * 0.44)
        let bodyPath = NSBezierPath(roundedRect: bodyRect, xRadius: side * 0.20, yRadius: side * 0.20)

        let bodyTop = NSColor(calibratedRed: 0.98, green: 0.99 - holdProfile * 0.04, blue: 1.0, alpha: 0.95)
        let bodyBottom = NSColor(calibratedRed: 0.93, green: 0.96 - holdProfile * 0.05, blue: 1.0, alpha: 0.95)
        let bodyGradient = NSGradient(colors: [bodyTop, bodyBottom])
        bodyGradient?.draw(in: bodyPath, angle: -90.0)

        NSColor(calibratedRed: 0.76, green: 0.81, blue: 0.90, alpha: 0.9).setStroke()
        bodyPath.lineWidth = 1.4 + clickProfile * 0.2
        bodyPath.stroke()
    }

    private func drawHead(side: CGFloat, clickProfile: CGFloat, holdProfile: CGFloat, scrollProfile: CGFloat) {
        let earLift = (clickProfile * 0.04 + scrollProfile * 0.03 + poseEarLift * 0.05) * side
        let earSpread = (clickProfile * 0.03 + scrollProfile * 0.05 + poseEarSpread * 0.06) * side
        let leftEarRect = NSRect(x: -side * 0.17 - earSpread, y: side * 0.13 + earLift, width: side * 0.12, height: side * 0.36)
        let rightEarRect = NSRect(x: side * 0.05 + earSpread, y: side * 0.13 + earLift, width: side * 0.12, height: side * 0.36)
        let headRect = NSRect(x: -side * 0.24, y: -side * 0.02, width: side * 0.48, height: side * 0.40)

        drawEar(rect: leftEarRect, side: side, clickProfile: clickProfile)
        drawEar(rect: rightEarRect, side: side, clickProfile: clickProfile)

        let headPath = NSBezierPath(ovalIn: headRect)
        let tint = headTintAmount
        let warmTop = NSColor(
            calibratedRed: 0.98,
            green: 0.98 - tint * 0.20 - holdProfile * 0.03,
            blue: 1.0 - tint * 0.35,
            alpha: 0.98)
        let warmBottom = NSColor(
            calibratedRed: 0.94,
            green: 0.95 - tint * 0.18 - holdProfile * 0.04,
            blue: 0.99 - tint * 0.28,
            alpha: 0.98)

        let headGradient = NSGradient(colors: [warmTop, warmBottom])
        headGradient?.draw(in: headPath, angle: -90.0)

        NSColor(calibratedRed: 0.74, green: 0.79, blue: 0.90, alpha: 0.92).setStroke()
        headPath.lineWidth = 1.6
        headPath.stroke()
    }

    private func drawEar(rect: NSRect, side: CGFloat, clickProfile: CGFloat) {
        let earPath = NSBezierPath(roundedRect: rect, xRadius: side * 0.06, yRadius: side * 0.06)
        NSColor(calibratedRed: 0.96, green: 0.98, blue: 1.0, alpha: 0.98).setFill()
        earPath.fill()
        NSColor(calibratedRed: 0.74, green: 0.79, blue: 0.90, alpha: 0.92).setStroke()
        earPath.lineWidth = 1.3
        earPath.stroke()

        let innerInsetX = rect.width * 0.24
        let innerInsetY = rect.height * 0.22
        let innerRect = rect.insetBy(dx: innerInsetX, dy: innerInsetY)
        NSColor(calibratedRed: 1.0, green: 0.78, blue: 0.84, alpha: 0.66 + 0.20 * clickProfile).setFill()
        NSBezierPath(roundedRect: innerRect, xRadius: innerRect.width * 0.5, yRadius: innerRect.width * 0.5).fill()
    }

    private func drawFace(side: CGFloat, clickProfile: CGFloat, scrollProfile: CGFloat, holdProfile: CGFloat) {
        let eyeY = side * 0.14
        let eyeOffsetX = side * 0.09
        let eyeWidth = side * 0.045
        let eyeHeight = side * 0.065

        if actionCode == .clickReact {
            drawSmileEye(centerX: -eyeOffsetX, centerY: eyeY, width: eyeWidth * 1.3)
            drawSmileEye(centerX: eyeOffsetX, centerY: eyeY, width: eyeWidth * 1.3)
        } else {
            let blink = (actionCode == .holdReact) ? mfxClamp(holdProfile * 0.6, min: 0.0, max: 0.8) : 0.0
            let actualEyeHeight = eyeHeight * (1.0 - blink)
            NSColor(calibratedWhite: 0.18, alpha: 0.95).setFill()
            NSBezierPath(ovalIn: NSRect(x: -eyeOffsetX - eyeWidth * 0.5, y: eyeY - actualEyeHeight * 0.5, width: eyeWidth, height: max(2.0, actualEyeHeight))).fill()
            NSBezierPath(ovalIn: NSRect(x: eyeOffsetX - eyeWidth * 0.5, y: eyeY - actualEyeHeight * 0.5, width: eyeWidth, height: max(2.0, actualEyeHeight))).fill()
        }

        let noseRect = NSRect(x: -side * 0.018, y: side * 0.075, width: side * 0.036, height: side * 0.026)
        NSColor(calibratedRed: 1.0, green: 0.56, blue: 0.66, alpha: 0.90).setFill()
        NSBezierPath(ovalIn: noseRect).fill()

        let mouthPath = NSBezierPath()
        mouthPath.move(to: NSPoint(x: 0.0, y: side * 0.067))
        mouthPath.line(to: NSPoint(x: 0.0, y: side * 0.038))
        mouthPath.curve(to: NSPoint(x: -side * 0.045, y: side * 0.017), controlPoint1: NSPoint(x: -side * 0.006, y: side * 0.028), controlPoint2: NSPoint(x: -side * 0.026, y: side * 0.012))
        mouthPath.move(to: NSPoint(x: 0.0, y: side * 0.038))
        mouthPath.curve(to: NSPoint(x: side * 0.045, y: side * 0.017), controlPoint1: NSPoint(x: side * 0.006, y: side * 0.028), controlPoint2: NSPoint(x: side * 0.026, y: side * 0.012))
        NSColor(calibratedWhite: 0.25, alpha: 0.88).setStroke()
        mouthPath.lineWidth = 1.3
        mouthPath.lineCapStyle = .round
        mouthPath.stroke()

        let blushAlpha = 0.18 + 0.20 * max(clickProfile, scrollProfile)
        NSColor(calibratedRed: 1.0, green: 0.68, blue: 0.73, alpha: blushAlpha).setFill()
        NSBezierPath(ovalIn: NSRect(x: -side * 0.17, y: side * 0.07, width: side * 0.07, height: side * 0.04)).fill()
        NSBezierPath(ovalIn: NSRect(x: side * 0.10, y: side * 0.07, width: side * 0.07, height: side * 0.04)).fill()
    }

    private func drawSmileEye(centerX: CGFloat, centerY: CGFloat, width: CGFloat) {
        let path = NSBezierPath()
        path.move(to: NSPoint(x: centerX - width * 0.5, y: centerY))
        path.curve(to: NSPoint(x: centerX + width * 0.5, y: centerY),
                   controlPoint1: NSPoint(x: centerX - width * 0.2, y: centerY + width * 0.32),
                   controlPoint2: NSPoint(x: centerX + width * 0.2, y: centerY + width * 0.32))
        NSColor(calibratedWhite: 0.2, alpha: 0.96).setStroke()
        path.lineWidth = 1.8
        path.lineCapStyle = .round
        path.stroke()
    }

    private func drawActionAura(side: CGFloat, clickProfile: CGFloat, holdProfile: CGFloat, scrollProfile: CGFloat) {
        let auraStrength = max(clickProfile, holdProfile, scrollProfile)
        if auraStrength <= 0.001 {
            return
        }
        let auraRadius = side * (0.34 + auraStrength * 0.18)
        let auraRect = NSRect(x: -auraRadius, y: -side * 0.05 - auraRadius, width: auraRadius * 2.0, height: auraRadius * 2.0)
        let auraColor: NSColor
        switch actionCode {
        case .scrollReact:
            auraColor = NSColor(calibratedRed: 0.42, green: 0.82, blue: 1.0, alpha: 0.24 * auraStrength)
        case .holdReact:
            auraColor = NSColor(calibratedRed: 1.0, green: 0.77, blue: 0.54, alpha: 0.22 * auraStrength)
        default:
            auraColor = NSColor(calibratedRed: 1.0, green: 0.67, blue: 0.73, alpha: 0.22 * auraStrength)
        }
        auraColor.setStroke()
        let auraPath = NSBezierPath(ovalIn: auraRect)
        auraPath.lineWidth = 2.0
        auraPath.stroke()
    }
}

@MainActor
private final class MfxMouseCompanionPanelHandle: NSObject {
    private let panel: NSPanel
    private let contentView: NSView
    private let companionView: MfxMouseCompanionPanelView
    private let sceneView: SCNView
    private let sceneRootNode = SCNNode()
    private let sceneCameraNode = SCNNode()
    private let targetPetSizePx: CGFloat
    private var panelCanvasSize: CGSize
    private let baseCameraPosition = SCNVector3(x: 0.0, y: 0.24, z: 3.2)
    private var positionMode: MfxMouseCompanionPositionMode = .fixedBottomLeft
    private var offsetX: CGFloat
    private var offsetY: CGFloat
    private var modelNode: SCNNode?
    private var modelLoaded = false
    private var boneNodesByName: [String: [SCNNode]] = [:]
    private var poseBindingNodesByIndex: [[SCNNode]] = []
    private var restLocalTransformByNode: [ObjectIdentifier: simd_float4x4] = [:]
    private var semanticChestNodes: [SCNNode] = []
    private var semanticHeadNodes: [SCNNode] = []
    private var clickActionClip: MfxActionClip?
    private var actionBoneRemapLower: [String: [String]] = [:]
    private var actionTrackNodeCache: [String: [SCNNode]] = [:]
    private var headTintTargetNodes: [SCNNode] = []
    private var headTintBaseMultiplyByMaterial: [ObjectIdentifier: NSColor] = [:]
    private var appliedHeadTintAmount: CGFloat = -1.0
    private let headTintColor = NSColor(calibratedRed: 1.0, green: 0.30, blue: 0.30, alpha: 1.0)
    private var modelBaseScale: CGFloat = 1.0
    private var modelBasePosition = SCNVector3Zero
    private let modelFacingYaw: CGFloat = .pi
    private var lastModelActionCode: Int32 = -1
    private var modelBobTime: CGFloat = 0.0
    private var modelLastTick: CFTimeInterval = CACurrentMediaTime()
    private var clickOneShotActive = false
    private var clickOneShotElapsed: CGFloat = 0.0
    private var clickOneShotDuration: CGFloat = 0.30
    private var pendingInitialPanelFit = false
    private var modelFrameTimer: Timer?
    private var currentActionCode: Int32 = MfxMouseCompanionActionCode.idle.rawValue
    private var currentActionIntensity: Float = 0.0
    private var currentHeadTintAmount: Float = 0.0

    init(sizePx: Int, offsetX: Int, offsetY: Int) {
        let side = CGFloat(max(96, sizePx))
        targetPetSizePx = side
        panelCanvasSize = CGSize(width: side, height: side)
        panel = NSPanel(
            contentRect: NSRect(x: 0, y: 0, width: side, height: side),
            styleMask: .borderless,
            backing: .buffered,
            defer: false)
        panel.isOpaque = false
        panel.backgroundColor = .clear
        panel.hasShadow = false
        panel.ignoresMouseEvents = true
        panel.hidesOnDeactivate = false
        panel.level = .statusBar
        panel.collectionBehavior = [.canJoinAllSpaces, .fullScreenAuxiliary]

        contentView = NSView(frame: NSRect(x: 0, y: 0, width: side, height: side))
        contentView.wantsLayer = true
        contentView.layer?.backgroundColor = NSColor.clear.cgColor
        contentView.layer?.borderWidth = 2.0
        contentView.layer?.borderColor = NSColor(
            calibratedRed: 0.12,
            green: 0.68,
            blue: 1.0,
            alpha: 0.92).cgColor
        contentView.layer?.cornerRadius = 6.0

        companionView = MfxMouseCompanionPanelView(frame: NSRect(x: 0, y: 0, width: side, height: side))
        companionView.wantsLayer = true
        sceneView = SCNView(frame: NSRect(x: 0, y: 0, width: side, height: side))
        sceneView.wantsLayer = true
        sceneView.backgroundColor = .clear
        sceneView.autoenablesDefaultLighting = false
        sceneView.rendersContinuously = true
        sceneView.antialiasingMode = .multisampling4X
        sceneView.isPlaying = true
        sceneView.allowsCameraControl = false
        sceneView.autoresizingMask = [.width, .height]
        sceneView.isHidden = true

        let scene = SCNScene()
        scene.rootNode.addChildNode(sceneRootNode)
        sceneView.scene = scene

        sceneCameraNode.camera = SCNCamera()
        sceneCameraNode.camera?.zNear = 0.01
        sceneCameraNode.camera?.zFar = 1000.0
        sceneCameraNode.camera?.fieldOfView = 64.0
        sceneCameraNode.position = baseCameraPosition
        scene.rootNode.addChildNode(sceneCameraNode)
        sceneView.pointOfView = sceneCameraNode

        let keyLight = SCNNode()
        keyLight.light = SCNLight()
        keyLight.light?.type = .directional
        keyLight.light?.intensity = 620
        keyLight.eulerAngles = SCNVector3(x: -0.78, y: 0.95, z: 0.0)
        scene.rootNode.addChildNode(keyLight)

        let fillLight = SCNNode()
        fillLight.light = SCNLight()
        fillLight.light?.type = .ambient
        fillLight.light?.intensity = 700
        scene.rootNode.addChildNode(fillLight)

        contentView.addSubview(sceneView)
        contentView.addSubview(companionView)
        panel.contentView = contentView

        self.offsetX = CGFloat(offsetX)
        self.offsetY = CGFloat(offsetY)

        super.init()
        companionView.start()
        startModelFrameLoop()
    }

    func show() {
        if positionMode == .fixedBottomLeft {
            setFixedBottomLeftOrigin()
        }
        panel.orderFrontRegardless()
    }

    func hide() {
        panel.orderOut(nil)
    }

    func closeAndCleanup() {
        hide()
        stopModelFrameLoop()
        companionView.stop()
        sceneView.removeFromSuperview()
        companionView.removeFromSuperview()
        panel.close()
    }

    func configure(positionModeCode: Int32, offsetX: Int32, offsetY: Int32) {
        positionMode = MfxMouseCompanionPositionMode(rawValue: positionModeCode) ?? .fixedBottomLeft
        self.offsetX = CGFloat(offsetX)
        self.offsetY = CGFloat(offsetY)
        if positionMode == .fixedBottomLeft {
            setFixedBottomLeftOrigin()
        }
    }

    func update(actionCode: Int32, actionIntensity: Float, headTintAmount: Float) {
        currentActionCode = actionCode
        currentActionIntensity = actionIntensity
        currentHeadTintAmount = headTintAmount

        if actionCode == MfxMouseCompanionActionCode.clickReact.rawValue {
            beginClickOneShot()
        }

        if modelLoaded, let modelNode {
            updateLoadedModel(
                node: modelNode,
                actionCode: actionCode,
                actionIntensity: actionIntensity,
                headTintAmount: headTintAmount)
            return
        }
        companionView.updateState(
            actionCode: actionCode,
            actionIntensity: actionIntensity,
            headTintAmount: headTintAmount)
    }

    func loadModel(path: String) -> Bool {
        let url = URL(fileURLWithPath: path)
        guard FileManager.default.fileExists(atPath: url.path) else {
            return false
        }
        guard let scene = loadScene(url: url) else {
            return false
        }
        guard !scene.rootNode.childNodes.isEmpty else {
            return false
        }

        let container = SCNNode()
        for child in scene.rootNode.childNodes {
            container.addChildNode(child.clone())
        }
        hydrateAnonymousSkeletonNamesIfNeeded(root: container, sourceURL: url)
        disableImportedAnimations(root: container)
        guard containsRenderableGeometry(root: container) else {
            return false
        }

        modelNode?.removeFromParentNode()
        sceneRootNode.addChildNode(container)
        modelNode = container
        rebuildBoneNodeIndex(root: container)
        resolveSemanticCoreBones()
        rebuildHeadTintTargets(root: container)
        actionTrackNodeCache.removeAll(keepingCapacity: true)
        poseBindingNodesByIndex.removeAll(keepingCapacity: true)
        sceneView.isHidden = false
        companionView.isHidden = true
        normalizeModelTransform()
        pendingInitialPanelFit = true
        modelLoaded = true
        modelLastTick = CACurrentMediaTime()
        lastModelActionCode = -1
        clickOneShotActive = false
        clickOneShotElapsed = 0.0
        updateLoadedModel(
            node: container,
            actionCode: currentActionCode,
            actionIntensity: currentActionIntensity,
            headTintAmount: currentHeadTintAmount)
        return true
    }

    func loadActionLibrary(path: String) -> Bool {
        let url = URL(fileURLWithPath: path)
        guard FileManager.default.fileExists(atPath: url.path),
              let data = try? Data(contentsOf: url),
              let rootAny = try? JSONSerialization.jsonObject(with: data),
              let root = rootAny as? [String: Any] else {
            clickActionClip = nil
            actionBoneRemapLower.removeAll(keepingCapacity: true)
            actionTrackNodeCache.removeAll(keepingCapacity: true)
            clickOneShotDuration = 0.30
            return false
        }

        var remapLower: [String: [String]] = [:]
        if let remap = root["bone_remap"] as? [String: Any] {
            for (key, value) in remap {
                guard let aliasesAny = value as? [Any] else {
                    continue
                }
                let aliases = aliasesAny.compactMap { $0 as? String }
                if !aliases.isEmpty {
                    remapLower[key.lowercased()] = aliases
                }
            }
        }

        var parsedClickClip: MfxActionClip?
        if let clips = root["clips"] as? [Any] {
            for clipAny in clips {
                guard let clipDict = clipAny as? [String: Any],
                      let action = clipDict["action"] as? String,
                      action == "clickReact" else {
                    continue
                }
                guard let tracksAny = clipDict["tracks"] as? [Any] else {
                    continue
                }
                var tracks: [MfxActionTrack] = []
                tracks.reserveCapacity(tracksAny.count)
                for trackAny in tracksAny {
                    guard let trackDict = trackAny as? [String: Any],
                          let bone = trackDict["bone"] as? String,
                          let keyframesAny = trackDict["keyframes"] as? [Any] else {
                        continue
                    }
                    var keyframes: [MfxActionKeyframe] = []
                    keyframes.reserveCapacity(keyframesAny.count)
                    for keyAny in keyframesAny {
                        guard let keyDict = keyAny as? [String: Any],
                              let tValue = keyDict["t"] as? NSNumber else {
                            continue
                        }
                        var rotation: simd_quatf?
                        if let rotAny = keyDict["rot"] as? [Any], rotAny.count >= 4,
                           let x = rotAny[0] as? NSNumber,
                           let y = rotAny[1] as? NSNumber,
                           let z = rotAny[2] as? NSNumber,
                           let w = rotAny[3] as? NSNumber {
                            var q = simd_quatf(ix: x.floatValue, iy: y.floatValue, iz: z.floatValue, r: w.floatValue)
                            if simd_length_squared(q.vector) >= 0.000001 {
                                q = simd_normalize(q)
                                rotation = q
                            }
                        }
                        var scale: SIMD3<Float>?
                        if let scaleAny = keyDict["scale"] as? [Any], scaleAny.count >= 3,
                           let sx = scaleAny[0] as? NSNumber,
                           let sy = scaleAny[1] as? NSNumber,
                           let sz = scaleAny[2] as? NSNumber {
                            scale = SIMD3<Float>(sx.floatValue, sy.floatValue, sz.floatValue)
                        }
                        keyframes.append(MfxActionKeyframe(
                            t: CGFloat(tValue.doubleValue),
                            rotation: rotation,
                            scale: scale))
                    }
                    if keyframes.isEmpty {
                        continue
                    }
                    keyframes.sort { lhs, rhs in
                        lhs.t < rhs.t
                    }
                    tracks.append(MfxActionTrack(bone: bone, keyframes: keyframes))
                }
                if tracks.isEmpty {
                    continue
                }
                let durationValue = (clipDict["duration"] as? NSNumber)?.doubleValue ?? 0.30
                let duration = mfxClamp(CGFloat(durationValue), min: 0.05, max: 5.0)
                let loop = (clipDict["loop"] as? Bool) ?? true
                parsedClickClip = MfxActionClip(action: action, duration: duration, loop: loop, tracks: tracks)
                break
            }
        }

        clickActionClip = parsedClickClip
        actionBoneRemapLower = remapLower
        actionTrackNodeCache.removeAll(keepingCapacity: true)
        clickOneShotDuration = parsedClickClip?.duration ?? 0.30
        return parsedClickClip != nil
    }

    func configurePoseBinding(names: [String]) -> Bool {
        guard modelLoaded else {
            return companionView.configurePoseBinding(names: names)
        }
        guard !names.isEmpty else {
            poseBindingNodesByIndex.removeAll(keepingCapacity: true)
            return false
        }
        var mapping: [[SCNNode]] = []
        mapping.reserveCapacity(names.count)
        for name in names {
            let key = Self.normalizedBoneName(name)
            if let key, let nodes = boneNodesByName[key], !nodes.isEmpty {
                mapping.append(nodes)
            } else {
                mapping.append([])
            }
        }
        poseBindingNodesByIndex = mapping
        return !poseBindingNodesByIndex.isEmpty
    }

    func applyPose(indices: [Int32], positions: [Float], rotations: [Float], scales: [Float]) {
        guard modelLoaded else {
            companionView.applyPose(indices: indices, positions: positions, rotations: rotations, scales: scales)
            return
        }
        guard modelNode != nil, !indices.isEmpty, !poseBindingNodesByIndex.isEmpty else {
            return
        }
        if clickOneShotActive {
            // Keep click window visually deterministic: one-shot press/rebound only.
            restoreAllPoseBindingNodesToRest()
            return
        }
        let poseCount = indices.count
        if positions.count < poseCount * 3 || rotations.count < poseCount * 4 || scales.count < poseCount * 3 {
            return
        }

        for nodes in poseBindingNodesByIndex {
            for node in nodes {
                let nodeId = ObjectIdentifier(node)
                guard let restLocal = restLocalTransformByNode[nodeId] else {
                    continue
                }
                node.simdTransform = restLocal
            }
        }

        for idx in 0..<poseCount {
            let poseBoneIndex = Int(indices[idx])
            if poseBoneIndex < 0 || poseBoneIndex >= poseBindingNodesByIndex.count {
                continue
            }
            let nodes = poseBindingNodesByIndex[poseBoneIndex]
            if nodes.isEmpty {
                continue
            }
            let pBase = idx * 3
            let rBase = idx * 4
            let localDelta = composeLocalDelta(
                positionX: positions[pBase],
                positionY: positions[pBase + 1],
                positionZ: positions[pBase + 2],
                rotationX: rotations[rBase],
                rotationY: rotations[rBase + 1],
                rotationZ: rotations[rBase + 2],
                rotationW: rotations[rBase + 3],
                scaleX: scales[pBase],
                scaleY: scales[pBase + 1],
                scaleZ: scales[pBase + 2])
            for node in nodes {
                let nodeId = ObjectIdentifier(node)
                guard let restLocal = restLocalTransformByNode[nodeId] else {
                    continue
                }
                node.simdTransform = simd_mul(restLocal, localDelta)
            }
        }
    }

    func moveFollow(cursorX: Int32, cursorY: Int32) {
        if positionMode != .follow {
            return
        }
        let desired = NSPoint(
            x: CGFloat(cursorX) - panel.frame.width * 0.52 + offsetX,
            y: CGFloat(cursorY) - panel.frame.height * 0.48 + offsetY)
        panel.setFrameOrigin(clampToDesktop(desired))
    }

    private func setFixedBottomLeftOrigin() {
        let desktop = resolveDesktopBounds()
        let desired = NSPoint(
            x: desktop.minX + offsetX,
            y: desktop.minY + offsetY)
        panel.setFrameOrigin(clampToDesktop(desired))
    }

    private func resolveDesktopBounds() -> NSRect {
        let screens = NSScreen.screens
        guard let first = screens.first else {
            return panel.frame
        }
        var union = first.frame
        for screen in screens.dropFirst() {
            union = union.union(screen.frame)
        }
        return union
    }

    private func clampToDesktop(_ origin: NSPoint) -> NSPoint {
        let desktop = resolveDesktopBounds()
        let width = panel.frame.width
        let height = panel.frame.height
        let minX = desktop.minX
        let minY = desktop.minY
        let maxX = max(minX, desktop.maxX - width)
        let maxY = max(minY, desktop.maxY - height)
        return NSPoint(
            x: min(max(origin.x, minX), maxX),
            y: min(max(origin.y, minY), maxY))
    }

    private func resizePanelCanvasIfNeeded(_ newSize: CGSize) {
        let snapped = CGSize(
            width: ceil(max(targetPetSizePx, newSize.width)),
            height: ceil(max(targetPetSizePx, newSize.height)))
        if abs(snapped.width - panelCanvasSize.width) < 0.5,
           abs(snapped.height - panelCanvasSize.height) < 0.5 {
            return
        }

        let oldFrame = panel.frame
        let newOrigin: NSPoint
        if positionMode == .fixedBottomLeft {
            newOrigin = oldFrame.origin
        } else {
            newOrigin = NSPoint(
                x: oldFrame.midX - snapped.width * 0.5,
                y: oldFrame.midY - snapped.height * 0.5)
        }

        panelCanvasSize = snapped
        updateCameraForCurrentCanvas()
        let bounds = NSRect(origin: .zero, size: snapped)
        panel.setFrame(NSRect(origin: clampToDesktop(newOrigin), size: snapped), display: true)
        contentView.frame = bounds
        sceneView.frame = bounds
        companionView.frame = bounds
    }

    private func updateCameraForCurrentCanvas() {
        let widthRatio = panelCanvasSize.width / max(1.0, targetPetSizePx)
        let heightRatio = panelCanvasSize.height / max(1.0, targetPetSizePx)
        let ratio = max(1.0, max(widthRatio, heightRatio))
        sceneCameraNode.position = SCNVector3(
            x: baseCameraPosition.x,
            y: baseCameraPosition.y,
            z: baseCameraPosition.z * ratio)
    }

    private func startModelFrameLoop() {
        if modelFrameTimer != nil {
            return
        }
        let timer = Timer(
            timeInterval: 1.0 / 120.0,
            target: self,
            selector: #selector(onModelFrame(_:)),
            userInfo: nil,
            repeats: true)
        RunLoop.main.add(timer, forMode: .common)
        modelFrameTimer = timer
    }

    private func stopModelFrameLoop() {
        modelFrameTimer?.invalidate()
        modelFrameTimer = nil
    }

    @objc
    private func onModelFrame(_ timer: Timer) {
        _ = timer
        guard modelLoaded, let modelNode else {
            return
        }
        updateLoadedModel(
            node: modelNode,
            actionCode: currentActionCode,
            actionIntensity: currentActionIntensity,
            headTintAmount: currentHeadTintAmount)
        if pendingInitialPanelFit {
            fitCanvasToModel()
            pendingInitialPanelFit = false
        }
    }

    private func loadScene(url: URL) -> SCNScene? {
        return try? SCNScene(url: url, options: nil)
    }

    private func disableImportedAnimations(root: SCNNode) {
        root.removeAllAnimations()
        root.enumerateChildNodes { node, _ in
            node.removeAllAnimations()
        }
    }

    private func hydrateAnonymousSkeletonNamesIfNeeded(root: SCNNode, sourceURL: URL) {
        guard sourceURL.pathExtension.lowercased() == "usdz" else {
            return
        }
        guard let glbURL = canonicalGlbMetadataURL(for: sourceURL) else {
            return
        }
        guard let canonicalNames = parseJointDescendantNamesFromGlb(url: glbURL), !canonicalNames.isEmpty else {
            return
        }
        guard let skeletonContainer = findSkeletonContainer(in: root) else {
            return
        }

        var anonymousNodes: [SCNNode] = []
        collectAnonymousSkeletonNodes(from: skeletonContainer, into: &anonymousNodes)
        guard anonymousNodes.count == canonicalNames.count else {
            return
        }

        for (node, name) in zip(anonymousNodes, canonicalNames) {
            if node.name == nil || isAnonymousSkeletonNodeName(node.name) {
                node.name = name
            }
        }
    }

    private func canonicalGlbMetadataURL(for sourceURL: URL) -> URL? {
        let glbURL = sourceURL.deletingPathExtension().appendingPathExtension("glb")
        return FileManager.default.fileExists(atPath: glbURL.path) ? glbURL : nil
    }

    private func parseJointDescendantNamesFromGlb(url: URL) -> [String]? {
        guard let data = try? Data(contentsOf: url),
              let root = parseGlbRootJson(data: data),
              let nodesAny = root["nodes"] as? [Any],
              let skinsAny = root["skins"] as? [Any],
              let firstSkin = skinsAny.first as? [String: Any],
              let jointsAny = firstSkin["joints"] as? [Any],
              !jointsAny.isEmpty else {
            return nil
        }

        var nodes: [MfxGlbNodeMetadata] = []
        nodes.reserveCapacity(nodesAny.count)
        for nodeAny in nodesAny {
            guard let nodeDict = nodeAny as? [String: Any] else {
                nodes.append(MfxGlbNodeMetadata(name: nil, children: []))
                continue
            }
            let name = nodeDict["name"] as? String
            let children = (nodeDict["children"] as? [Any])?.compactMap { value -> Int? in
                if let number = value as? NSNumber {
                    return number.intValue
                }
                return nil
            } ?? []
            nodes.append(MfxGlbNodeMetadata(name: name, children: children))
        }

        let jointIds = Set(jointsAny.compactMap { value -> Int? in
            if let number = value as? NSNumber {
                return number.intValue
            }
            return nil
        })
        guard !jointIds.isEmpty else {
            return nil
        }

        let rootJointId: Int
        if let skeletonValue = firstSkin["skeleton"] as? NSNumber {
            rootJointId = skeletonValue.intValue
        } else if let firstJoint = jointsAny.first as? NSNumber {
            rootJointId = firstJoint.intValue
        } else {
            return nil
        }

        var orderedNames: [String] = []
        appendJointDescendantNames(
            nodeIndex: rootJointId,
            nodes: nodes,
            jointIds: jointIds,
            includeCurrent: false,
            output: &orderedNames)
        return orderedNames.isEmpty ? nil : orderedNames
    }

    private func parseGlbRootJson(data: Data) -> [String: Any]? {
        guard data.count >= 20 else {
            return nil
        }
        let magic = readUInt32LE(data: data, offset: 0)
        let version = readUInt32LE(data: data, offset: 4)
        let fileLength = Int(readUInt32LE(data: data, offset: 8))
        guard magic == 0x46546C67, version == 2, fileLength <= data.count else {
            return nil
        }

        let jsonChunkLength = Int(readUInt32LE(data: data, offset: 12))
        let jsonChunkType = readUInt32LE(data: data, offset: 16)
        guard jsonChunkType == 0x4E4F534A else {
            return nil
        }

        let jsonStart = 20
        let jsonEnd = jsonStart + jsonChunkLength
        guard jsonChunkLength > 0, jsonEnd <= data.count else {
            return nil
        }

        let jsonData = data.subdata(in: jsonStart..<jsonEnd)
        guard let rootAny = try? JSONSerialization.jsonObject(with: jsonData),
              let root = rootAny as? [String: Any] else {
            return nil
        }
        return root
    }

    private func appendJointDescendantNames(
        nodeIndex: Int,
        nodes: [MfxGlbNodeMetadata],
        jointIds: Set<Int>,
        includeCurrent: Bool,
        output: inout [String]
    ) {
        guard nodeIndex >= 0, nodeIndex < nodes.count else {
            return
        }
        let node = nodes[nodeIndex]
        if includeCurrent, jointIds.contains(nodeIndex), let name = node.name, !name.isEmpty {
            output.append(name)
        }
        for childIndex in node.children {
            appendJointDescendantNames(
                nodeIndex: childIndex,
                nodes: nodes,
                jointIds: jointIds,
                includeCurrent: true,
                output: &output)
        }
    }

    private func findSkeletonContainer(in root: SCNNode) -> SCNNode? {
        var match: SCNNode?
        root.enumerateChildNodes { node, stop in
            guard let name = node.name?.lowercased(), name == "skeleton" else {
                return
            }
            match = node
            stop.pointee = true
        }
        return match
    }

    private func collectAnonymousSkeletonNodes(from node: SCNNode, into output: inout [SCNNode]) {
        for child in node.childNodes {
            if isAnonymousSkeletonNodeName(child.name) {
                output.append(child)
            }
            collectAnonymousSkeletonNodes(from: child, into: &output)
        }
    }

    private func isAnonymousSkeletonNodeName(_ value: String?) -> Bool {
        guard let value else {
            return false
        }
        guard value.hasPrefix("n") else {
            return false
        }
        return Int(value.dropFirst()) != nil
    }

    private func readUInt32LE(data: Data, offset: Int) -> UInt32 {
        let b0 = UInt32(data[offset + 0])
        let b1 = UInt32(data[offset + 1]) << 8
        let b2 = UInt32(data[offset + 2]) << 16
        let b3 = UInt32(data[offset + 3]) << 24
        return b0 | b1 | b2 | b3
    }

    private func containsRenderableGeometry(root: SCNNode) -> Bool {
        var hasRenderable = false
        root.enumerateChildNodes { node, stop in
            guard let geometry = node.geometry else {
                return
            }
            if !geometry.sources(for: .vertex).isEmpty {
                hasRenderable = true
                stop.pointee = true
            }
        }
        return hasRenderable
    }

    private func rebuildBoneNodeIndex(root: SCNNode) {
        boneNodesByName.removeAll(keepingCapacity: true)
        restLocalTransformByNode.removeAll(keepingCapacity: true)
        actionTrackNodeCache.removeAll(keepingCapacity: true)
        root.enumerateChildNodes { node, _ in
            guard let key = Self.normalizedBoneName(node.name), !key.isEmpty else {
                return
            }
            self.boneNodesByName[key, default: []].append(node)
            self.restLocalTransformByNode[ObjectIdentifier(node)] = node.simdTransform
        }
    }

    private func resolveSemanticCoreBones() {
        semanticChestNodes = resolveSemanticNodes(
            exactCandidates: ["chest", "upperchest", "spine2", "spine.002", "spine", "torso", "body"],
            fallbackContains: ["chest", "spine", "torso", "body"],
            limit: 1)
        semanticHeadNodes = resolveSemanticNodes(
            exactCandidates: ["head", "face", "kao", "neck", "頭", "顔"],
            fallbackContains: ["head", "face", "kao", "neck", "頭", "顔"],
            limit: 1)
    }

    private func resolveSemanticNodes(
        exactCandidates: [String],
        fallbackContains: [String],
        limit: Int
    ) -> [SCNNode] {
        var nodes: [SCNNode] = []
        for name in exactCandidates {
            guard let key = Self.normalizedBoneName(name), let matched = boneNodesByName[key], !matched.isEmpty else {
                continue
            }
            nodes.append(contentsOf: matched)
            if nodes.count >= limit {
                break
            }
        }
        if nodes.isEmpty {
            for (key, matched) in boneNodesByName {
                if fallbackContains.contains(where: { key.contains($0.lowercased()) }) {
                    nodes.append(contentsOf: matched)
                }
            }
        }
        if nodes.isEmpty {
            return []
        }
        let sorted = nodes.sorted { lhs, rhs in
            lhs.worldPosition.y > rhs.worldPosition.y
        }
        return Array(sorted.prefix(max(0, limit)))
    }

    private func restoreNodesToRest(_ nodes: [SCNNode]) {
        for node in nodes {
            let nodeId = ObjectIdentifier(node)
            guard let rest = restLocalTransformByNode[nodeId] else {
                continue
            }
            node.simdTransform = rest
        }
    }

    private func restoreAllPoseBindingNodesToRest() {
        for nodes in poseBindingNodesByIndex {
            restoreNodesToRest(nodes)
        }
    }

    private func resolveActionTrackNodes(_ boneName: String) -> [SCNNode] {
        let key = boneName.lowercased()
        if let cached = actionTrackNodeCache[key] {
            return cached
        }
        let resolved = resolveActionTrackNodesInternal(boneName, visited: Set([key]))
        actionTrackNodeCache[key] = resolved
        return resolved
    }

    private func resolveActionTrackNodesInternal(_ boneName: String, visited: Set<String>) -> [SCNNode] {
        guard let lower = Self.normalizedBoneName(boneName) else {
            return []
        }
        if let exact = boneNodesByName[lower], !exact.isEmpty {
            return [exact[0]]
        }

        if lower == "chest" {
            if !semanticChestNodes.isEmpty {
                return [semanticChestNodes[0]]
            }
            let fallback = resolveSemanticNodes(
                exactCandidates: ["chest", "upperchest", "spine2", "spine.002", "spine", "torso", "body"],
                fallbackContains: ["chest", "spine", "torso", "body"],
                limit: 1)
            return fallback
        }
        if lower == "head" {
            if !semanticHeadNodes.isEmpty {
                return [semanticHeadNodes[0]]
            }
            let fallback = resolveSemanticNodes(
                exactCandidates: ["head", "face", "kao", "neck", "頭", "顔"],
                fallbackContains: ["head", "face", "kao", "neck", "頭", "顔"],
                limit: 1)
            return fallback
        }
        if lower == "spine" {
            let fallback = resolveSemanticNodes(
                exactCandidates: ["spine", "spine1", "spine01", "hips", "pelvis", "root", "torso", "body"],
                fallbackContains: ["spine", "hips", "pelvis", "root", "torso", "body"],
                limit: 1)
            return fallback
        }

        var containsMatches: [SCNNode] = []
        for (candidateName, nodes) in boneNodesByName where candidateName.contains(lower) {
            containsMatches.append(contentsOf: nodes)
        }
        if !containsMatches.isEmpty {
            let sorted = containsMatches.sorted { lhs, rhs in
                lhs.worldPosition.y > rhs.worldPosition.y
            }
            return [sorted[0]]
        }

        if let aliases = actionBoneRemapLower[lower] {
            for alias in aliases {
                guard let aliasLower = Self.normalizedBoneName(alias), !visited.contains(aliasLower) else {
                    continue
                }
                var nextVisited = visited
                nextVisited.insert(aliasLower)
                let mapped = resolveActionTrackNodesInternal(alias, visited: nextVisited)
                if !mapped.isEmpty {
                    return mapped
                }
            }
        }

        return []
    }

    private func sampleActionTrack(_ track: MfxActionTrack, localTime: CGFloat) -> MfxActionTrackSample {
        let keyframes = track.keyframes
        if keyframes.isEmpty {
            return MfxActionTrackSample(rotation: nil, scale: nil)
        }
        if keyframes.count == 1 {
            return MfxActionTrackSample(rotation: keyframes[0].rotation, scale: keyframes[0].scale)
        }

        let first = keyframes[0]
        let last = keyframes[keyframes.count - 1]
        if localTime <= first.t {
            return MfxActionTrackSample(rotation: first.rotation, scale: first.scale)
        }
        if localTime >= last.t {
            return MfxActionTrackSample(rotation: last.rotation, scale: last.scale)
        }

        var left = 0
        var right = keyframes.count - 1
        while left <= right {
            let mid = (left + right) >> 1
            if keyframes[mid].t <= localTime {
                left = mid + 1
            } else {
                right = mid - 1
            }
        }

        let a = keyframes[max(0, right)]
        let b = keyframes[min(keyframes.count - 1, right + 1)]
        let span = max(0.000001, b.t - a.t)
        let alpha = Float(mfxClamp((localTime - a.t) / span, min: 0.0, max: 1.0))

        var sampledRotation: simd_quatf?
        if a.rotation != nil || b.rotation != nil {
            let qa = a.rotation ?? simd_quatf(angle: 0.0, axis: SIMD3<Float>(0.0, 1.0, 0.0))
            let qb = b.rotation ?? qa
            sampledRotation = simd_normalize(simd_slerp(qa, qb, alpha))
        }

        var sampledScale: SIMD3<Float>?
        if a.scale != nil || b.scale != nil {
            let sa = a.scale ?? SIMD3<Float>(1.0, 1.0, 1.0)
            let sb = b.scale ?? sa
            sampledScale = sa + (sb - sa) * alpha
        }

        return MfxActionTrackSample(rotation: sampledRotation, scale: sampledScale)
    }

    private func applyClickActionClipPose(localTime: CGFloat) -> Bool {
        guard let clip = clickActionClip else {
            return false
        }
        let t = mfxClamp(localTime, min: 0.0, max: clip.duration)
        var anyApplied = false

        for track in clip.tracks {
            let nodes = resolveActionTrackNodes(track.bone)
            if nodes.isEmpty {
                continue
            }
            let sample = sampleActionTrack(track, localTime: t)
            for node in nodes {
                let nodeId = ObjectIdentifier(node)
                guard let rest = restLocalTransformByNode[nodeId] else {
                    continue
                }
                let rotation = sample.rotation ?? simd_quatf(angle: 0.0, axis: SIMD3<Float>(0.0, 1.0, 0.0))
                let scale = sample.scale ?? SIMD3<Float>(1.0, 1.0, 1.0)
                let delta = composeLocalDelta(
                    positionX: 0.0,
                    positionY: 0.0,
                    positionZ: 0.0,
                    rotationX: rotation.vector.x,
                    rotationY: rotation.vector.y,
                    rotationZ: rotation.vector.z,
                    rotationW: rotation.vector.w,
                    scaleX: scale.x,
                    scaleY: scale.y,
                    scaleZ: scale.z)
                node.simdTransform = simd_mul(rest, delta)
                anyApplied = true
            }
        }

        return anyApplied
    }

    private func normalizeModelTransform() {
        guard let modelNode else {
            return
        }
        let (minV, maxV) = modelNode.boundingBox
        let sx = maxV.x - minV.x
        let sy = maxV.y - minV.y
        let sz = maxV.z - minV.z
        let fitX = sx * 1.14
        let fitY = sy * 1.24
        let fitZ = sz * 1.12
        let maxDim = max(0.001, max(fitX, max(fitY, fitZ)))
        if !sx.isFinite || !sy.isFinite || !sz.isFinite || maxDim < 0.001 {
            modelBaseScale = 1.0
            modelNode.scale = SCNVector3(x: modelBaseScale, y: modelBaseScale, z: modelBaseScale)
            modelNode.position = SCNVector3Zero
            modelBasePosition = modelNode.position
            modelNode.eulerAngles = SCNVector3(x: 0.0, y: modelFacingYaw, z: 0.0)
            return
        }

        modelBaseScale = min(10.0, max(0.02, 0.70 / maxDim))
        modelNode.scale = SCNVector3(x: modelBaseScale, y: modelBaseScale, z: modelBaseScale)
        let cx = (minV.x + maxV.x) * 0.5
        let cy = (minV.y + maxV.y) * 0.5
        let cz = (minV.z + maxV.z) * 0.5
        let verticalLift = sy * modelBaseScale * 0.08
        modelNode.position = SCNVector3(
            x: -(cx * modelBaseScale),
            y: -(cy * modelBaseScale) + verticalLift,
            z: -(cz * modelBaseScale))
        modelBasePosition = modelNode.position
        modelNode.eulerAngles = SCNVector3(x: 0.0, y: modelFacingYaw, z: 0.0)
    }

    private func fitCanvasToModel() {
        guard let modelNode,
              let measured = measuredRenderableBounds(for: modelNode) else {
            return
        }
        resizePanelCanvasIfNeeded(
            CGSize(
                width: measured.width * 1.5,
                height: measured.height * 3.0))
        recenterModelInCanvas(modelNode)
        ensureVerticalInsetsAfterFit(for: modelNode)
    }

    private func ensureVerticalInsetsAfterFit(for node: SCNNode) {
        let minimumTopInset: CGFloat = 18.0
        let minimumBottomInset: CGFloat = 14.0
        for _ in 0..<2 {
            guard let measured = measuredRenderableBounds(for: node) else {
                return
            }
            let desiredHeight = max(
                panelCanvasSize.height,
                measured.height + minimumTopInset + minimumBottomInset)
            if desiredHeight <= panelCanvasSize.height + 0.5 {
                return
            }
            resizePanelCanvasIfNeeded(
                CGSize(
                    width: panelCanvasSize.width,
                    height: desiredHeight))
            recenterModelInCanvas(node)
        }
    }

    private func recenterModelInCanvas(_ node: SCNNode) {
        guard let projected = measuredRenderableBounds(for: node) else {
            return
        }
        let deltaX = panelCanvasSize.width * 0.5 - projected.midX
        let deltaY = panelCanvasSize.height * 0.5 - projected.midY
        if abs(deltaX) < 0.5, abs(deltaY) < 0.5 {
            return
        }

        let anchor = sceneView.projectPoint(node.presentation.position)
        let shifted = SCNVector3(
            x: anchor.x + deltaX,
            y: anchor.y - deltaY,
            z: anchor.z)
        let worldAnchor = sceneView.unprojectPoint(anchor)
        let worldShifted = sceneView.unprojectPoint(shifted)
        node.position.x += worldShifted.x - worldAnchor.x
        node.position.y += worldShifted.y - worldAnchor.y
        node.position.z += worldShifted.z - worldAnchor.z
        modelBasePosition = node.position
    }

    private func measuredRenderableBounds(for root: SCNNode) -> CGRect? {
        if let snapshot = snapshotVisibleBounds() {
            return snapshot
        }
        return projectedRenderableBounds(for: root)
    }

    private func projectedRenderableBounds(for root: SCNNode) -> CGRect? {
        sceneView.layoutSubtreeIfNeeded()
        var minX = CGFloat.greatestFiniteMagnitude
        var minY = CGFloat.greatestFiniteMagnitude
        var maxX = -CGFloat.greatestFiniteMagnitude
        var maxY = -CGFloat.greatestFiniteMagnitude
        var found = false

        root.enumerateChildNodes { node, _ in
            guard let geometry = node.geometry, !geometry.sources(for: .vertex).isEmpty else {
                return
            }
            let (localMin, localMax) = geometry.boundingBox
            let corners = [
                SCNVector3(localMin.x, localMin.y, localMin.z),
                SCNVector3(localMin.x, localMin.y, localMax.z),
                SCNVector3(localMin.x, localMax.y, localMin.z),
                SCNVector3(localMin.x, localMax.y, localMax.z),
                SCNVector3(localMax.x, localMin.y, localMin.z),
                SCNVector3(localMax.x, localMin.y, localMax.z),
                SCNVector3(localMax.x, localMax.y, localMin.z),
                SCNVector3(localMax.x, localMax.y, localMax.z),
            ]
            for corner in corners {
                let world = node.presentation.convertPosition(corner, to: nil)
                let projected = sceneView.projectPoint(world)
                minX = min(minX, projected.x)
                minY = min(minY, projected.y)
                maxX = max(maxX, projected.x)
                maxY = max(maxY, projected.y)
            }
            found = true
        }

        guard found,
              minX.isFinite, minY.isFinite, maxX.isFinite, maxY.isFinite,
              maxX > minX, maxY > minY else {
            return nil
        }
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }

    private func snapshotVisibleBounds() -> CGRect? {
        sceneView.layoutSubtreeIfNeeded()
        let originalBackground = sceneView.backgroundColor
        let chroma = NSColor(
            calibratedRed: 1.0,
            green: 0.0,
            blue: 1.0,
            alpha: 1.0)
        sceneView.backgroundColor = chroma
        let image = sceneView.snapshot()
        sceneView.backgroundColor = originalBackground

        guard let tiffData = image.tiffRepresentation,
              let bitmap = NSBitmapImageRep(data: tiffData) else {
            return nil
        }

        let width = bitmap.pixelsWide
        let height = bitmap.pixelsHigh
        guard width > 0, height > 0 else {
            return nil
        }

        var minX = width
        var minY = height
        var maxX = -1
        var maxY = -1

        for y in 0..<height {
            for x in 0..<width {
                guard let rawColor = bitmap.colorAt(x: x, y: y),
                      let color = rawColor.usingColorSpace(.deviceRGB) else {
                    continue
                }
                if color.alphaComponent < 0.05 {
                    continue
                }
                let distance =
                    abs(color.redComponent - 1.0) +
                    abs(color.greenComponent - 0.0) +
                    abs(color.blueComponent - 1.0)
                if distance < 0.15 {
                    continue
                }
                if x < minX { minX = x }
                if y < minY { minY = y }
                if x > maxX { maxX = x }
                if y > maxY { maxY = y }
            }
        }

        guard maxX >= minX, maxY >= minY else {
            return nil
        }

        let scaleX = max(0.0001, image.size.width / CGFloat(width))
        let scaleY = max(0.0001, image.size.height / CGFloat(height))
        let bounds = CGRect(
            x: CGFloat(minX) * scaleX,
            y: CGFloat(minY) * scaleY,
            width: CGFloat(maxX - minX + 1) * scaleX,
            height: CGFloat(maxY - minY + 1) * scaleY)

        let widthCoverage = bounds.width / max(1.0, panelCanvasSize.width)
        let heightCoverage = bounds.height / max(1.0, panelCanvasSize.height)
        if widthCoverage > 0.97, heightCoverage > 0.97 {
            return nil
        }
        return bounds
    }

    private func updateLoadedModel(
        node: SCNNode,
        actionCode: Int32,
        actionIntensity: Float,
        headTintAmount: Float
    ) {
        let now = CACurrentMediaTime()
        let dt = CGFloat(max(0.0, min(0.05, now - modelLastTick)))
        modelLastTick = now
        modelBobTime += dt

        let resolvedIntensity = CGFloat(mfxClamp(CGFloat(actionIntensity), min: 0.0, max: 1.0))
        let oneShotDuration = mfxClamp(clickOneShotDuration, min: 0.05, max: 5.0)
        if clickOneShotActive {
            clickOneShotElapsed += dt
            if clickOneShotElapsed >= oneShotDuration {
                clickOneShotElapsed = oneShotDuration
                clickOneShotActive = false
            }
        }
        let clickTime = mfxClamp(clickOneShotElapsed, min: 0.0, max: oneShotDuration)
        let clickPeakTime: CGFloat = 0.055
        let effectiveActionCode = clickOneShotActive
            ? MfxMouseCompanionActionCode.clickReact.rawValue
            : actionCode

        if effectiveActionCode != lastModelActionCode {
            lastModelActionCode = effectiveActionCode
            runActionPulse(node: node, actionCode: effectiveActionCode, intensity: actionIntensity)
        }

        if clickOneShotActive {
            node.eulerAngles.x = 0.0
            node.eulerAngles.y = modelFacingYaw
            node.position = modelBasePosition
        } else {
            let dragLean = (effectiveActionCode == MfxMouseCompanionActionCode.drag.rawValue) ? (resolvedIntensity * 0.05) : 0.0
            let followLean = (effectiveActionCode == MfxMouseCompanionActionCode.follow.rawValue) ? (resolvedIntensity * 0.02) : 0.0
            let dragYaw = (effectiveActionCode == MfxMouseCompanionActionCode.drag.rawValue) ? (-0.20 * resolvedIntensity) : 0.0
            node.eulerAngles.x = -dragLean - followLean
            node.eulerAngles.y = modelFacingYaw + dragYaw

            let bobStrength = (0.010 + resolvedIntensity * 0.012)
            let sway = sin(Double(modelBobTime * 2.0)) * Double(bobStrength)
            node.position.x = modelBasePosition.x + CGFloat(sway)
            node.position.y = modelBasePosition.y
            node.position.z = modelBasePosition.z
        }
        node.scale.x = modelBaseScale
        node.scale.y = modelBaseScale
        node.scale.z = modelBaseScale

        restoreNodesToRest(semanticChestNodes)
        restoreNodesToRest(semanticHeadNodes)
        let clipApplied = clickOneShotActive && applyClickActionClipPose(localTime: clickTime)
        if !clipApplied && clickOneShotActive {
            let clickStrength = mfxClamp(max(0.72, resolvedIntensity), min: 0.0, max: 1.0)
            var chestScaleX: CGFloat = 1.0
            var chestScaleY: CGFloat = 1.0
            var chestScaleZ: CGFloat = 1.0
            if clickTime <= clickPeakTime {
                let a = mfxClamp(clickTime / clickPeakTime, min: 0.0, max: 1.0)
                let eased = a * a * (3.0 - 2.0 * a)
                chestScaleX = 1.0 + 0.06 * eased * clickStrength
                chestScaleY = 1.0 - 0.08 * eased * clickStrength
                chestScaleZ = 1.0 + 0.06 * eased * clickStrength
            } else {
                let b = mfxClamp((clickTime - clickPeakTime) / max(0.001, oneShotDuration - clickPeakTime), min: 0.0, max: 1.0)
                let eased = b * b * (3.0 - 2.0 * b)
                chestScaleX = (1.06 - 0.06 * eased) * clickStrength + (1.0 - clickStrength)
                chestScaleY = (0.92 + 0.08 * eased) * clickStrength + (1.0 - clickStrength)
                chestScaleZ = (1.06 - 0.06 * eased) * clickStrength + (1.0 - clickStrength)
            }
            for chest in semanticChestNodes {
                chest.scale.x *= chestScaleX
                chest.scale.y *= chestScaleY
                chest.scale.z *= chestScaleZ
            }
        }

        let tint = mfxClamp(CGFloat(headTintAmount), min: 0.0, max: 1.0)
        applyHeadTint(tintAmount: tint)
    }

    private func beginClickOneShot() {
        clickOneShotActive = true
        clickOneShotDuration = mfxClamp(clickActionClip?.duration ?? 0.30, min: 0.05, max: 5.0)
        clickOneShotElapsed = 0.0
        restoreAllPoseBindingNodesToRest()
    }

    private func runActionPulse(node: SCNNode, actionCode: Int32, intensity: Float) {
        if actionCode == MfxMouseCompanionActionCode.clickReact.rawValue {
            return
        }
        node.removeAction(forKey: "mfx_action_pulse")
        let amp = CGFloat(max(0.02, min(0.12, intensity * 0.16)))
        let upDuration = (actionCode == MfxMouseCompanionActionCode.clickReact.rawValue) ? 0.10 : 0.14
        let up = SCNAction.scale(to: modelBaseScale * (1.0 + amp), duration: upDuration)
        let down = SCNAction.scale(to: modelBaseScale, duration: 0.16)
        up.timingMode = .easeOut
        down.timingMode = .easeInEaseOut
        node.runAction(SCNAction.sequence([up, down]), forKey: "mfx_action_pulse")
    }

    private func rebuildHeadTintTargets(root: SCNNode) {
        headTintTargetNodes.removeAll(keepingCapacity: true)
        headTintBaseMultiplyByMaterial.removeAll(keepingCapacity: true)
        appliedHeadTintAmount = -1.0

        var tintableNodes: [SCNNode] = []
        root.enumerateChildNodes { node, _ in
            guard let geometry = node.geometry, !geometry.materials.isEmpty else {
                return
            }
            tintableNodes.append(node)
        }
        guard !tintableNodes.isEmpty else {
            return
        }

        var selected = tintableNodes.filter { node in
            guard let name = node.name?.lowercased() else {
                return false
            }
            return name.contains("head") ||
                name.contains("face") ||
                name.contains("kao") ||
                name.contains("頭") ||
                name.contains("顔")
        }

        if selected.isEmpty {
            let ys = tintableNodes.map(meshWorldCenterY)
            if let minY = ys.min(), let maxY = ys.max() {
                let thresholdY = minY + (maxY - minY) * 0.5
                selected = tintableNodes.filter { meshWorldCenterY($0) >= thresholdY }
            }
        }

        if selected.isEmpty, let topmost = tintableNodes.max(by: { meshWorldCenterY($0) < meshWorldCenterY($1) }) {
            selected = [topmost]
        }

        headTintTargetNodes = selected
        for node in headTintTargetNodes {
            guard let geometry = node.geometry else {
                continue
            }
            for material in geometry.materials {
                let materialId = ObjectIdentifier(material)
                if headTintBaseMultiplyByMaterial[materialId] == nil {
                    headTintBaseMultiplyByMaterial[materialId] = resolvedMaterialMultiplyColor(material) ?? NSColor.white
                }
            }
        }
    }

    private func applyHeadTint(tintAmount: CGFloat) {
        let tint = mfxClamp(tintAmount, min: 0.0, max: 1.0)
        if abs(tint - appliedHeadTintAmount) < 0.0001 {
            return
        }
        for child in headTintTargetNodes {
            guard let geometry = child.geometry else {
                continue
            }
            for material in geometry.materials {
                let materialId = ObjectIdentifier(material)
                let baseColor = headTintBaseMultiplyByMaterial[materialId] ?? NSColor.white
                material.multiply.contents = blendedColor(
                    from: baseColor,
                    to: headTintColor,
                    amount: tint)
            }
        }
        appliedHeadTintAmount = tint
    }

    private func meshWorldCenterY(_ node: SCNNode) -> CGFloat {
        guard let geometry = node.geometry else {
            return CGFloat(node.presentation.worldPosition.y)
        }
        let (localMin, localMax) = geometry.boundingBox
        let center = SCNVector3(
            (localMin.x + localMax.x) * 0.5,
            (localMin.y + localMax.y) * 0.5,
            (localMin.z + localMax.z) * 0.5)
        let world = node.presentation.convertPosition(center, to: nil)
        return CGFloat(world.y)
    }

    private func resolvedMaterialMultiplyColor(_ material: SCNMaterial) -> NSColor? {
        if let color = material.multiply.contents as? NSColor {
            return color.usingColorSpace(.deviceRGB) ?? color
        }
        return nil
    }

    private func blendedColor(from: NSColor, to: NSColor, amount: CGFloat) -> NSColor {
        let alpha = mfxClamp(amount, min: 0.0, max: 1.0)
        let lhs = from.usingColorSpace(.deviceRGB) ?? from
        let rhs = to.usingColorSpace(.deviceRGB) ?? to
        return NSColor(
            calibratedRed: lhs.redComponent + (rhs.redComponent - lhs.redComponent) * alpha,
            green: lhs.greenComponent + (rhs.greenComponent - lhs.greenComponent) * alpha,
            blue: lhs.blueComponent + (rhs.blueComponent - lhs.blueComponent) * alpha,
            alpha: lhs.alphaComponent + (rhs.alphaComponent - lhs.alphaComponent) * alpha)
    }

    private static func normalizedBoneName(_ value: String?) -> String? {
        guard let value else {
            return nil
        }
        let trimmed = value.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.isEmpty {
            return nil
        }
        return trimmed.lowercased()
    }

    private func composeLocalDelta(
        positionX: Float,
        positionY: Float,
        positionZ: Float,
        rotationX: Float,
        rotationY: Float,
        rotationZ: Float,
        rotationW: Float,
        scaleX: Float,
        scaleY: Float,
        scaleZ: Float
    ) -> simd_float4x4 {
        var rotation = simd_quatf(ix: rotationX, iy: rotationY, iz: rotationZ, r: rotationW)
        let rotationVector = rotation.vector
        if !rotationVector.x.isFinite ||
            !rotationVector.y.isFinite ||
            !rotationVector.z.isFinite ||
            !rotationVector.w.isFinite ||
            simd_length_squared(rotationVector) < 0.000001 {
            rotation = simd_quatf(angle: 0.0, axis: SIMD3<Float>(0.0, 1.0, 0.0))
        } else {
            rotation = simd_normalize(rotation)
        }

        var scale = SIMD3<Float>(
            scaleX.isFinite ? scaleX : 1.0,
            scaleY.isFinite ? scaleY : 1.0,
            scaleZ.isFinite ? scaleZ : 1.0)
        if scale.x == 0.0 { scale.x = 1.0 }
        if scale.y == 0.0 { scale.y = 1.0 }
        if scale.z == 0.0 { scale.z = 1.0 }

        let translation = SIMD3<Float>(
            positionX.isFinite ? positionX : 0.0,
            positionY.isFinite ? positionY : 0.0,
            positionZ.isFinite ? positionZ : 0.0)

        var scaleMatrix = matrix_identity_float4x4
        scaleMatrix.columns.0.x = scale.x
        scaleMatrix.columns.1.y = scale.y
        scaleMatrix.columns.2.z = scale.z

        var translationMatrix = matrix_identity_float4x4
        translationMatrix.columns.3 = SIMD4<Float>(translation.x, translation.y, translation.z, 1.0)
        return simd_mul(simd_mul(translationMatrix, simd_float4x4(rotation)), scaleMatrix)
    }
}

@MainActor
private func mfxCreateMouseCompanionPanelOnMainThread(_ sizePx: Int, _ offsetX: Int, _ offsetY: Int) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)
    let handle = MfxMouseCompanionPanelHandle(sizePx: sizePx, offsetX: offsetX, offsetY: offsetY)
    return Unmanaged.passRetained(handle).toOpaque()
}

@MainActor
private func mfxReleaseMouseCompanionPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxMouseCompanionPanelHandle>.fromOpaque(ptr).takeRetainedValue()
    handle.closeAndCleanup()
}

@MainActor
private func mfxWithMouseCompanionPanelHandle(
    _ panelHandleBits: UInt,
    _ body: (MfxMouseCompanionPanelHandle) -> Void
) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxMouseCompanionPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    body(handle)
}

@_cdecl("mfx_macos_mouse_companion_panel_create_v1")
public func mfx_macos_mouse_companion_panel_create_v1(
    _ sizePx: Int32,
    _ offsetX: Int32,
    _ offsetY: Int32
) -> UnsafeMutableRawPointer? {
    let resolvedSize = Int(sizePx)
    let resolvedOffsetX = Int(offsetX)
    let resolvedOffsetY = Int(offsetY)

    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateMouseCompanionPanelOnMainThread(resolvedSize, resolvedOffsetX, resolvedOffsetY))
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateMouseCompanionPanelOnMainThread(resolvedSize, resolvedOffsetX, resolvedOffsetY))
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_mouse_companion_panel_release_v1")
public func mfx_macos_mouse_companion_panel_release_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseMouseCompanionPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxReleaseMouseCompanionPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_mouse_companion_panel_show_v1")
public func mfx_macos_mouse_companion_panel_show_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                handle.show()
            }
        }
    }
}

@_cdecl("mfx_macos_mouse_companion_panel_hide_v1")
public func mfx_macos_mouse_companion_panel_hide_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                handle.hide()
            }
        }
    }
}

@_cdecl("mfx_macos_mouse_companion_panel_configure_v1")
public func mfx_macos_mouse_companion_panel_configure_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ positionModeCode: Int32,
    _ offsetX: Int32,
    _ offsetY: Int32
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                handle.configure(positionModeCode: positionModeCode, offsetX: offsetX, offsetY: offsetY)
            }
        }
    }
}

@_cdecl("mfx_macos_mouse_companion_panel_update_v1")
public func mfx_macos_mouse_companion_panel_update_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ actionCode: Int32,
    _ actionIntensity: Float,
    _ headTintAmount: Float
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                handle.update(actionCode: actionCode, actionIntensity: actionIntensity, headTintAmount: headTintAmount)
            }
        }
    }
}

@_cdecl("mfx_macos_mouse_companion_panel_move_follow_v1")
public func mfx_macos_mouse_companion_panel_move_follow_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                handle.moveFollow(cursorX: x, cursorY: y)
            }
        }
    }
}

@_cdecl("mfx_macos_mouse_companion_panel_load_model_v1")
public func mfx_macos_mouse_companion_panel_load_model_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ modelPathUtf8: UnsafePointer<CChar>?
) -> Bool {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return false
    }
    guard let modelPathUtf8, let path = String(validatingCString: modelPathUtf8), !path.isEmpty else {
        return false
    }

    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            var loaded = false
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                loaded = handle.loadModel(path: path)
            }
            return loaded
        }
    }

    var loaded = false
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                loaded = handle.loadModel(path: path)
            }
        }
    }
    return loaded
}

@_cdecl("mfx_macos_mouse_companion_panel_load_action_library_v1")
public func mfx_macos_mouse_companion_panel_load_action_library_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ actionLibraryPathUtf8: UnsafePointer<CChar>?
) -> Bool {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return false
    }
    guard let actionLibraryPathUtf8,
          let path = String(validatingCString: actionLibraryPathUtf8),
          !path.isEmpty else {
        return false
    }

    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            var loaded = false
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                loaded = handle.loadActionLibrary(path: path)
            }
            return loaded
        }
    }

    var loaded = false
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                loaded = handle.loadActionLibrary(path: path)
            }
        }
    }
    return loaded
}

@_cdecl("mfx_macos_mouse_companion_panel_configure_pose_binding_v1")
public func mfx_macos_mouse_companion_panel_configure_pose_binding_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ boneNames: UnsafePointer<UnsafePointer<CChar>?>?,
    _ boneCount: Int32
) -> Bool {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 || boneCount <= 0 {
        return false
    }
    var names: [String] = []
    names.reserveCapacity(Int(boneCount))
    if let boneNames {
        for i in 0..<Int(boneCount) {
            if let cName = boneNames[i] {
                names.append(String(cString: cName))
            }
        }
    }
    if names.isEmpty {
        return false
    }

    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            var configured = false
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                configured = handle.configurePoseBinding(names: names)
            }
            return configured
        }
    }

    var configured = false
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                configured = handle.configurePoseBinding(names: names)
            }
        }
    }
    return configured
}

@_cdecl("mfx_macos_mouse_companion_panel_apply_pose_v1")
public func mfx_macos_mouse_companion_panel_apply_pose_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ boneIndices: UnsafePointer<Int32>?,
    _ positions: UnsafePointer<Float>?,
    _ rotations: UnsafePointer<Float>?,
    _ scales: UnsafePointer<Float>?,
    _ poseCount: Int32
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 || poseCount <= 0 {
        return
    }
    guard let boneIndices, let positions, let rotations, let scales else {
        return
    }

    let count = Int(poseCount)
    let indicesArray = Array(UnsafeBufferPointer(start: boneIndices, count: count))
    let positionsArray = Array(UnsafeBufferPointer(start: positions, count: count * 3))
    let rotationsArray = Array(UnsafeBufferPointer(start: rotations, count: count * 4))
    let scalesArray = Array(UnsafeBufferPointer(start: scales, count: count * 3))

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxWithMouseCompanionPanelHandle(panelHandleBits) { handle in
                handle.applyPose(
                    indices: indicesArray,
                    positions: positionsArray,
                    rotations: rotationsArray,
                    scales: scalesArray)
            }
        }
    }
}
