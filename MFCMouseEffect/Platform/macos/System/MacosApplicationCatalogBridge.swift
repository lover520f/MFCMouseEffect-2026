import AppKit
import Foundation

public typealias MfxMacosApplicationCatalogEmitFn = @convention(c) (
    UnsafePointer<CChar>?,
    UnsafePointer<CChar>?,
    UnsafePointer<CChar>?,
    UnsafeMutableRawPointer?
) -> Void

private struct MfxApplicationCatalogScanRoot {
    let url: URL
    let source: String
}

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

private func mfxNormalizeToken(_ value: String) -> String {
    value.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
}

private func mfxDirectoryExists(_ path: String) -> Bool {
    var isDirectory = ObjCBool(false)
    return FileManager.default.fileExists(atPath: path, isDirectory: &isDirectory)
        && isDirectory.boolValue
}

private func mfxBuildApplicationCatalogScanRoots() -> [MfxApplicationCatalogScanRoot] {
    var roots: [MfxApplicationCatalogScanRoot] = []
    var dedup = Set<String>()

    func addRoot(_ path: String, source: String) {
        guard mfxDirectoryExists(path) else {
            return
        }
        let canonical = URL(fileURLWithPath: path, isDirectory: true)
            .standardizedFileURL
            .resolvingSymlinksInPath()
            .path
        let dedupKey = mfxNormalizeToken(canonical)
        guard !dedupKey.isEmpty else {
            return
        }
        guard dedup.insert(dedupKey).inserted else {
            return
        }
        roots.append(
            MfxApplicationCatalogScanRoot(
                url: URL(fileURLWithPath: canonical, isDirectory: true),
                source: source
            )
        )
    }

    addRoot("/Applications", source: "applications")
    addRoot("/System/Applications", source: "system")
    addRoot((NSHomeDirectory() as NSString).appendingPathComponent("Applications"), source: "home")
    return roots
}

private func mfxCollectApplicationBundleUrls(_ root: URL) -> [URL] {
    let fileManager = FileManager.default
    let keys: [URLResourceKey] = [.isDirectoryKey, .isApplicationKey]
    guard let enumerator = fileManager.enumerator(
        at: root,
        includingPropertiesForKeys: keys,
        options: [.skipsHiddenFiles],
        errorHandler: { _, _ in true }
    ) else {
        return []
    }

    var bundles: [URL] = []
    for case let candidate as URL in enumerator {
        guard let values = try? candidate.resourceValues(forKeys: Set(keys)) else {
            continue
        }
        guard values.isDirectory == true else {
            continue
        }
        guard values.isApplication == true else {
            continue
        }
        bundles.append(candidate)
        enumerator.skipDescendants()
    }
    return bundles
}

private func mfxResolveProcessName(_ bundleUrl: URL) -> String {
    let bundleStem = mfxNormalizeToken(bundleUrl.deletingPathExtension().lastPathComponent)
    if !bundleStem.isEmpty {
        return "\(bundleStem).app"
    }

    if let executable = Bundle(url: bundleUrl)?.executableURL?.lastPathComponent {
        let normalized = mfxNormalizeToken(executable)
        if !normalized.isEmpty {
            return "\(normalized).app"
        }
    }

    if let executable = Bundle(url: bundleUrl)?.object(forInfoDictionaryKey: "CFBundleExecutable") as? String {
        let normalized = mfxNormalizeToken(executable)
        if !normalized.isEmpty {
            return "\(normalized).app"
        }
    }
    return ""
}

private func mfxResolveDisplayName(_ bundleUrl: URL, fallback: String) -> String {
    let displayName = FileManager.default.displayName(atPath: bundleUrl.path)
        .trimmingCharacters(in: .whitespacesAndNewlines)
    if !displayName.isEmpty {
        return displayName
    }

    let bundleStem = bundleUrl.deletingPathExtension().lastPathComponent
        .trimmingCharacters(in: .whitespacesAndNewlines)
    if !bundleStem.isEmpty {
        return bundleStem
    }
    return fallback
}

@_cdecl("mfx_macos_scan_application_catalog_v1")
public func mfx_macos_scan_application_catalog_v1(
    _ emitFn: MfxMacosApplicationCatalogEmitFn?,
    _ context: UnsafeMutableRawPointer?,
    _ outErrorUtf8: UnsafeMutablePointer<CChar>?,
    _ outErrorCapacity: Int32
) -> Int32 {
    mfxCopyCString("", outErrorUtf8, outErrorCapacity)
    guard let emitFn else {
        mfxCopyCString("emit_function_missing", outErrorUtf8, outErrorCapacity)
        return -1
    }

    let roots = mfxBuildApplicationCatalogScanRoots()
    var emittedCount: Int32 = 0
    for root in roots {
        let bundleUrls = mfxCollectApplicationBundleUrls(root.url)
        for bundleUrl in bundleUrls {
            let processName = mfxResolveProcessName(bundleUrl)
            if processName.isEmpty {
                continue
            }
            let displayName = mfxResolveDisplayName(bundleUrl, fallback: processName)
            processName.withCString { processNameUtf8 in
                displayName.withCString { displayNameUtf8 in
                    root.source.withCString { sourceUtf8 in
                        emitFn(processNameUtf8, displayNameUtf8, sourceUtf8, context)
                    }
                }
            }
            emittedCount += 1
        }
    }

    return emittedCount
}
