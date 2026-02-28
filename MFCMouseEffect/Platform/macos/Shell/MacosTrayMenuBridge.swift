@preconcurrency import AppKit
@preconcurrency import Foundation

public typealias MfxTrayActionCallback = @convention(c) (UnsafeMutableRawPointer?) -> Void

private final class MfxTrayActionBridge: NSObject {
    private let callbackContextBits: UInt
    private let onOpenSettingsCallback: MfxTrayActionCallback?
    private let onExitCallback: MfxTrayActionCallback?

    init(
        callbackContextBits: UInt,
        onOpenSettingsCallback: MfxTrayActionCallback?,
        onExitCallback: MfxTrayActionCallback?
    ) {
        self.callbackContextBits = callbackContextBits
        self.onOpenSettingsCallback = onOpenSettingsCallback
        self.onExitCallback = onExitCallback
    }

    private var callbackContext: UnsafeMutableRawPointer? {
        return UnsafeMutableRawPointer(bitPattern: callbackContextBits)
    }

    @objc
    func onOpenSettings(_ sender: Any?) {
        _ = sender
        onOpenSettingsCallback?(callbackContext)
    }

    @objc
    func onExit(_ sender: Any?) {
        _ = sender
        onExitCallback?(callbackContext)
    }
}

private final class MfxTrayMenuHandle: NSObject {
    var statusItem: NSStatusItem?
    var menu: NSMenu?
    var actionBridge: MfxTrayActionBridge?

    func cleanup() {
        if let statusItem {
            NSStatusBar.system.removeStatusItem(statusItem)
            self.statusItem = nil
        }
        self.menu = nil
        self.actionBridge = nil
    }
}

private func mfxNormalizeTrayText(_ value: UnsafePointer<CChar>?, _ fallback: String) -> String {
    guard let value else {
        return fallback
    }
    let decoded = String(cString: value)
    if decoded.isEmpty {
        return fallback
    }
    return decoded
}

private func mfxIsTraySettingsAutoTriggerEnabled() -> Bool {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_SETTINGS_ACTION"] ?? ""
    if raw.isEmpty {
        return false
    }
    let normalized = raw.lowercased()
    return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on"
}

@MainActor
private func mfxCreateTrayMenuOnMainThread(
    settingsTitle: String,
    exitTitle: String,
    tooltip: String,
    callbackContextBits: UInt,
    onOpenSettings: MfxTrayActionCallback?,
    onExit: MfxTrayActionCallback?
) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)

    let statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    let actionBridge = MfxTrayActionBridge(
        callbackContextBits: callbackContextBits,
        onOpenSettingsCallback: onOpenSettings,
        onExitCallback: onExit
    )
    let menu = NSMenu(title: "MFCMouseEffect")

    let settingsItem = NSMenuItem(
        title: settingsTitle,
        action: #selector(MfxTrayActionBridge.onOpenSettings(_:)),
        keyEquivalent: ","
    )
    settingsItem.keyEquivalentModifierMask = [.command]
    settingsItem.target = actionBridge
    menu.addItem(settingsItem)

    menu.addItem(NSMenuItem.separator())

    let exitItem = NSMenuItem(
        title: exitTitle,
        action: #selector(MfxTrayActionBridge.onExit(_:)),
        keyEquivalent: "q"
    )
    exitItem.keyEquivalentModifierMask = [.command]
    exitItem.target = actionBridge
    menu.addItem(exitItem)

    statusItem.menu = menu
    if let button = statusItem.button {
        button.title = "MFX"
        button.toolTip = tooltip
    }

    let handle = MfxTrayMenuHandle()
    handle.statusItem = statusItem
    handle.menu = menu
    handle.actionBridge = actionBridge
    return Unmanaged.passRetained(handle).toOpaque()
}

