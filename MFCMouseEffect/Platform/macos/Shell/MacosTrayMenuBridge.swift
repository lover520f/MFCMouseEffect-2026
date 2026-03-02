@preconcurrency import AppKit
@preconcurrency import Foundation

public typealias MfxTrayActionCallback = @convention(c) (UnsafeMutableRawPointer?) -> Void
public typealias MfxTrayThemeSelectCallback = @convention(c) (UnsafeMutableRawPointer?, UnsafePointer<CChar>?) -> Void
public typealias MfxTrayEffectSelectCallback = @convention(c) (
    UnsafeMutableRawPointer?,
    UnsafePointer<CChar>?,
    UnsafePointer<CChar>?
) -> Void

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

private final class MfxTrayEffectSelection: NSObject {
    let category: String
    let value: String

    init(category: String, value: String) {
        self.category = category
        self.value = value
    }
}

private final class MfxTrayActionBridge: NSObject {
    private let callbackContextBits: UInt
    private let onOpenSettingsCallback: MfxTrayActionCallback?
    private let onReloadConfigCallback: MfxTrayActionCallback?
    private let onExitCallback: MfxTrayActionCallback?
    private let onStarProjectCallback: MfxTrayActionCallback?
    private let onThemeSelectCallback: MfxTrayThemeSelectCallback?
    private let onEffectSelectCallback: MfxTrayEffectSelectCallback?

