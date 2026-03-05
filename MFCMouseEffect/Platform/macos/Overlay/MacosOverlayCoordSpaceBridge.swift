@preconcurrency import AppKit
@preconcurrency import ApplicationServices
@preconcurrency import Foundation

private let mfxNSScreenNumberKey = NSDeviceDescriptionKey("NSScreenNumber")

private struct MfxDisplayTransform {
    let quartzBounds: CGRect
    let cocoaFrame: CGRect
}

private final class MfxDisplayTransformCache: @unchecked Sendable {
    private let lock = NSLock()
    private var transforms: [MfxDisplayTransform] = []

    func store(_ newTransforms: [MfxDisplayTransform]) {
        lock.lock()
        transforms = newTransforms
        lock.unlock()
    }

    func snapshot() -> [MfxDisplayTransform] {
        lock.lock()
        let snapshot = transforms
        lock.unlock()
        return snapshot
    }
}

private let mfxDisplayTransformCache = MfxDisplayTransformCache()

private func mfxClampToInt32(_ value: Double) -> Int32 {
    let rounded = value.rounded()
    if rounded < Double(Int32.min) {
        return Int32.min
    }
    if rounded > Double(Int32.max) {
        return Int32.max
    }
    return Int32(rounded)
}

private func mfxBuildDisplayTransformsOnMainThread() -> [MfxDisplayTransform] {
    let screens = NSScreen.screens
    if screens.isEmpty {
        return []
    }

    var transforms: [MfxDisplayTransform] = []
    transforms.reserveCapacity(screens.count)

    for screen in screens {
        guard
            let screenNumber = screen.deviceDescription[mfxNSScreenNumberKey] as? NSNumber
        else {
            continue
        }
        let displayId = CGDirectDisplayID(screenNumber.uint32Value)
        let bounds = CGDisplayBounds(displayId)
        let frame = screen.frame
        if bounds.width <= 0 || bounds.height <= 0 || frame.width <= 0 || frame.height <= 0 {
            continue
        }
        transforms.append(MfxDisplayTransform(quartzBounds: bounds, cocoaFrame: frame))
    }

    return transforms
}

private func mfxDistanceSquaredToRect(_ point: CGPoint, _ rect: CGRect) -> CGFloat {
    let dx: CGFloat
    if point.x < rect.minX {
        dx = rect.minX - point.x
    } else if point.x > rect.maxX {
        dx = point.x - rect.maxX
    } else {
        dx = 0
    }

    let dy: CGFloat
    if point.y < rect.minY {
        dy = rect.minY - point.y
    } else if point.y > rect.maxY {
        dy = point.y - rect.maxY
    } else {
        dy = 0
    }

    return dx * dx + dy * dy
}

private func mfxTryConvertUsingTransforms(
    _ transforms: [MfxDisplayTransform],
    _ inX: Int32,
    _ inY: Int32
) -> (Bool, Int32, Int32) {
    if transforms.isEmpty {
        return (false, inX, inY)
    }

    let quartzPoint = CGPoint(x: CGFloat(inX), y: CGFloat(inY))
    var selected = transforms.first
    var bestDistance = CGFloat.greatestFiniteMagnitude
    for transform in transforms {
        if transform.quartzBounds.contains(quartzPoint) {
            selected = transform
            bestDistance = 0
            break
        }

        let distance = mfxDistanceSquaredToRect(quartzPoint, transform.quartzBounds)
        if distance < bestDistance {
            bestDistance = distance
            selected = transform
        }
    }

    guard let transform = selected else {
        return (false, inX, inY)
    }

    let scaleX = transform.cocoaFrame.width / transform.quartzBounds.width
    let scaleY = transform.cocoaFrame.height / transform.quartzBounds.height
    let localX = (CGFloat(inX) - transform.quartzBounds.origin.x) * scaleX
    let localY = (CGFloat(inY) - transform.quartzBounds.origin.y) * scaleY

    let cocoaX = transform.cocoaFrame.origin.x + localX
    let cocoaY = transform.cocoaFrame.origin.y + transform.cocoaFrame.height - localY
    return (true, mfxClampToInt32(cocoaX), mfxClampToInt32(cocoaY))
}

private func mfxTryConvertWithDesktopBoundsFallback(
    _ inX: Int32,
    _ inY: Int32
) -> (Bool, Int32, Int32) {
    var displayCount: UInt32 = 0
    guard CGGetActiveDisplayList(0, nil, &displayCount) == .success, displayCount > 0 else {
        return (false, inX, inY)
    }

    var displayIds = Array(repeating: CGDirectDisplayID(), count: Int(displayCount))
    guard CGGetActiveDisplayList(displayCount, &displayIds, &displayCount) == .success else {
        return (false, inX, inY)
    }

    var desktopBounds = CGRect.null
    for displayId in displayIds.prefix(Int(displayCount)) {
        desktopBounds = desktopBounds.union(CGDisplayBounds(displayId))
    }
    if desktopBounds.isNull || desktopBounds.width <= 0 || desktopBounds.height <= 0 {
        return (false, inX, inY)
    }

    let cocoaX = CGFloat(inX)
    let cocoaY = desktopBounds.origin.y + desktopBounds.height - CGFloat(inY)
    return (true, mfxClampToInt32(cocoaX), mfxClampToInt32(cocoaY))
}

@MainActor
private func mfxConvertQuartzToCocoaOnMainThread(_ inX: Int32, _ inY: Int32) -> (Bool, Int32, Int32) {
    let transforms = mfxBuildDisplayTransformsOnMainThread()
    mfxDisplayTransformCache.store(transforms)
    let converted = mfxTryConvertUsingTransforms(transforms, inX, inY)
    if converted.0 {
        return converted
    }
    return mfxTryConvertWithDesktopBoundsFallback(inX, inY)
}

@_cdecl("mfx_macos_overlay_quartz_to_cocoa_v1")
public func mfx_macos_overlay_quartz_to_cocoa_v1(
    _ inX: Int32,
    _ inY: Int32,
    _ outX: UnsafeMutablePointer<Int32>?,
    _ outY: UnsafeMutablePointer<Int32>?
) -> Int32 {
    guard let outX, let outY else {
        return 0
    }

    let result: (Bool, Int32, Int32)
    if Thread.isMainThread {
        result = MainActor.assumeIsolated {
            mfxConvertQuartzToCocoaOnMainThread(inX, inY)
        }
    } else {
        DispatchQueue.main.async {
            MainActor.assumeIsolated {
                mfxDisplayTransformCache.store(mfxBuildDisplayTransformsOnMainThread())
            }
        }

        let cached = mfxTryConvertUsingTransforms(
            mfxDisplayTransformCache.snapshot(),
            inX,
            inY
        )
        if cached.0 {
            result = cached
        } else {
            result = mfxTryConvertWithDesktopBoundsFallback(inX, inY)
        }
    }

    if !result.0 {
        let fallback = mfxTryConvertWithDesktopBoundsFallback(inX, inY)
        if !fallback.0 {
            return 0
        }
        outX.pointee = fallback.1
        outY.pointee = fallback.2
        return 1
    }

    outX.pointee = result.1
    outY.pointee = result.2
    return 1
}
