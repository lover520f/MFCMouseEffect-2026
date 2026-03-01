@preconcurrency import AppKit
@preconcurrency import QuartzCore
@preconcurrency import Foundation

private func mfxClamp(_ value: CGFloat, min minValue: CGFloat, max maxValue: CGFloat) -> CGFloat {
    if value < minValue {
        return minValue
    }
    if value > maxValue {
        return maxValue
    }
    return value
}

private func mfxColorFromArgb(_ argb: UInt32, _ alphaScale: CGFloat) -> NSColor {
    let baseAlpha = CGFloat(Double((argb >> 24) & 0xFF) / 255.0)
    let alpha = mfxClamp(baseAlpha * alphaScale, min: 0.0, max: 1.0)
    let red = CGFloat(Double((argb >> 16) & 0xFF) / 255.0)
    let green = CGFloat(Double((argb >> 8) & 0xFF) / 255.0)
    let blue = CGFloat(Double(argb & 0xFF) / 255.0)
    return NSColor(calibratedRed: red, green: green, blue: blue, alpha: alpha)
}

private func mfxCreateTintedImage(_ image: NSImage, tintColor: NSColor) -> NSImage? {
    let size = image.size
    guard size.width > 0.0, size.height > 0.0 else {
        return nil
    }
    let rect = NSRect(origin: .zero, size: size)
    let tinted = NSImage(size: size)
    tinted.lockFocus()
    defer { tinted.unlockFocus() }
    image.draw(in: rect, from: .zero, operation: .sourceOver, fraction: 1.0)
    tintColor.set()
    rect.fill(using: .sourceAtop)
    return tinted
}

@MainActor
private func mfxCreateWasmImageOverlayOnMainThread(
    frameX: Double,
    frameY: Double,
    frameSize: Double,
    imagePathUtf8: String,
    tintArgb: UInt32,
    applyTint: Bool,
    alphaScale: Double,
    durationSec: Double,
    rotationRad: Double,
    motionDx: Double,
    motionDy: Double
) -> UnsafeMutableRawPointer? {
    let size = max(1.0, CGFloat(frameSize))
    let frame = NSRect(x: frameX, y: frameY, width: size, height: size)
    let window = NSWindow(
        contentRect: frame,
        styleMask: .borderless,
        backing: .buffered,
        defer: false
    )
    window.isOpaque = false
    window.backgroundColor = .clear
    window.hasShadow = false
    window.ignoresMouseEvents = true
    window.level = .statusBar
    window.collectionBehavior = [.canJoinAllSpaces, .transient]

    let content = NSView(frame: NSRect(x: 0.0, y: 0.0, width: size, height: size))
    content.wantsLayer = true
    window.contentView = content

    let alphaScaleClamped = mfxClamp(CGFloat(alphaScale), min: 0.0, max: 1.0)
    var renderedImage = false
    if !imagePathUtf8.isEmpty, let image = NSImage(contentsOfFile: imagePathUtf8) {
        let imageInset = mfxClamp(size * 0.16, min: 8.0, max: 60.0)
        let resolvedImage: NSImage
        if applyTint {
            let tintColor = mfxColorFromArgb(tintArgb, alphaScaleClamped)
            resolvedImage = mfxCreateTintedImage(image, tintColor: tintColor) ?? image
        } else {
            resolvedImage = image
        }
        let imageView = NSImageView(
            frame: NSRect(
                x: imageInset,
                y: imageInset,
                width: size - imageInset * 2.0,
                height: size - imageInset * 2.0
            )
        )
        imageView.image = resolvedImage
        imageView.imageScaling = .scaleProportionallyUpOrDown
        imageView.alphaValue = alphaScaleClamped
        content.addSubview(imageView)
        renderedImage = true
    }

    let ringInset = mfxClamp(size * 0.13, min: 8.0, max: 36.0)
    let ring = CAShapeLayer()
    ring.frame = content.bounds
    let ringRect = CGRect(
        x: ringInset,
        y: ringInset,
        width: size - ringInset * 2.0,
        height: size - ringInset * 2.0
    )
    ring.path = CGPath(ellipseIn: ringRect, transform: nil)
    let fillAlphaScale = renderedImage ? 0.08 * alphaScaleClamped : 0.22 * alphaScaleClamped
    let strokeAlphaScale = 0.95 * alphaScaleClamped
    ring.fillColor = mfxColorFromArgb(tintArgb, fillAlphaScale).cgColor
    ring.strokeColor = mfxColorFromArgb(tintArgb, strokeAlphaScale).cgColor
    ring.lineWidth = mfxClamp(size * 0.022, min: 1.5, max: 5.0)
    ring.opacity = 0.98
    content.layer?.addSublayer(ring)

    let duration = max(0.05, durationSec)
    let animScale = CABasicAnimation(keyPath: "transform.scale")
    animScale.fromValue = 0.15
    animScale.toValue = 1.0
    animScale.duration = duration
    animScale.timingFunction = CAMediaTimingFunction(name: .easeOut)

    let fade = CABasicAnimation(keyPath: "opacity")
    fade.fromValue = 0.98
    fade.toValue = 0.0
    fade.duration = duration
    fade.timingFunction = CAMediaTimingFunction(name: .easeIn)

    let group = CAAnimationGroup()
    group.animations = [animScale, fade]
    group.duration = duration
    group.fillMode = .forwards
    group.isRemovedOnCompletion = false
    ring.add(group, forKey: "mfx_wasm_image_overlay")

    if abs(rotationRad) > 0.001 {
        let rotate = CABasicAnimation(keyPath: "transform.rotation.z")
        rotate.fromValue = 0.0
        rotate.toValue = rotationRad
        rotate.duration = duration
        rotate.timingFunction = CAMediaTimingFunction(name: .easeOut)
        rotate.fillMode = .forwards
        rotate.isRemovedOnCompletion = false
        content.layer?.add(rotate, forKey: "mfx_wasm_image_rotate")
    }

    if abs(motionDx) > 0.01 || abs(motionDy) > 0.01 {
        var endFrame = frame
        endFrame.origin.x += CGFloat(motionDx)
        endFrame.origin.y += CGFloat(motionDy)
        NSAnimationContext.runAnimationGroup({ context in
            context.duration = duration
            context.timingFunction = CAMediaTimingFunction(name: .easeOut)
            window.animator().setFrame(endFrame, display: false)
        }, completionHandler: nil)
    }

    return Unmanaged.passRetained(window).toOpaque()
}

