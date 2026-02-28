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

private func mfxResolveInitialDirectory(_ initialPathUtf8: String) -> URL? {
    if initialPathUtf8.isEmpty {
        return nil
    }
    let expanded = NSString(string: initialPathUtf8).expandingTildeInPath
    if expanded.isEmpty {
        return nil
    }

    var isDir = ObjCBool(false)
    if FileManager.default.fileExists(atPath: expanded, isDirectory: &isDir) {
        if isDir.boolValue {
            return URL(fileURLWithPath: expanded, isDirectory: true)
        }
        return URL(fileURLWithPath: expanded).deletingLastPathComponent()
    }
    return nil
}

@MainActor
private func mfxPickFolderViaOpenPanelOnMainThread(
    titleUtf8: String,
    initialPathUtf8: String
) -> (code: Int32, path: String, error: String) {
    NSApplication.shared.activate(ignoringOtherApps: true)

    let panel = NSOpenPanel()
    panel.canChooseFiles = false
    panel.canChooseDirectories = true
    panel.allowsMultipleSelection = false
    panel.resolvesAliases = true
    if !titleUtf8.isEmpty {
        panel.message = titleUtf8
    }
    if let initialUrl = mfxResolveInitialDirectory(initialPathUtf8) {
        panel.directoryURL = initialUrl
    }

    let result = panel.runModal()
    if result == .OK {
        guard let selectedPath = panel.url?.path, !selectedPath.isEmpty else {
            return (-1, "", "selected folder path missing")
        }
        return (1, selectedPath, "")
    }
    if result == .cancel {
        return (0, "", "cancelled")
    }
    return (-1, "", "failed to show folder dialog")
}

private func mfxTrimAsciiWhitespace(_ value: String) -> String {
    value.trimmingCharacters(in: .whitespacesAndNewlines)
}

private func mfxEscapeForAppleScriptString(_ value: String) -> String {
    var escaped = ""
    escaped.reserveCapacity(value.count)
    for scalar in value.unicodeScalars {
        switch scalar {
        case "\\":
            escaped.append("\\\\")
        case "\"":
            escaped.append("\\\"")
        case "\n", "\r":
            escaped.append(" ")
        default:
            escaped.append(String(scalar))
        }
    }
    return escaped
}

private func mfxBuildAppleScriptChooseFolderSource(
    titleUtf8: String,
    initialPathUtf8: String
) -> String {
    let prompt = mfxEscapeForAppleScriptString(
        titleUtf8.isEmpty ? "Select WASM plugin folder" : titleUtf8
    )
    let initialDirectory = mfxResolveInitialDirectory(initialPathUtf8)?.path ?? ""
    let safeInitialDirectory = mfxEscapeForAppleScriptString(initialDirectory)

    var source = "try\n"
    var chooseLine = "set pickedFolder to choose folder with prompt \"\(prompt)\""
    if !safeInitialDirectory.isEmpty {
        chooseLine += " default location ((POSIX file \"\(safeInitialDirectory)\") as alias)"
    }
    source += chooseLine
    source += "\nreturn POSIX path of pickedFolder\n"
    source += "on error number -128\n"
    source += "return \"__MFX_CANCELLED__\"\n"
    source += "on error errMsg number errNum\n"
    source += "return \"__MFX_ERROR__\" & errNum & \":\" & errMsg\n"
    source += "end try\n"
    return source
}

private func mfxReadAppleScriptError(_ errorInfo: NSDictionary?) -> String {
    guard let errorInfo = errorInfo as? [String: Any], !errorInfo.isEmpty else {
        return "apple_script_execute_failed"
    }

    var out = ""
    if let number = errorInfo["NSAppleScriptErrorNumber"] as? NSNumber {
        out += "code=\(number.intValue)"
    }
    if let message = errorInfo["NSAppleScriptErrorMessage"] as? String, !message.isEmpty {
        if !out.isEmpty {
            out += ","
        }
        out += message
    }
    return out.isEmpty ? "apple_script_execute_failed" : out
}