    init(
        callbackContextBits: UInt,
        onOpenSettingsCallback: MfxTrayActionCallback?,
        onReloadConfigCallback: MfxTrayActionCallback?,
        onExitCallback: MfxTrayActionCallback?,
        onStarProjectCallback: MfxTrayActionCallback?,
        onThemeSelectCallback: MfxTrayThemeSelectCallback?,
        onEffectSelectCallback: MfxTrayEffectSelectCallback?
    ) {
        self.callbackContextBits = callbackContextBits
        self.onOpenSettingsCallback = onOpenSettingsCallback
        self.onReloadConfigCallback = onReloadConfigCallback
        self.onExitCallback = onExitCallback
        self.onStarProjectCallback = onStarProjectCallback
        self.onThemeSelectCallback = onThemeSelectCallback
        self.onEffectSelectCallback = onEffectSelectCallback
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
    func onReloadConfig(_ sender: Any?) {
        _ = sender
        onReloadConfigCallback?(callbackContext)
    }

    @objc
    func onExit(_ sender: Any?) {
        _ = sender
        onExitCallback?(callbackContext)
    }

    @objc
    func onOpenStarProject(_ sender: Any?) {
        _ = sender
        onStarProjectCallback?(callbackContext)
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

    @objc
    func onSelectEffect(_ sender: Any?) {
        guard
            let item = sender as? NSMenuItem,
            let payload = item.representedObject as? MfxTrayEffectSelection,
            !payload.category.isEmpty,
            !payload.value.isEmpty
        else {
            return
        }
        payload.category.withCString { categoryRaw in
            payload.value.withCString { valueRaw in
                onEffectSelectCallback?(callbackContext, categoryRaw, valueRaw)
            }
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

private func mfxReadTrayAutoTriggerEffectCategory() -> String {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_EFFECT_CATEGORY"] ?? ""
    if raw.isEmpty {
        return ""
    }
    return raw.trimmingCharacters(in: .whitespacesAndNewlines)
}

private func mfxReadTrayAutoTriggerEffectValue() -> String {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_EFFECT_VALUE"] ?? ""
    if raw.isEmpty {
        return ""
    }
    return raw.trimmingCharacters(in: .whitespacesAndNewlines)
}

private func mfxIsTrayReloadAutoTriggerEnabled() -> Bool {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_RELOAD_ACTION"] ?? ""
    if raw.isEmpty {
        return false
    }
    let normalized = raw.lowercased()
    return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on"
}

private func mfxIsTrayStarAutoTriggerEnabled() -> Bool {
    let raw = ProcessInfo.processInfo.environment["MFX_TEST_TRAY_AUTO_TRIGGER_STAR_ACTION"] ?? ""
    if raw.isEmpty {
        return false
    }
    let normalized = raw.lowercased()
    return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on"
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

private func mfxFindEffectMenuItem(_ menu: NSMenu?, category: String, value: String) -> NSMenuItem? {
    guard let menu else {
        return nil
    }
    for item in menu.items {
        if let payload = item.representedObject as? MfxTrayEffectSelection,
           payload.category == category,
           payload.value == value {
            return item
        }
        if let nested = mfxFindEffectMenuItem(item.submenu, category: category, value: value) {
            return nested
        }
    }
    return nil
}

private struct MfxTrayEffectSectionSpec {
    let category: String
    let title: String
    let values: [String]
    let labels: [String]
    let selectedValue: String
}

private func mfxAddEffectSection(
    _ menu: NSMenu,
    _ actionBridge: MfxTrayActionBridge,
    _ section: MfxTrayEffectSectionSpec
) -> Bool {
    if section.category.isEmpty || section.title.isEmpty || section.values.isEmpty {
        return false
    }
    let submenu = NSMenu(title: section.title)
    let maxCount = min(section.values.count, section.labels.count)
    for index in 0..<maxCount {
        let value = section.values[index].trimmingCharacters(in: .whitespacesAndNewlines)
        if value.isEmpty {
            continue
        }
        let label = section.labels[index].isEmpty ? value : section.labels[index]
        let item = NSMenuItem(
            title: label,
            action: #selector(MfxTrayActionBridge.onSelectEffect(_:)),
            keyEquivalent: ""
        )
        item.target = actionBridge
        item.representedObject = MfxTrayEffectSelection(category: section.category, value: value)
        if value == section.selectedValue {
            item.state = .on
        }
        submenu.addItem(item)
    }
    if submenu.items.isEmpty {
        return false
    }
    let root = NSMenuItem(title: section.title, action: nil, keyEquivalent: "")
    root.submenu = submenu
    menu.addItem(root)
    return true
}

private func mfxAddMenuSeparatorIfNeeded(_ menu: NSMenu) {
    guard !menu.items.isEmpty else {
        return
    }
    if menu.items.last?.isSeparatorItem == true {
        return
    }
    menu.addItem(NSMenuItem.separator())
}

@MainActor
private func mfxCreateTrayMenuOnMainThread(
    themeTitle: String,
    starProjectTitle: String,
    settingsTitle: String,
    reloadConfigTitle: String,
    exitTitle: String,
    tooltip: String,
    themeValues: [String],
    themeLabels: [String],
    selectedThemeValue: String,
    effectSections: [MfxTrayEffectSectionSpec],
    callbackContextBits: UInt,
    onOpenSettings: MfxTrayActionCallback?,
    onReloadConfig: MfxTrayActionCallback?,
    onExit: MfxTrayActionCallback?,
    onStarProject: MfxTrayActionCallback?,
    onThemeSelect: MfxTrayThemeSelectCallback?,
    onEffectSelect: MfxTrayEffectSelectCallback?
) -> UnsafeMutableRawPointer? {
    let app = NSApplication.shared
    app.setActivationPolicy(.accessory)

    let statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    let actionBridge = MfxTrayActionBridge(
        callbackContextBits: callbackContextBits,
        onOpenSettingsCallback: onOpenSettings,
        onReloadConfigCallback: onReloadConfig,
        onExitCallback: onExit,
        onStarProjectCallback: onStarProject,
        onThemeSelectCallback: onThemeSelect,
        onEffectSelectCallback: onEffectSelect
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
            mfxAddMenuSeparatorIfNeeded(menu)
        }
    }

    var hasEffectSection = false
    for section in effectSections {
        if mfxAddEffectSection(menu, actionBridge, section) {
            hasEffectSection = true
        }
    }
    if hasEffectSection {
        mfxAddMenuSeparatorIfNeeded(menu)
    }

    if !starProjectTitle.isEmpty {
        let starItem = NSMenuItem(
            title: starProjectTitle,
            action: #selector(MfxTrayActionBridge.onOpenStarProject(_:)),
            keyEquivalent: ""
        )
        starItem.target = actionBridge
        menu.addItem(starItem)
    }

    if !reloadConfigTitle.isEmpty || !settingsTitle.isEmpty {
        mfxAddMenuSeparatorIfNeeded(menu)
    }

    let settingsItem = NSMenuItem(
        title: settingsTitle,
        action: #selector(MfxTrayActionBridge.onOpenSettings(_:)),
        keyEquivalent: ","
    )
    settingsItem.keyEquivalentModifierMask = [.command]
    settingsItem.target = actionBridge
    menu.addItem(settingsItem)

    if !reloadConfigTitle.isEmpty {
        let reloadItem = NSMenuItem(
            title: reloadConfigTitle,
            action: #selector(MfxTrayActionBridge.onReloadConfig(_:)),
            keyEquivalent: "r"
        )
        reloadItem.keyEquivalentModifierMask = [.command]
        reloadItem.target = actionBridge
        menu.addItem(reloadItem)
    }

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
    starProjectTitle: String,
    settingsTitle: String,
    reloadConfigTitle: String,
    exitTitle: String,
    tooltip: String,
    themeValues: [String],
    themeLabels: [String],
    selectedThemeValue: String,
    effectSections: [MfxTrayEffectSectionSpec],
    callbackContextBits: UInt,
    onOpenSettings: MfxTrayActionCallback?,
    onReloadConfig: MfxTrayActionCallback?,
    onExit: MfxTrayActionCallback?,
    onStarProject: MfxTrayActionCallback?,
    onThemeSelect: MfxTrayThemeSelectCallback?,
    onEffectSelect: MfxTrayEffectSelectCallback?
) -> UnsafeMutableRawPointer? {
    if Thread.isMainThread {
        let menuHandleBits = MainActor.assumeIsolated {
            UInt(
                bitPattern: mfxCreateTrayMenuOnMainThread(
                    themeTitle: themeTitle,
                    starProjectTitle: starProjectTitle,
                    settingsTitle: settingsTitle,
                    reloadConfigTitle: reloadConfigTitle,
                    exitTitle: exitTitle,
                    tooltip: tooltip,
                    themeValues: themeValues,
                    themeLabels: themeLabels,
                    selectedThemeValue: selectedThemeValue,
                    effectSections: effectSections,
                    callbackContextBits: callbackContextBits,
                    onOpenSettings: onOpenSettings,
                    onReloadConfig: onReloadConfig,
                    onExit: onExit,
                    onStarProject: onStarProject,
                    onThemeSelect: onThemeSelect,
                    onEffectSelect: onEffectSelect
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
                    starProjectTitle: starProjectTitle,
                    settingsTitle: settingsTitle,
                    reloadConfigTitle: reloadConfigTitle,
                    exitTitle: exitTitle,
                    tooltip: tooltip,
                    themeValues: themeValues,
                    themeLabels: themeLabels,
                    selectedThemeValue: selectedThemeValue,
                    effectSections: effectSections,
                    callbackContextBits: callbackContextBits,
                    onOpenSettings: onOpenSettings,
                    onReloadConfig: onReloadConfig,
                    onExit: onExit,
                    onStarProject: onStarProject,
                    onThemeSelect: onThemeSelect,
                    onEffectSelect: onEffectSelect
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
        starProjectTitle: "",
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        reloadConfigTitle: "",
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        themeValues: [],
        themeLabels: [],
        selectedThemeValue: "",
        effectSections: [],
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onReloadConfig: nil,
        onExit: onExit,
        onStarProject: nil,
        onThemeSelect: nil,
        onEffectSelect: nil
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
        starProjectTitle: "",
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        reloadConfigTitle: "",
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        themeValues: values,
        themeLabels: labels,
        selectedThemeValue: mfxNormalizeTrayText(selectedThemeValueUtf8, ""),
        effectSections: [],
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onReloadConfig: nil,
        onExit: onExit,
        onStarProject: nil,
        onThemeSelect: onThemeSelect,
        onEffectSelect: nil
    )
}

@_cdecl("mfx_macos_tray_menu_create_v3")
public func mfx_macos_tray_menu_create_v3(
    _ themeTitleUtf8: UnsafePointer<CChar>?,
    _ settingsTitleUtf8: UnsafePointer<CChar>?,
    _ exitTitleUtf8: UnsafePointer<CChar>?,
    _ tooltipUtf8: UnsafePointer<CChar>?,
    _ themeValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ themeLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ themeCount: UInt32,
    _ selectedThemeValueUtf8: UnsafePointer<CChar>?,
    _ clickTitleUtf8: UnsafePointer<CChar>?,
    _ clickValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ clickLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ clickCount: UInt32,
    _ selectedClickValueUtf8: UnsafePointer<CChar>?,
    _ trailTitleUtf8: UnsafePointer<CChar>?,
    _ trailValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ trailLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ trailCount: UInt32,
    _ selectedTrailValueUtf8: UnsafePointer<CChar>?,
    _ scrollTitleUtf8: UnsafePointer<CChar>?,
    _ scrollValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ scrollLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ scrollCount: UInt32,
    _ selectedScrollValueUtf8: UnsafePointer<CChar>?,
    _ holdTitleUtf8: UnsafePointer<CChar>?,
    _ holdValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ holdLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ holdCount: UInt32,
    _ selectedHoldValueUtf8: UnsafePointer<CChar>?,
    _ hoverTitleUtf8: UnsafePointer<CChar>?,
    _ hoverValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ hoverLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ hoverCount: UInt32,
    _ selectedHoverValueUtf8: UnsafePointer<CChar>?,
    _ callbackContext: UnsafeMutableRawPointer?,
    _ onOpenSettings: MfxTrayActionCallback?,
    _ onExit: MfxTrayActionCallback?,
    _ onThemeSelect: MfxTrayThemeSelectCallback?,
    _ onEffectSelect: MfxTrayEffectSelectCallback?
) -> UnsafeMutableRawPointer? {
    let themeValues = mfxReadUtf8StringArray(themeValuesUtf8, themeCount)
    let themeLabels = mfxReadUtf8StringArray(themeLabelsUtf8, themeCount)
    let sections: [MfxTrayEffectSectionSpec] = [
        MfxTrayEffectSectionSpec(
            category: "click",
            title: mfxNormalizeTrayText(clickTitleUtf8, "Click Effects"),
            values: mfxReadUtf8StringArray(clickValuesUtf8, clickCount),
            labels: mfxReadUtf8StringArray(clickLabelsUtf8, clickCount),
            selectedValue: mfxNormalizeTrayText(selectedClickValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "trail",
            title: mfxNormalizeTrayText(trailTitleUtf8, "Trail Effects"),
            values: mfxReadUtf8StringArray(trailValuesUtf8, trailCount),
            labels: mfxReadUtf8StringArray(trailLabelsUtf8, trailCount),
            selectedValue: mfxNormalizeTrayText(selectedTrailValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "scroll",
            title: mfxNormalizeTrayText(scrollTitleUtf8, "Scroll Effects"),
            values: mfxReadUtf8StringArray(scrollValuesUtf8, scrollCount),
            labels: mfxReadUtf8StringArray(scrollLabelsUtf8, scrollCount),
            selectedValue: mfxNormalizeTrayText(selectedScrollValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "hold",
            title: mfxNormalizeTrayText(holdTitleUtf8, "Hold Effects"),
            values: mfxReadUtf8StringArray(holdValuesUtf8, holdCount),
            labels: mfxReadUtf8StringArray(holdLabelsUtf8, holdCount),
            selectedValue: mfxNormalizeTrayText(selectedHoldValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "hover",
            title: mfxNormalizeTrayText(hoverTitleUtf8, "Hover Effects"),
            values: mfxReadUtf8StringArray(hoverValuesUtf8, hoverCount),
            labels: mfxReadUtf8StringArray(hoverLabelsUtf8, hoverCount),
            selectedValue: mfxNormalizeTrayText(selectedHoverValueUtf8, "")
        ),
    ]

    return mfxCreateTrayMenu(
        themeTitle: mfxNormalizeTrayText(themeTitleUtf8, "Theme"),
        starProjectTitle: "",
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        reloadConfigTitle: "",
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        themeValues: themeValues,
        themeLabels: themeLabels,
        selectedThemeValue: mfxNormalizeTrayText(selectedThemeValueUtf8, ""),
        effectSections: sections,
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onReloadConfig: nil,
        onExit: onExit,
        onStarProject: nil,
        onThemeSelect: onThemeSelect,
        onEffectSelect: onEffectSelect
    )
}

@_cdecl("mfx_macos_tray_menu_create_v4")
public func mfx_macos_tray_menu_create_v4(
    _ themeTitleUtf8: UnsafePointer<CChar>?,
    _ starProjectTitleUtf8: UnsafePointer<CChar>?,
    _ settingsTitleUtf8: UnsafePointer<CChar>?,
    _ reloadConfigTitleUtf8: UnsafePointer<CChar>?,
    _ exitTitleUtf8: UnsafePointer<CChar>?,
    _ tooltipUtf8: UnsafePointer<CChar>?,
    _ themeValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ themeLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ themeCount: UInt32,
    _ selectedThemeValueUtf8: UnsafePointer<CChar>?,
    _ clickTitleUtf8: UnsafePointer<CChar>?,
    _ clickValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ clickLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ clickCount: UInt32,
    _ selectedClickValueUtf8: UnsafePointer<CChar>?,
    _ trailTitleUtf8: UnsafePointer<CChar>?,
    _ trailValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ trailLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ trailCount: UInt32,
    _ selectedTrailValueUtf8: UnsafePointer<CChar>?,
    _ scrollTitleUtf8: UnsafePointer<CChar>?,
    _ scrollValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ scrollLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ scrollCount: UInt32,
    _ selectedScrollValueUtf8: UnsafePointer<CChar>?,
    _ holdTitleUtf8: UnsafePointer<CChar>?,
    _ holdValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ holdLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ holdCount: UInt32,
    _ selectedHoldValueUtf8: UnsafePointer<CChar>?,
    _ hoverTitleUtf8: UnsafePointer<CChar>?,
    _ hoverValuesUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ hoverLabelsUtf8: UnsafePointer<UnsafePointer<CChar>?>?,
    _ hoverCount: UInt32,
    _ selectedHoverValueUtf8: UnsafePointer<CChar>?,
    _ callbackContext: UnsafeMutableRawPointer?,
    _ onOpenSettings: MfxTrayActionCallback?,
    _ onReloadConfig: MfxTrayActionCallback?,
    _ onExit: MfxTrayActionCallback?,
    _ onStarProject: MfxTrayActionCallback?,
    _ onThemeSelect: MfxTrayThemeSelectCallback?,
    _ onEffectSelect: MfxTrayEffectSelectCallback?
) -> UnsafeMutableRawPointer? {
    let themeValues = mfxReadUtf8StringArray(themeValuesUtf8, themeCount)
    let themeLabels = mfxReadUtf8StringArray(themeLabelsUtf8, themeCount)
    let sections: [MfxTrayEffectSectionSpec] = [
        MfxTrayEffectSectionSpec(
            category: "click",
            title: mfxNormalizeTrayText(clickTitleUtf8, "Click Effects"),
            values: mfxReadUtf8StringArray(clickValuesUtf8, clickCount),
            labels: mfxReadUtf8StringArray(clickLabelsUtf8, clickCount),
            selectedValue: mfxNormalizeTrayText(selectedClickValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "trail",
            title: mfxNormalizeTrayText(trailTitleUtf8, "Trail Effects"),
            values: mfxReadUtf8StringArray(trailValuesUtf8, trailCount),
            labels: mfxReadUtf8StringArray(trailLabelsUtf8, trailCount),
            selectedValue: mfxNormalizeTrayText(selectedTrailValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "scroll",
            title: mfxNormalizeTrayText(scrollTitleUtf8, "Scroll Effects"),
            values: mfxReadUtf8StringArray(scrollValuesUtf8, scrollCount),
            labels: mfxReadUtf8StringArray(scrollLabelsUtf8, scrollCount),
            selectedValue: mfxNormalizeTrayText(selectedScrollValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "hold",
            title: mfxNormalizeTrayText(holdTitleUtf8, "Hold Effects"),
            values: mfxReadUtf8StringArray(holdValuesUtf8, holdCount),
            labels: mfxReadUtf8StringArray(holdLabelsUtf8, holdCount),
            selectedValue: mfxNormalizeTrayText(selectedHoldValueUtf8, "")
        ),
        MfxTrayEffectSectionSpec(
            category: "hover",
            title: mfxNormalizeTrayText(hoverTitleUtf8, "Hover Effects"),
            values: mfxReadUtf8StringArray(hoverValuesUtf8, hoverCount),
            labels: mfxReadUtf8StringArray(hoverLabelsUtf8, hoverCount),
            selectedValue: mfxNormalizeTrayText(selectedHoverValueUtf8, "")
        ),
    ]

    return mfxCreateTrayMenu(
        themeTitle: mfxNormalizeTrayText(themeTitleUtf8, "Theme"),
        starProjectTitle: mfxNormalizeTrayText(starProjectTitleUtf8, "★ Star Project"),
        settingsTitle: mfxNormalizeTrayText(settingsTitleUtf8, "Settings"),
        reloadConfigTitle: mfxNormalizeTrayText(reloadConfigTitleUtf8, "Reload config"),
        exitTitle: mfxNormalizeTrayText(exitTitleUtf8, "Exit"),
        tooltip: mfxNormalizeTrayText(tooltipUtf8, "MFCMouseEffect"),
        themeValues: themeValues,
        themeLabels: themeLabels,
        selectedThemeValue: mfxNormalizeTrayText(selectedThemeValueUtf8, ""),
        effectSections: sections,
        callbackContextBits: UInt(bitPattern: callbackContext),
        onOpenSettings: onOpenSettings,
        onReloadConfig: onReloadConfig,
        onExit: onExit,
        onStarProject: onStarProject,
        onThemeSelect: onThemeSelect,
        onEffectSelect: onEffectSelect
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

    let autoEffectCategory = mfxReadTrayAutoTriggerEffectCategory()
    let autoEffectValue = mfxReadTrayAutoTriggerEffectValue()
    if !autoEffectCategory.isEmpty && !autoEffectValue.isEmpty {
        let menuHandleBits = UInt(bitPattern: menuHandle)
        if menuHandleBits != 0 {
            DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(100)) {
                guard let ptr = UnsafeMutableRawPointer(bitPattern: menuHandleBits) else {
                    return
                }
                let handle = Unmanaged<MfxTrayMenuHandle>.fromOpaque(ptr).takeUnretainedValue()
                guard let item = mfxFindEffectMenuItem(handle.menu, category: autoEffectCategory, value: autoEffectValue) else {
                    return
                }
                handle.actionBridge?.onSelectEffect(item)
            }
        }
    }

    if mfxIsTrayStarAutoTriggerEnabled() {
        let menuHandleBits = UInt(bitPattern: menuHandle)
        if menuHandleBits != 0 {
            DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(110)) {
                guard let ptr = UnsafeMutableRawPointer(bitPattern: menuHandleBits) else {
                    return
                }
                let handle = Unmanaged<MfxTrayMenuHandle>.fromOpaque(ptr).takeUnretainedValue()
                handle.actionBridge?.onOpenStarProject(nil)
            }
        }
    }

    if mfxIsTrayReloadAutoTriggerEnabled() {
        let menuHandleBits = UInt(bitPattern: menuHandle)
        if menuHandleBits != 0 {
            DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(120)) {
                guard let ptr = UnsafeMutableRawPointer(bitPattern: menuHandleBits) else {
                    return
                }
                let handle = Unmanaged<MfxTrayMenuHandle>.fromOpaque(ptr).takeUnretainedValue()
                handle.actionBridge?.onReloadConfig(nil)
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

    DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(130)) {
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
