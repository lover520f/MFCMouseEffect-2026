@preconcurrency import AppKit
@preconcurrency import Foundation

public typealias MfxTrayActionCallback = @convention(c) (UnsafeMutableRawPointer?) -> Void
public typealias MfxTrayThemeSelectCallback = @convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?) -> Void

@MainActor
private var mfxTrayIconCache: NSImage? = nil
@MainActor
private var mfxTrayIconCacheInitialized = false

private func mfxProjectLogoPathCandidates() -> [String] {
    var candidates: [String] = []
    let envIconPath = ProcessInfo.processInfo.environment["MFX_MACOS_APP_ICON_PATH"] ?? ""
    if !envIconPath.isEmpty {
        candidates.append(envIconPath)
    }

    if let bundleLogo = Bundle.main.url(forResource: "logo_elegant", withExtension: "png")?.path {
        candidates.append(bundleLogo)
    }

    let cwd = FileManager.default.currentDirectoryPath
    if !cwd.isEmpty {
        candidates.append("\(cwd)/res/logo_elegant.png")
        candidates.append("\(cwd)/MFCMouseEffect/res/logo_elegant.png")
    }

    if let exe = ProcessInfo.processInfo.arguments.first, !exe.isEmpty {
        let exeDir = URL(fileURLWithPath: exe).deletingLastPathComponent().path
        if !exeDir.isEmpty {
            candidates.append("\(exeDir)/res/logo_elegant.png")
            candidates.append("\(exeDir)/../res/logo_elegant.png")
            candidates.append("\(exeDir)/../MFCMouseEffect/res/logo_elegant.png")
        }
    }

    return candidates
}

@MainActor
private func mfxResolveTrayIconImage() -> NSImage? {
    if mfxTrayIconCacheInitialized {
        return mfxTrayIconCache
    }
    mfxTrayIconCacheInitialized = true

    for candidate in mfxProjectLogoPathCandidates() {
        if FileManager.default.fileExists(atPath: candidate), let image = NSImage(contentsOfFile: candidate) {
            image.size = NSSize(width: 16, height: 16)
            image.isTemplate = false
            mfxTrayIconCache = image
            return image
        }
    }

    if #available(macOS 11.0, *) {
        let symbol = NSImage(systemSymbolName: "sparkles", accessibilityDescription: "MFX")
        symbol?.isTemplate = true
        mfxTrayIconCache = symbol
        return symbol
    }

    return nil
}

private final class MfxTrayActionBridge: NSObject {
    private let callbackContextBits: UInt
    private let onOpenSettingsCallback: MfxTrayActionCallback?
    private let onExitCallback: MfxTrayActionCallback?
    private let onThemeSelectCallback: MfxTrayThemeSelectCallback?

    init(
        callbackContextBits: UInt,
        onOpenSettingsCallback: MfxTrayActionCallback?,
        onExitCallback: MfxTrayActionCallback?,
        onThemeSelectCallback: MfxTrayThemeSelectCallback?
    ) {
        self.callbackContextBits = callbackContextBits
        self.onOpenSettingsCallback = onOpenSettingsCallback
        self.onExitCallback = onExitCallback
        self.onThemeSelectCallback = onThemeSelectCallback
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

    @objc
    func onSelectTheme(_ sender: Any?) {
        guard
            let item = sender as? NSMenuItem,
            let themeValue = item.representedObject as? String,
            !themeValue.isEmpty
        else {
            return
        }
        themeValue.withCString { raw in
            onThemeSelectCallback?(callbackContext, raw)
        }
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

private func mfxReadUtf8StringArray(_ values: UnsafePointer<UnsafePointer<CChar>?>?, _ count: UInt32) -> [String] {
    guard let values, count > 0 else {
        return []
    }
    var out: [String] = []
    out.reserveCapacity(Int(count))
    for index in 0..<Int(count) {
        guard let ptr = values[index] else {
            out.append("")
            continue
        }
        out.append(String(cString: ptr))
    }
    return out
}

private func mfxIsTraySettingsAutoTriggerEnabled() -> Bool {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_SETTINGS_ACTION"] ?? ""
    if raw.isEmpty {
        return false
    }
    let normalized = raw.lowercased()
    return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on"
}

private func mfxReadTrayAutoTriggerThemeValue() -> String {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_THEME_VALUE"] ?? ""
    if raw.isEmpty {
        return ""
    }
    return raw.trimmingCharacters(in: .whitespacesAndNewlines)
}

private func mfxFindThemeMenuItem(_ menu: NSMenu?, value: String) -> NSMenuItem? {
    guard let menu else {
        return nil
    }
    for item in menu.items {
        if let representedValue = item.representedObject as? String, representedValue == value {
            return item
        }
        if let nested = mfxFindThemeMenuItem(item.submenu, value: value) {
            return nested
        }
    }
    return nil
}

@MainActor
private func mfxCreateTrayMenuOnMainThread(
    themeTitle: String,
    settingsTitle: String,
    exitTitle: String,
    tooltip: String,
    themeValues: [String],
    themeLabels: [String],
    selectedThemeValue: String,
    callbackContextBits: UInt,
    onOpenSettings: MfxTrayActionCallback?,
    onExit: MfxTrayActionCallback?,
    onThemeSelect: MfxTrayThemeSelectCallback?
) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)

    let statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    let actionBridge = MfxTrayActionBridge(
        callbackContextBits: callbackContextBits,
        onOpenSettingsCallback: onOpenSettings,
        onExitCallback: onExit,
        onThemeSelectCallback: onThemeSelect
    )
    let menu = NSMenu(title: "MFCMouseEffect")

    if !themeValues.isEmpty {
        let themeMenu = NSMenu(title: themeTitle)
        let selectedTheme = selectedThemeValue
        let maxCount = min(themeValues.count, themeLabels.count)
        for index in 0..<maxCount {
            let value = themeValues[index]
            if value.isEmpty {
                continue
            }
            let label = themeLabels[index].isEmpty ? value : themeLabels[index]
            let item = NSMenuItem(
                title: label,
                action: #selector(MfxTrayActionBridge.onSelectTheme(_:)),
                keyEquivalent: ""
            )
            item.target = actionBridge
            item.representedObject = value
            if value == selectedTheme {
                item.state = .on
            }
            themeMenu.addItem(item)
        }
        if themeMenu.items.count > 0 {
            let rootItem = NSMenuItem(title: themeTitle, action: nil, keyEquivalent: "")
            rootItem.submenu = themeMenu
            menu.addItem(rootItem)
            menu.addItem(NSMenuItem.separator())
        }
    }

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
        if let trayImage = mfxResolveTrayIconImage() {
            button.image = trayImage
            button.imagePosition = .imageOnly
            button.title = ""
        } else {
            button.title = "MFX"
        }
        button.toolTip = tooltip
    }

