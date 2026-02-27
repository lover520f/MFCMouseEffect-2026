import Foundation
import AppKit
import UserNotifications

@available(macOS 11.0, *)
private func mfxDeliverWithUserNotifications(_ safeTitle: String, _ safeMessage: String) -> Bool {
    guard let bundleId = Bundle.main.bundleIdentifier, !bundleId.isEmpty else {
        return false
    }

    let center = UNUserNotificationCenter.current()
    let semaphore = DispatchSemaphore(value: 0)
    var delivered = false

    center.getNotificationSettings { settings in
        let enqueue: () -> Void = {
            let content = UNMutableNotificationContent()
            content.title = safeTitle
            content.body = safeMessage

            let request = UNNotificationRequest(
                identifier: "mfx.warning.\(UUID().uuidString)",
                content: content,
                trigger: nil)
            center.add(request) { error in
                delivered = (error == nil)
                semaphore.signal()
            }
        }

        switch settings.authorizationStatus {
        case .authorized, .provisional, .ephemeral:
            enqueue()
        case .notDetermined:
            center.requestAuthorization(options: [.alert, .sound]) { granted, _ in
                if granted {
                    enqueue()
                } else {
                    semaphore.signal()
                }
            }
        default:
            semaphore.signal()
        }
    }

    _ = semaphore.wait(timeout: .now() + 1.5)
    return delivered
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

    if #available(macOS 11.0, *) {
        if mfxDeliverWithUserNotifications(safeTitle, safeMessage) {
            return true
        }
    } else {
        let notification = NSUserNotification()
        notification.title = safeTitle
        notification.informativeText = safeMessage
        NSUserNotificationCenter.default.deliver(notification)
        return true
    }

    let escapedTitle = safeTitle
        .replacingOccurrences(of: "\\", with: "\\\\")
        .replacingOccurrences(of: "\"", with: "\\\"")
    let escapedMessage = safeMessage
        .replacingOccurrences(of: "\\", with: "\\\\")
        .replacingOccurrences(of: "\"", with: "\\\"")
    let script = "display notification \"\(escapedMessage)\" with title \"\(escapedTitle)\""

    let process = Process()
    process.executableURL = URL(fileURLWithPath: "/usr/bin/osascript")
    process.arguments = ["-e", script]

    if #available(macOS 10.15, *) {
        process.standardOutput = FileHandle.nullDevice
        process.standardError = FileHandle.nullDevice
    }

    do {
        try process.run()
        process.waitUntilExit()
        return process.terminationStatus == 0
    } catch {
        return false
    }
}
