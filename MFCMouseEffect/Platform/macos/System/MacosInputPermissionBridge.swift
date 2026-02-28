import ApplicationServices

@_cdecl("mfx_macos_is_process_trusted_v1")
public func mfx_macos_is_process_trusted_v1() -> Int32 {
    return AXIsProcessTrusted() ? 1 : 0
}