@MainActor
private func mfxPickFolderViaAppleScriptOnMainThread(
    titleUtf8: String,
    initialPathUtf8: String
) -> (code: Int32, path: String, error: String) {
    let source = mfxBuildAppleScriptChooseFolderSource(
        titleUtf8: titleUtf8,
        initialPathUtf8: initialPathUtf8
    )
    guard !source.isEmpty else {
        return (-1, "", "apple_script_source_invalid")
    }

    guard let script = NSAppleScript(source: source) else {
        return (-1, "", "apple_script_compile_failed")
    }

    var errorInfo: NSDictionary?
    let result = script.executeAndReturnError(&errorInfo)
    if errorInfo != nil && result.stringValue == nil {
        return (-1, "", mfxReadAppleScriptError(errorInfo))
    }

    let trimmed = mfxTrimAsciiWhitespace(result.stringValue ?? "")
    if trimmed == "__MFX_CANCELLED__" {
        return (0, "", "cancelled")
    }
    if trimmed.hasPrefix("__MFX_ERROR__") {
        return (-1, "", trimmed.isEmpty ? "apple_script_choose_folder_failed" : trimmed)
    }
    if trimmed.isEmpty {
        return (-1, "", "selected folder path missing")
    }
    return (1, trimmed, "")
}

@MainActor
private func mfxPickFolderOnMainThread(titleUtf8: String, initialPathUtf8: String) -> (code: Int32, path: String, error: String) {
    let openPanelResult = mfxPickFolderViaOpenPanelOnMainThread(
        titleUtf8: titleUtf8,
        initialPathUtf8: initialPathUtf8
    )
    if openPanelResult.code >= 0 {
        return openPanelResult
    }

    var appleScriptResult = mfxPickFolderViaAppleScriptOnMainThread(
        titleUtf8: titleUtf8,
        initialPathUtf8: initialPathUtf8
    )
    if appleScriptResult.code < 0 && !openPanelResult.error.isEmpty {
        if !appleScriptResult.error.isEmpty {
            appleScriptResult.error += "; "
        }
        appleScriptResult.error += "fallback_from_open_panel: \(openPanelResult.error)"
    }
    return appleScriptResult
}

@_cdecl("mfx_macos_pick_folder_v1")
public func mfx_macos_pick_folder_v1(
    _ titleUtf8: UnsafePointer<CChar>?,
    _ initialPathUtf8: UnsafePointer<CChar>?,
    _ outPathUtf8: UnsafeMutablePointer<CChar>?,
    _ outPathCapacity: Int32,
    _ outErrorUtf8: UnsafeMutablePointer<CChar>?,
    _ outErrorCapacity: Int32
) -> Int32 {
    mfxCopyCString("", outPathUtf8, outPathCapacity)
    mfxCopyCString("", outErrorUtf8, outErrorCapacity)

    let title = titleUtf8.map { String(cString: $0) } ?? ""
    let initialPath = initialPathUtf8.map { String(cString: $0) } ?? ""

    let outcome: (code: Int32, path: String, error: String)
    if Thread.isMainThread {
        outcome = MainActor.assumeIsolated {
            mfxPickFolderOnMainThread(titleUtf8: title, initialPathUtf8: initialPath)
        }
    } else {
        var captured: (code: Int32, path: String, error: String) = (-1, "", "failed to dispatch picker to main thread")
        DispatchQueue.main.sync {
            captured = MainActor.assumeIsolated {
                mfxPickFolderOnMainThread(titleUtf8: title, initialPathUtf8: initialPath)
            }
        }
        outcome = captured
    }

    mfxCopyCString(outcome.path, outPathUtf8, outPathCapacity)
    mfxCopyCString(outcome.error, outErrorUtf8, outErrorCapacity)
    return outcome.code
}
