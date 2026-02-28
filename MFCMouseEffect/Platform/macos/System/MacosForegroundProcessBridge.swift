import AppKit
import Foundation

private func mfxCopyCString(_ value: String, _ outBuffer: UnsafeMutablePointer<CChar>?, _ capacity: Int32) {
    guard let outBuffer, capacity > 0 else {
        return
    }
    let maxCopy = Int(capacity) - 1
    if maxCopy < 0 {
        return
    }

    let utf8 = value.utf8CString
    let copyCount = min(maxCopy, max(0, utf8.count - 1))
    if copyCount > 0 {
        for i in 0..<copyCount {
            outBuffer[i] = utf8[i]
        }
    }
    outBuffer[copyCount] = 0
}

private func mfxNormalizeProcessName(_ value: String?) -> String {
    guard let value else {
        return ""
    }
    let trimmed = value.trimmingCharacters(in: .whitespacesAndNewlines)
    if trimmed.isEmpty {
        return ""
    }
    return trimmed.lowercased()
}

@MainActor
private func mfxResolveProcessNameFromApp(_ app: NSRunningApplication?) -> String {
    guard let app else {
        return ""
    }

    if let executableUrl = app.executableURL {
        let normalized = mfxNormalizeProcessName(executableUrl.lastPathComponent)
        if !normalized.isEmpty {
            return normalized
        }
    }

    return mfxNormalizeProcessName(app.localizedName)
}

@MainActor
private func mfxResolveForegroundProcessNameOnMainThread() -> String {
    var processName = mfxResolveProcessNameFromApp(NSWorkspace.shared.frontmostApplication)
    if !processName.isEmpty {
        return processName
    }

    processName = mfxResolveProcessNameFromApp(NSRunningApplication.current)
    if !processName.isEmpty {
        return processName
    }

    processName = mfxNormalizeProcessName(ProcessInfo.processInfo.processName)
    if !processName.isEmpty {
        return processName
    }
    return "unknown"
}

@_cdecl("mfx_macos_resolve_foreground_process_name_v1")
public func mfx_macos_resolve_foreground_process_name_v1(
    _ outProcessNameUtf8: UnsafeMutablePointer<CChar>?,
    _ outProcessNameCapacity: Int32
) -> Int32 {
    mfxCopyCString("", outProcessNameUtf8, outProcessNameCapacity)
    guard outProcessNameUtf8 != nil, outProcessNameCapacity > 0 else {
        return -1
    }

    let processName: String
    if Thread.isMainThread {
        processName = MainActor.assumeIsolated {
            mfxResolveForegroundProcessNameOnMainThread()
        }
    } else {
        var captured = ""
        DispatchQueue.main.sync {
            captured = MainActor.assumeIsolated {
                mfxResolveForegroundProcessNameOnMainThread()
            }
        }
        processName = captured
    }

    if processName.isEmpty {
        return 0
    }
    mfxCopyCString(processName, outProcessNameUtf8, outProcessNameCapacity)
    return 1
}