private func mfxCreateTrayMenu(
    settingsTitle: String,
    exitTitle: String,
    tooltip: String,
    callbackContextBits: UInt,
    onOpenSettings: MfxTrayActionCallback?,
    onExit: MfxTrayActionCallback?
) -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let menuHandleBits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTrayMenuOnMainThread(
                    settingsTitle: settingsTitle,
                    exitTitle: exitTitle,
                    tooltip: tooltip,
                    callbackContextBits: callbackContextBits,
                    onOpenSettings: onOpenSettings,
                    onExit: onExit
                )
            )
        }
        if menuHandleBits == 0 {
            return nil
        }
        return UnsafeMutableRawPointer(bitPattern: menuHandleBits)
    }

    var menuHandleBits: UInt = 0
    DispatchQueue.main.sync {
        menuHandleBits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTrayMenuOnMainThread(
                    settingsTitle: settingsTitle,
                    exitTitle: exitTitle,
                    tooltip: tooltip,
                    callbackContextBits: callbackContextBits,
                    onOpenSettings: onOpenSettings,
                    onExit: onExit
                )
            )
        }
    }
    if menuHandleBits == 0 {
        return nil
    }
    return UnsafeMutableRawPointer(bitPattern: menuHandleBits)
}

@MainActor
private func mfxReleaseTrayMenuOnMainThread(_ menuHandleBits: UInt) {
    if menuHandleBits == 0 {
        return
    }
    guard let menuHandle = UnsafeMutableRawPointer(bitPattern: menuHandleBits) else {
        return
    }
    let handle = Unmanaged<MfxTrayMenuHandle>.fromOpaque(menuHandle).takeRetainedValue()
    handle.cleanup()
}

@_cdecl("mfx_macos_tray_menu_create_v1")
public func mfx_macos_tray_menu_create_v1(
    _ settingsTitleUtf8: UnsafePointer<CChar>?,
    _ exitTitleUtf8: UnsafePointer<CChar>?,
    _ tooltipUtf8: UnsafePointer<CChar>?,
    _ callbackContext: UnsafeMutableRawPointer?,
    _ onOpenSettings: MfxTrayActionCallback?,
    _ onExit: MfxTrayActionCallback?
) -> UnsafeMutableRawPointer? {
    return mfxCreateTrayMenu(
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onExit: onExit
    )
}

@_cdecl("mfx_macos_tray_menu_release_v1")
public func mfx_macos_tray_menu_release_v1(_ menuHandle: UnsafeMutableRawPointer?) {
    let menuHandleBits = UInt(bitPattern: menuHandle)
    if menuHandleBits == 0 {
        return
    }
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxReleaseTrayMenuOnMainThread(menuHandleBits)
        }
        return
    }
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxReleaseTrayMenuOnMainThread(menuHandleBits)
        }
    }
}

@_cdecl("mfx_macos_tray_menu_schedule_auto_open_settings_v1")
public func mfx_macos_tray_menu_schedule_auto_open_settings_v1(_ menuHandle: UnsafeMutableRawPointer?) {
    if !mfxIsTraySettingsAutoTriggerEnabled() {
        return
    }

    let menuHandleBits = UInt(bitPattern: menuHandle)
    if menuHandleBits == 0 {
        return
    }

    DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(120)) {
        guard let ptr = UnsafeMutableRawPointer(bitPattern: menuHandleBits) else {
            return
        }
        let handle = Unmanaged<MfxTrayMenuHandle>.fromOpaque(ptr).takeUnretainedValue()
        handle.actionBridge?.onOpenSettings(nil)
    }
}

@_cdecl("mfx_macos_tray_terminate_app_v1")
public func mfx_macos_tray_terminate_app_v1() {
    @MainActor
    func terminateOnMainThread() {
        NSApplication.shared.terminate(nil)
    }

    if Thread.isMainThread {
        MainActor.assumeIsolated {
            terminateOnMainThread()
        }
        return
    }
    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            terminateOnMainThread()
        }
    }
}
