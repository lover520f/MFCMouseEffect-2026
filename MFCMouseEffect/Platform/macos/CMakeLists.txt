if(TARGET mfx_shell_macos)
    return()
endif()

if(NOT TARGET mfx_shell_posix_common)
    message(FATAL_ERROR "mfx_shell_posix_common is required before configuring mfx_shell_macos.")
endif()

set(MFX_MACOS_TRAY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayService.cpp")
set(MFX_MACOS_TRAY_LOCALIZATION_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuLocalization.cpp")
set(MFX_MACOS_TRAY_RUNTIME_HELPERS_SOURCE "")
set(MFX_MACOS_TRAY_MENU_FACTORY_SOURCE "")
set(MFX_MACOS_TRAY_MENU_FACTORY_ACTION_BRIDGE_SOURCE "")
set(MFX_MACOS_TRAY_MENU_FACTORY_ITEMS_SOURCE "")
if(APPLE)
    set(MFX_MACOS_TRAY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayService.mm")
    set(MFX_MACOS_TRAY_LOCALIZATION_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuLocalization.mm")
    set(MFX_MACOS_TRAY_RUNTIME_HELPERS_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayRuntimeHelpers.mm")
    set(MFX_MACOS_TRAY_MENU_FACTORY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuFactory.mm")
    set(MFX_MACOS_TRAY_MENU_FACTORY_ACTION_BRIDGE_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuFactory.ActionBridge.mm")
    set(MFX_MACOS_TRAY_MENU_FACTORY_ITEMS_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuFactory.Items.mm")
endif()

add_library(mfx_shell_macos STATIC
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageCodec.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageHost.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageHost.Messaging.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageHost.Timers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseEffect.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Plan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayStyle.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseWindowRegistry.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectCreatorRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectCreatorRegistry.Table.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectRenderProfile.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectRenderProfile.ClickTrail.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectRenderProfile.ScrollHoldHover.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosOverlayRenderSupport.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseEffect.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayStyle.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Plan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseWindowRegistry.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseEffect.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayStyle.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Plan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseWindowRegistry.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseEffect.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayStyle.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseEffect.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Accent.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.State.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Update.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosOverlayCoordSpaceService.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Lifecycle.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Display.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Probes.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.ProbeHelpers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Text.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Image.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderResolvers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRenderer.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Plan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayRenderMath.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayPolicy.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.Admission.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.Internals.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayRuntime.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.Layout.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.Style.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTransientOverlay.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosDpiAwarenessService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopBridge.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopService.RunLoop.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosShellServicesFactory.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosSettingsLauncher.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosSingleInstanceGuard.cpp"
    "${MFX_MACOS_TRAY_RUNTIME_HELPERS_SOURCE}"
    "${MFX_MACOS_TRAY_MENU_FACTORY_SOURCE}"
    "${MFX_MACOS_TRAY_MENU_FACTORY_ACTION_BRIDGE_SOURCE}"
    "${MFX_MACOS_TRAY_MENU_FACTORY_ITEMS_SOURCE}"
    "${MFX_MACOS_TRAY_LOCALIZATION_SOURCE}"
    "${MFX_MACOS_TRAY_SOURCE}"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogEntryStore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanRoots.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosCursorPositionService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanWorkflow.BundleResolve.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanWorkflow.BundleResolve.Helpers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanWorkflow.RootScan.Enumerate.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanWorkflow.RootScan.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanWorkflow.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosForegroundProcessService.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosForegroundProcessService.Resolve.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Mouse.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Key.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTap.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.RunLoop.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.RunLoopHelpers.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputEventUtils.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputPermissionState.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputPermissionState.Parse.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjector.EventPost.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjector.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyResolver.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyResolver.NonModifier.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Printable.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Function.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Special.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Modifier.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosVirtualKeyMapper.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosVirtualKeyMapper.KeyPairs.mm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosUserNotificationService.AppleScript.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosUserNotificationService.cpp"
)

target_link_libraries(mfx_shell_macos
    PUBLIC
        mfx_shell_contracts
        mfx_shell_posix_common
)
if(APPLE)
    target_link_libraries(mfx_shell_macos PUBLIC "-framework CoreFoundation")
    target_link_libraries(mfx_shell_macos PUBLIC "-framework AppKit")
    target_link_libraries(mfx_shell_macos PUBLIC "-framework ApplicationServices")
    target_link_libraries(mfx_shell_macos PUBLIC "-framework QuartzCore")
endif()
target_compile_features(mfx_shell_macos PUBLIC cxx_std_17)
target_compile_definitions(mfx_shell_macos PRIVATE MFX_TARGET_PLATFORM_MACOS=1)

if(APPLE)
    add_executable(mfx_shell_macos_smoke
        "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosShellSmokeMain.cpp"
    )
    target_link_libraries(mfx_shell_macos_smoke
        PRIVATE
            mfx_shell_contracts
            mfx_shell_macos
    )
    target_compile_features(mfx_shell_macos_smoke PRIVATE cxx_std_17)
    target_compile_definitions(mfx_shell_macos_smoke PRIVATE MFX_TARGET_PLATFORM_MACOS=1)

    add_executable(mfx_shell_macos_tray_smoke
        "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTraySmokeMain.mm"
    )
    target_link_libraries(mfx_shell_macos_tray_smoke
        PRIVATE
            mfx_shell_contracts
            mfx_shell_macos
    )
    target_compile_features(mfx_shell_macos_tray_smoke PRIVATE cxx_std_17)
    target_compile_definitions(mfx_shell_macos_tray_smoke PRIVATE MFX_TARGET_PLATFORM_MACOS=1)
endif()

message(STATUS "Configured shell package target: mfx_shell_macos")
