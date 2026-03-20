import Foundation
import AppKit
import UserNotifications

@MainActor
private var mfxNotificationAppIconInitialized = false

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

private func mfxResolveNotificationAppIcon() -> NSImage? {
    for candidate in mfxProjectLogoPathCandidates() {
        if FileManager.default.fileExists(atPath: candidate), let image = NSImage(contentsOfFile: candidate) {
            return image
        }
    }

    if #available(macOS 11.0, *) {
        return NSImage(systemSymbolName: "sparkles", accessibilityDescription: "MFX")
    }

    return nil
}

@MainActor
private func mfxEnsureNotificationAppIconOnMainActor() {
    if mfxNotificationAppIconInitialized {
        return
    }
    mfxNotificationAppIconInitialized = true

    guard let icon = mfxResolveNotificationAppIcon() else {
        return
    }
    NSApplication.shared.applicationIconImage = icon
}

private func mfxEnsureNotificationAppIcon() {
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxEnsureNotificationAppIconOnMainActor()
        }
        return
    }
    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxEnsureNotificationAppIconOnMainActor()
        }
    }
}

@available(macOS 11.0, *)
private func mfxDeliverWithUserNotifications(_ safeTitle: String, _ safeMessage: String) {
    guard let bundleId = Bundle.main.bundleIdentifier, !bundleId.isEmpty else {
        return
    }

    let center = UNUserNotificationCenter.current()
    center.getNotificationSettings { settings in
        let enqueue: () -> Void = {
            let content = UNMutableNotificationContent()
            content.title = safeTitle
            content.body = safeMessage

            let request = UNNotificationRequest(
                identifier: "mfx.warning.\(UUID().uuidString)",
                content: content,
                trigger: nil)
            center.add(request) { _ in }
        }

        switch settings.authorizationStatus {
        case .authorized, .provisional, .ephemeral:
            enqueue()
        case .notDetermined:
            center.requestAuthorization(options: [.alert, .sound]) { granted, _ in
                if granted {
                    enqueue()
                }
            }
        default:
            break
        }
    }
}

@_cdecl("mfx_macos_show_warning_notification")
public func mfx_macos_show_warning_notification(
    _ safeTitleUtf8: UnsafePointer<CChar>?,
    _ safeMessageUtf8: UnsafePointer<CChar>?
) -> Bool {
    guard let titlePtr = safeTitleUtf8, let messagePtr = safeMessageUtf8 else {
        return false
    }

    let safeTitle = String(cString: titlePtr)
    let safeMessage = String(cString: messagePtr)
    mfxEnsureNotificationAppIcon()

    if #available(macOS 11.0, *) {
        mfxDeliverWithUserNotifications(safeTitle, safeMessage)
        return true
    } else {
        DispatchQueue.main.async {
            let notification = NSUserNotification()
            notification.title = safeTitle
            notification.informativeText = safeMessage
            NSUserNotificationCenter.default.deliver(notification)
        }
        return true
    }
}
