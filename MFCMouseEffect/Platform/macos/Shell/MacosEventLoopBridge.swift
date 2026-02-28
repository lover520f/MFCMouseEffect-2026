@preconcurrency import AppKit
@preconcurrency import Foundation

@MainActor
private func mfxEnsureApplicationReadyOnMainThread() {
    _ = NSApplication.shared
}

@MainActor
private func mfxRunApplicationEventLoopOnMainThread() {
    NSApplication.shared.run()
}

@MainActor
private func mfxStopApplicationEventLoopOnMainThread() {
    guard let app = NSApp else {
        return
    }

    app.stop(nil)
    let wakeEvent = NSEvent.otherEvent(
        with: .applicationDefined,
        location: NSZeroPoint,
        modifierFlags: [],
        timestamp: 0,
        windowNumber: 0,
        context: nil,
        subtype: 0,
        data1: 0,
        data2: 0
    )
    if let wakeEvent {
        app.postEvent(wakeEvent, atStart: false)
    }
}

@_cdecl("mfx_macos_event_loop_ensure_application_ready_v1")
public func mfx_macos_event_loop_ensure_application_ready_v1() {
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxEnsureApplicationReadyOnMainThread()
        }
        return
    }
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxEnsureApplicationReadyOnMainThread()
        }
    }
}

@_cdecl("mfx_macos_event_loop_run_application_v1")
public func mfx_macos_event_loop_run_application_v1() {
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxRunApplicationEventLoopOnMainThread()
        }
        return
    }
    DispatchQueue.main.sync {
        MainActor.assumeIsolated {
            mfxRunApplicationEventLoopOnMainThread()
        }
    }
}

@_cdecl("mfx_macos_event_loop_stop_application_v1")
public func mfx_macos_event_loop_stop_application_v1() {
    if Thread.isMainThread {
        MainActor.assumeIsolated {
            mfxStopApplicationEventLoopOnMainThread()
        }
        return
    }
    DispatchQueue.main.async {
        MainActor.assumeIsolated {
            mfxStopApplicationEventLoopOnMainThread()
        }
    }
}