@MainActor
private func mfxShowWasmImageOverlayOnMainThread(_ windowHandleBits: UInt) {
    guard windowHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: windowHandleBits) else {
        return
    }
    let window = Unmanaged<NSWindow>.fromOpaque(ptr).takeUnretainedValue()
    window.orderFrontRegardless()
}

@_cdecl("mfx_macos_wasm_image_overlay_create_v1")
public func mfx_macos_wasm_image_overlay_create_v1(
    _ frameX: Double,
    _ frameY: Double,
    _ frameSize: Double,
    _ imagePathUtf8: UnsafePointer<CChar>?,
    _ tintArgb: UInt32,
    _ applyTint: Int32,
    _ alphaScale: Double,
    _ durationSec: Double,
    _ rotationRad: Double,
    _ motionDx: Double,
    _ motionDy: Double
) -> UnsafeMutableRawPointer? {
    let imagePath = imagePathUtf8.map(String.init(cString:)) ?? ""
    let shouldApplyTint = applyTint != 0
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateWasmImageOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    imagePathUtf8: imagePath,
                    tintArgb: tintArgb,
                    applyTint: shouldApplyTint,
                    alphaScale: alphaScale,
                    durationSec: durationSec,
                    rotationRad: rotationRad,
                    motionDx: motionDx,
                    motionDy: motionDy
                )
            )
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateWasmImageOverlayOnMainThread(
                    frameX: frameX,
                    frameY: frameY,
                    frameSize: frameSize,
                    imagePathUtf8: imagePath,
                    tintArgb: tintArgb,
                    applyTint: shouldApplyTint,
                    alphaScale: alphaScale,
                    durationSec: durationSec,
                    rotationRad: rotationRad,
                    motionDx: motionDx,
                    motionDy: motionDy
                )
            )
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_wasm_image_overlay_show_v1")
public func mfx_macos_wasm_image_overlay_show_v1(_ windowHandle: UnsafeMutableRawPointer?) {
    let windowHandleBits = UInt(bitPattern: windowHandle)
    if windowHandleBits == 0 {
        return
    }
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxShowWasmImageOverlayOnMainThread(windowHandleBits)
        }
        return
    }
    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxShowWasmImageOverlayOnMainThread(windowHandleBits)
        }
    }
}
