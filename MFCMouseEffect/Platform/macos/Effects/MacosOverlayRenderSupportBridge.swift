@preconcurrency import AppKit
@preconcurrency import Foundation

private func mfxClampScale(_ value: CGFloat) -> CGFloat {
    if value < 1.0 {
        return 1.0
    }
    if value > 4.0 {
        return 4.0
    }
    return value
}

@MainActor
private func mfxResolveTargetScreenOnMainThread(_ x: Int32, _ y: Int32) -> NSScreen? {
    let screens = NSScreen.screens
    if screens.isEmpty {
        return nil
    }

    let point = NSPoint(x: CGFloat(x), y: CGFloat(y))
    for screen in screens where screen.frame.contains(point) {
        return screen
    }

    return NSScreen.main ?? screens.first
}

@MainActor
private func mfxCreateOverlayWindowOnMainThread(
    _ x: Double,
    _ y: Double,
    _ width: Double,
    _ height: Double
) -> UnsafeMutableRawPointer? {
    let frame = NSRect(x: x, y: y, width: width, height: height)
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
    return Unmanaged.passRetained(window).toOpaque()
}

@MainActor
private func mfxResolveContentScaleOnMainThread(_ x: Int32, _ y: Int32) -> Double {
    guard let screen = mfxResolveTargetScreenOnMainThread(x, y) else {
        return 1.0
    }
    return Double(mfxClampScale(screen.backingScaleFactor))
}

@MainActor
private func mfxReleaseOverlayWindowOnMainThread(_ windowHandleBits: UInt) {
    guard windowHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: windowHandleBits) else {
        return
    }
    let window = Unmanaged<NSWindow>.fromOpaque(ptr).takeRetainedValue()
    window.orderOut(nil)
}

@MainActor
private func mfxApplyContentScaleOnMainThread(_ contentHandleBits: UInt, _ x: Int32, _ y: Int32) {
    guard contentHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: contentHandleBits) else {
        return
    }
    let content = Unmanaged<NSView>.fromOpaque(ptr).takeUnretainedValue()
    content.wantsLayer = true
    guard let root = content.layer else {
        return
    }
    let scale = CGFloat(mfxResolveContentScaleOnMainThread(x, y))
    root.contentsScale = scale
    root.sublayers?.forEach { layer in
        layer.contentsScale = scale
    }
}

@_cdecl("mfx_macos_overlay_create_window_v1")
public func mfx_macos_overlay_create_window_v1(
    _ x: Double,
    _ y: Double,
    _ width: Double,
    _ height: Double
) -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateOverlayWindowOnMainThread(x, y, width, height))
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateOverlayWindowOnMainThread(x, y, width, height))
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@MainActor
private func mfxShowOverlayWindowOnMainThread(_ windowHandleBits: UInt) {
    guard windowHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: windowHandleBits) else {
        return
    }
    let window = Unmanaged<NSWindow>.fromOpaque(ptr).takeUnretainedValue()
    window.orderFrontRegardless()
}

@_cdecl("mfx_macos_overlay_release_window_v1")
public func mfx_macos_overlay_release_window_v1(_ windowHandle: UnsafeMutableRawPointer?) {
    let windowHandleBits = UInt(bitPattern: windowHandle)
    if windowHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseOverlayWindowOnMainThread(windowHandleBits)
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxReleaseOverlayWindowOnMainThread(windowHandleBits)
        }
    }
}

@_cdecl("mfx_macos_overlay_show_window_v1")
public func mfx_macos_overlay_show_window_v1(_ windowHandle: UnsafeMutableRawPointer?) {
    let windowHandleBits = UInt(bitPattern: windowHandle)
    if windowHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxShowOverlayWindowOnMainThread(windowHandleBits)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxShowOverlayWindowOnMainThread(windowHandleBits)
        }
    }
}

@_cdecl("mfx_macos_overlay_resolve_screen_frame_v1")
public func mfx_macos_overlay_resolve_screen_frame_v1(
    _ x: Int32,
    _ y: Int32,
    _ outX: UnsafeMutablePointer<Double>?,
    _ outY: UnsafeMutablePointer<Double>?,
    _ outWidth: UnsafeMutablePointer<Double>?,
    _ outHeight: UnsafeMutablePointer<Double>?
) -> Int32 {
    var frame = NSRect.zero
    var ok = false
    let resolveOnMain = {
        let screens = NSScreen.screens
        if screens.isEmpty {
            return
        }

        let point = NSPoint(x: CGFloat(x), y: CGFloat(y))
        for screen in screens where screen.frame.contains(point) {
            let candidate = screen.frame
            if candidate.width > 0.0 && candidate.height > 0.0 {
                frame = candidate
                ok = true
            }
            return
        }

        if let fallback = NSScreen.main ?? screens.first {
            let candidate = fallback.frame
            if candidate.width > 0.0 && candidate.height > 0.0 {
                frame = candidate
                ok = true
            }
        }
    }

    if Thread.isMainThread {
        resolveOnMain()
    } else {
        DispatchQueue.main.sync {
            resolveOnMain()
        }
    }

    if !ok {
        return 0
    }
    outX?.pointee = Double(frame.origin.x)
    outY?.pointee = Double(frame.origin.y)
    outWidth?.pointee = Double(frame.size.width)
    outHeight?.pointee = Double(frame.size.height)
    return 1
}

@_cdecl("mfx_macos_overlay_resolve_content_scale_v1")
public func mfx_macos_overlay_resolve_content_scale_v1(_ x: Int32, _ y: Int32) -> Double {
    if Thread.isMainThread {
        return MainActor.assumeIsolated {
            mfxResolveContentScaleOnMainThread(x, y)
        }
    }

    var value = 1.0
    DispatchQueue.main.sync {
        value = MainActor.assumeIsolated {
            mfxResolveContentScaleOnMainThread(x, y)
        }
    }
    return value
}

@_cdecl("mfx_macos_overlay_apply_content_scale_v1")
public func mfx_macos_overlay_apply_content_scale_v1(
    _ contentHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32
) {
    let contentHandleBits = UInt(bitPattern: contentHandle)
    if contentHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxApplyContentScaleOnMainThread(contentHandleBits, x, y)
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxApplyContentScaleOnMainThread(contentHandleBits, x, y)
        }
    }
}
