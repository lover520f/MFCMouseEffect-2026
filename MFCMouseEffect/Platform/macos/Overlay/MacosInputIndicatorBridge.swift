@preconcurrency import AppKit
@preconcurrency import Foundation

@MainActor
private final class MfxInputIndicatorPanelHandle: NSObject {
    private let panel: NSPanel
    private let label: NSTextField

    init(sizePx: Int) {
        let side = CGFloat(max(1, sizePx))
        panel = NSPanel(
            contentRect: NSRect(x: 0, y: 0, width: side, height: side),
            styleMask: .borderless,
            backing: .buffered,
            defer: false
        )

        panel.isOpaque = false
        panel.backgroundColor = .clear
        panel.hasShadow = false
        panel.ignoresMouseEvents = true
        panel.hidesOnDeactivate = false
        panel.level = .statusBar
        panel.collectionBehavior = [.canJoinAllSpaces, .transient]

        let content = NSView(frame: NSRect(x: 0, y: 0, width: side, height: side))
        content.wantsLayer = true
        content.layer?.backgroundColor = NSColor(calibratedWhite: 0, alpha: 0.58).cgColor
        content.layer?.cornerRadius = 14.0
        content.layer?.borderWidth = 1.0
        content.layer?.borderColor = NSColor(calibratedWhite: 1, alpha: 0.25).cgColor
        panel.contentView = content

        label = NSTextField(frame: NSRect(x: 0, y: (side - 32.0) * 0.5, width: side, height: 32))
        label.isEditable = false
        label.isBezeled = false
        label.drawsBackground = false
        label.isSelectable = false
        label.alignment = .center
        label.textColor = NSColor(calibratedRed: 0.84, green: 0.95, blue: 1.0, alpha: 1.0)
        label.font = NSFont.monospacedSystemFont(ofSize: 15, weight: .semibold)
        label.stringValue = ""
        content.addSubview(label)
    }

    func present(x: Int, y: Int, sizePx: Int, text: String) {
        let side = CGFloat(max(1, sizePx))
        panel.setFrame(NSRect(x: CGFloat(x), y: CGFloat(y), width: side, height: side), display: false)
        panel.contentView?.layer?.cornerRadius = side * 0.22
        label.frame = NSRect(x: 0, y: (side - 32.0) * 0.5, width: side, height: 32)
        label.stringValue = text
        panel.alphaValue = 1.0
        panel.orderFrontRegardless()
    }

    func hide() {
        panel.orderOut(nil)
    }

    func closeAndCleanup() {
        hide()
        label.removeFromSuperview()
        panel.close()
    }
}

@MainActor
private func mfxCreateInputIndicatorPanelOnMainThread(_ sizePx: Int) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)
    let handle = MfxInputIndicatorPanelHandle(sizePx: sizePx)
    return Unmanaged.passRetained(handle).toOpaque()
}

@MainActor
private func mfxReleaseInputIndicatorPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxInputIndicatorPanelHandle>.fromOpaque(ptr).takeRetainedValue()
    handle.closeAndCleanup()
}

@MainActor
private func mfxHideInputIndicatorPanelOnMainThread(_ panelHandleBits: UInt) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxInputIndicatorPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.hide()
}

@MainActor
private func mfxPresentInputIndicatorPanelOnMainThread(
    _ panelHandleBits: UInt,
    _ x: Int,
    _ y: Int,
    _ sizePx: Int,
    _ text: String
) {
    guard panelHandleBits != 0 else {
        return
    }
    guard let ptr = UnsafeMutableRawPointer(bitPattern: panelHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxInputIndicatorPanelHandle>.fromOpaque(ptr).takeUnretainedValue()
    handle.present(x: x, y: y, sizePx: sizePx, text: text)
}

@_cdecl("mfx_macos_input_indicator_panel_create_v1")
public func mfx_macos_input_indicator_panel_create_v1(_ sizePx: Int32) -> UnsafeMutableRawPointer? {
    let size = Int(sizePx)
    if Thread.isMainThread {
        let bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateInputIndicatorPanelOnMainThread(size))
        }
        return UnsafeMutableRawPointer(bitPattern: bits)
    }

    var bits: UInt = 0
    DispatchQueue.main.sync {
        bits = MainActor.assumeIsolated {
            UInt(bitPattern: mfxCreateInputIndicatorPanelOnMainThread(size))
        }
    }
    return UnsafeMutableRawPointer(bitPattern: bits)
}

@_cdecl("mfx_macos_input_indicator_panel_release_v1")
public func mfx_macos_input_indicator_panel_release_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseInputIndicatorPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxReleaseInputIndicatorPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_input_indicator_panel_hide_v1")
public func mfx_macos_input_indicator_panel_hide_v1(_ panelHandle: UnsafeMutableRawPointer?) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxHideInputIndicatorPanelOnMainThread(panelHandleBits)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxHideInputIndicatorPanelOnMainThread(panelHandleBits)
        }
    }
}

@_cdecl("mfx_macos_input_indicator_panel_present_v1")
public func mfx_macos_input_indicator_panel_present_v1(
    _ panelHandle: UnsafeMutableRawPointer?,
    _ x: Int32,
    _ y: Int32,
    _ sizePx: Int32,
    _ textUtf8: UnsafePointer<CChar>?
) {
    let panelHandleBits = UInt(bitPattern: panelHandle)
    if panelHandleBits == 0 {
        return
    }

    let text = textUtf8.map(String.init(cString:)) ?? ""
    let xInt = Int(x)
    let yInt = Int(y)
    let sizeInt = Int(sizePx)

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxPresentInputIndicatorPanelOnMainThread(panelHandleBits, xInt, yInt, sizeInt, text)
        }
        return
    }

    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxPresentInputIndicatorPanelOnMainThread(panelHandleBits, xInt, yInt, sizeInt, text)
        }
    }
}

