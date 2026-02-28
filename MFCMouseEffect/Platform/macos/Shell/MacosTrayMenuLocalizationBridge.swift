import Foundation

private func mfxFirstPreferredLanguageCode() -> String {
    guard let first = Locale.preferredLanguages.first else {
        return ""
    }
    return first.lowercased()
}

@_cdecl("mfx_macos_tray_prefers_zh_language_v1")
public func mfx_macos_tray_prefers_zh_language_v1() -> Int32 {
    let languageCode = mfxFirstPreferredLanguageCode()
    if languageCode.hasPrefix("zh") {
        return 1
    }
    return 0
}