    let handle = MfxTrayMenuHandle()
    handle.statusItem = statusItem
    handle.menu = menu
    handle.actionBridge = actionBridge
    return Unmanaged.passRetained(handle).toOpaque()
}

private func mfxCreateTrayMenu(
    themeTitle: String,
    settingsTitle: String,
    exitTitle: String,
    tooltip: String,
    themeValues: [String],
    themeLabels: [String],
    selectedThemeValue: String,
    callbackContextBits: UInt,
    onOpenSettings: MfxTrayActionCallback?,
    onExit: MfxTrayActionCallback?,
    onThemeSelect: MfxTrayThemeSelectCallback?
) -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let menuHandleBits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTrayMenuOnMainThread(
                    themeTitle: themeTitle,
                    settingsTitle: settingsTitle,
                    exitTitle: exitTitle,
                    tooltip: tooltip,
                    themeValues: themeValues,
                    themeLabels: themeLabels,
                    selectedThemeValue: selectedThemeValue,
                    callbackContextBits: callbackContextBits,
                    onOpenSettings: onOpenSettings,
                    onExit: onExit,
                    onThemeSelect: onThemeSelect
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
                    themeTitle: themeTitle,
                    settingsTitle: settingsTitle,
                    exitTitle: exitTitle,
                    tooltip: tooltip,
                    themeValues: themeValues,
                    themeLabels: themeLabels,
                    selectedThemeValue: selectedThemeValue,
                    callbackContextBits: callbackContextBits,
                    onOpenSettings: onOpenSettings,
                    onExit: onExit,
                    onThemeSelect: onThemeSelect
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
        themeTitle: "Theme",
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        themeValues: [],
        themeLabels: [],
        selectedThemeValue: "",
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onExit: onExit,
        onThemeSelect: nil
    )
}

@_cdecl("mfx_macos_tray_menu_create_v2")
public func mfx_macos_tray_menu_create_v2(
    _ themeTitleUtf8: UnsafePointer<CChar>?,
    _ settingsTitleUtf8: UnsafePointer<CChar>?,
    _ exitTitleUtf8: UnsafePointer<CChar>?,
    _ tooltipUtf8: UnsafePointer<CChar>?,
    _ themeValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ themeLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ themeCount: UInt32,
    _ selectedThemeValueUtf8: UnsafePointer<CChar>?,
    _ callbackContext: UnsafeMutableRawPointer?,
    _ onOpenSettings: MfxTrayActionCallback?,
    _ onExit: MfxTrayActionCallback?,
    _ onThemeSelect: MfxTrayThemeSelectCallback?
) -> UnsafeMutableRawPointer? {
    let values = mfxReadUtf8StringArray(themeValuesUtf8, themeCount)
    let labels = mfxReadUtf8StringArray(themeLabelsUtf8, themeCount)
    return mfxCreateTrayMenu(
        themeTitle: mfxNormalizeTrayText(themeTitleUtf8, "Theme"),
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        themeValues: values,
        themeLabels: labels,
        selectedThemeValue: mfxNormalizeTrayText(selectedThemeValueUtf8, ""),
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onExit: onExit,
        onThemeSelect: onThemeSelect
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
    let autoThemeValue = mfxReadTrayAutoTriggerThemeValue()
    if !autoThemeValue.isEmpty {
        let menuHandleBits = UInt(bitPattern: menuHandle)
        if menuHandleBits != 0 {
            DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(80)) {
                guard let ptr = UnsafeMutableRawPointer(bitPattern: menuHandleBits) else {
                    return
                }
                let handle = Unmanaged<MfxTrayMenuHandle>.fromOpaque(ptr).takeUnretainedValue()
                guard let item = mfxFindThemeMenuItem(handle.menu, value: autoThemeValue) else {
                    return
                }
                handle.actionBridge?.onSelectTheme(item)
            }
        }
    }

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
