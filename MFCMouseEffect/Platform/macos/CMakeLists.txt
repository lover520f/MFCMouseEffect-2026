if(TARGET mfx_shell_macos)
    return()
endif()

if(NOT TARGET mfx_shell_posix_common)
    message(FATAL_ERROR "mfx_shell_posix_common is required before configuring mfx_shell_macos.")
endif()

set(MFX_MACOS_TRAY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayService.cpp")
set(MFX_MACOS_TRAY_LOCALIZATION_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuLocalization.cpp")
set(MFX_MACOS_TRAY_MENU_FACTORY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuFactory.cpp")
set(MFX_MACOS_SWIFT_OBJECTS "")
set(MFX_SWIFT_LANGUAGE_MODE "auto" CACHE STRING "Swift language mode for macOS bridges: auto|5|6")
set_property(CACHE MFX_SWIFT_LANGUAGE_MODE PROPERTY STRINGS auto 5 6)
if(APPLE)
    find_program(MFX_SWIFTC_EXECUTABLE swiftc REQUIRED)
    set(MFX_SWIFTC_COMMON_ARGS -parse-as-library -c)
    set(MFX_SWIFT_MODULE_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/swift-module-cache")
    list(APPEND MFX_SWIFTC_COMMON_ARGS -module-cache-path "${MFX_SWIFT_MODULE_CACHE_DIR}")
    set(MFX_SWIFT_LANGUAGE_MODE_RESOLVED "")
    if(MFX_SWIFT_LANGUAGE_MODE STREQUAL "auto")
        execute_process(
            COMMAND "${MFX_SWIFTC_EXECUTABLE}" --version
            OUTPUT_VARIABLE MFX_SWIFTC_VERSION_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(MFX_SWIFTC_VERSION_OUTPUT MATCHES "Swift version ([0-9]+)")
            set(MFX_SWIFT_VERSION_MAJOR "${CMAKE_MATCH_1}")
            if(MFX_SWIFT_VERSION_MAJOR GREATER_EQUAL 6)
                set(MFX_SWIFT_LANGUAGE_MODE_RESOLVED "6")
            else()
                set(MFX_SWIFT_LANGUAGE_MODE_RESOLVED "5")
            endif()
        else()
            # Keep a deterministic fallback if version parsing fails.
            set(MFX_SWIFT_LANGUAGE_MODE_RESOLVED "5")
        endif()
    elseif(MFX_SWIFT_LANGUAGE_MODE STREQUAL "5" OR MFX_SWIFT_LANGUAGE_MODE STREQUAL "6")
        set(MFX_SWIFT_LANGUAGE_MODE_RESOLVED "${MFX_SWIFT_LANGUAGE_MODE}")
    else()
        message(FATAL_ERROR
            "Unsupported MFX_SWIFT_LANGUAGE_MODE='${MFX_SWIFT_LANGUAGE_MODE}'. "
            "Supported values: auto|5|6")
    endif()
    list(PREPEND MFX_SWIFTC_COMMON_ARGS -swift-version "${MFX_SWIFT_LANGUAGE_MODE_RESOLVED}")
    message(STATUS
        "macOS Swift language mode: ${MFX_SWIFT_LANGUAGE_MODE_RESOLVED} "
        "(requested=${MFX_SWIFT_LANGUAGE_MODE})")

    set(MFX_SWIFT_DEPLOYMENT_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    if(NOT MFX_SWIFT_DEPLOYMENT_TARGET)
        execute_process(
            COMMAND /usr/bin/xcrun --sdk macosx --show-sdk-platform-version
            OUTPUT_VARIABLE MFX_MACOS_SDK_PLATFORM_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(MFX_MACOS_SDK_PLATFORM_VERSION)
            string(REGEX MATCH "^[0-9]+(\\.[0-9]+)?" MFX_SWIFT_DEPLOYMENT_TARGET "${MFX_MACOS_SDK_PLATFORM_VERSION}")
        endif()
    endif()
    if(MFX_SWIFT_DEPLOYMENT_TARGET)
        set(MFX_SWIFT_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
        if(CMAKE_OSX_ARCHITECTURES)
            list(GET CMAKE_OSX_ARCHITECTURES 0 MFX_SWIFT_TARGET_ARCH)
        endif()
        if(MFX_SWIFT_TARGET_ARCH)
            list(PREPEND MFX_SWIFTC_COMMON_ARGS -target "${MFX_SWIFT_TARGET_ARCH}-apple-macosx${MFX_SWIFT_DEPLOYMENT_TARGET}")
        endif()
    endif()
    function(mfx_add_swift_bridge swift_relative_path swift_label)
        get_filename_component(mfx_swift_base_name "${swift_relative_path}" NAME)
        set(mfx_swift_source "${MFX_PROJECT_ROOT}/${swift_relative_path}")
        set(mfx_swift_object "${CMAKE_CURRENT_BINARY_DIR}/${mfx_swift_base_name}.o")
        add_custom_command(
            OUTPUT "${mfx_swift_object}"
            COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${mfx_swift_source}" -o "${mfx_swift_object}"
            DEPENDS "${mfx_swift_source}"
            COMMENT "Compile Swift ${swift_label}"
            VERBATIM
        )
        set_source_files_properties(
            "${mfx_swift_object}"
            PROPERTIES
                GENERATED TRUE
                EXTERNAL_OBJECT TRUE
        )
        set(mfx_swift_objects_local "${MFX_MACOS_SWIFT_OBJECTS}")
        list(APPEND mfx_swift_objects_local "${mfx_swift_object}")
        set(MFX_MACOS_SWIFT_OBJECTS "${mfx_swift_objects_local}" PARENT_SCOPE)
    endfunction()

    mfx_add_swift_bridge("Platform/macos/Shell/MacosUserNotificationBridge.swift" "notification bridge")
    mfx_add_swift_bridge("Platform/macos/System/MacosNativeFolderPickerBridge.swift" "folder picker bridge")
    mfx_add_swift_bridge("Platform/macos/Shell/MacosSettingsLauncherBridge.swift" "settings launcher bridge")
    mfx_add_swift_bridge("Platform/macos/System/MacosApplicationCatalogBridge.swift" "application catalog bridge")
    mfx_add_swift_bridge("Platform/macos/System/MacosForegroundProcessBridge.swift" "foreground process bridge")
    mfx_add_swift_bridge("Platform/macos/System/MacosInputPermissionBridge.swift" "input permission bridge")
    mfx_add_swift_bridge("Platform/macos/System/MacosLaunchAtStartupBridge.swift" "launch-at-startup bridge")
    mfx_add_swift_bridge("Platform/macos/Overlay/MacosInputIndicatorBridge.swift" "input indicator bridge")
    mfx_add_swift_bridge("Platform/macos/Overlay/MacosCursorDecorationBridge.swift" "cursor decoration bridge")
    mfx_add_swift_bridge("Platform/macos/Overlay/MacosOverlayCoordSpaceBridge.swift" "overlay coord-space bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosOverlayRenderSupportBridge.swift" "overlay render-support bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosClickPulseOverlayBridge.swift" "click-pulse overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosScrollPulseOverlayBridge.swift" "scroll-pulse overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosScrollHelixAnimator.swift" "scroll-helix animator bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosScrollTwinkleAnimator.swift" "scroll-twinkle animator bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosTrailPulseOverlayBridge.swift" "trail-pulse overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosLineTrailOverlayBridge.swift" "line-trail overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosTextEffectFallbackBridge.swift" "text-effect fallback bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosHoverPulseOverlayBridge.swift" "hover-pulse overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Effects/MacosHoldPulseOverlayBridge.swift" "hold-pulse overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Pet/MacosMouseCompanionPhase1Bridge.swift" "mouse companion phase1 bridge")
    mfx_add_swift_bridge("Platform/macos/Shell/MacosTrayMenuLocalizationBridge.swift" "tray localization bridge")
    mfx_add_swift_bridge("Platform/macos/Shell/MacosTrayMenuBridge.swift" "tray menu bridge")
    mfx_add_swift_bridge("Platform/macos/Shell/MacosEventLoopBridge.swift" "event loop bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmImageOverlayBridge.swift" "WASM image overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmPolylineOverlayBridge.swift" "WASM polyline overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmPathStrokeOverlayBridge.swift" "WASM path-stroke overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmPathFillOverlayBridge.swift" "WASM path-fill overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmGlowBatchOverlayBridge.swift" "WASM glow-batch overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmSpriteBatchOverlayBridge.swift" "WASM sprite-batch overlay bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmRetainedGlowEmitterBridge.swift" "WASM retained glow emitter bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmRetainedQuadFieldBridge.swift" "WASM retained quad field bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmRetainedRibbonTrailBridge.swift" "WASM retained ribbon trail bridge")
    mfx_add_swift_bridge("Platform/macos/Wasm/MacosWasmRetainedSpriteEmitterBridge.swift" "WASM retained sprite emitter bridge")
endif()

add_library(mfx_shell_macos STATIC
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/ClickEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/TextEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/TrailEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/TrailStyleCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/ScrollEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/HoverEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/HoldEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageCodec.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageHost.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageHost.Messaging.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Control/MacosDispatchMessageHost.Timers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseWindowRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectCreatorRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectCreatorRegistry.Table.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectComputeProfileAdapter.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectRenderProfile.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectRenderProfile.ClickTrail.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosEffectRenderProfile.ScrollHoldHover.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosOverlayRenderSupport.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosLineTrailOverlay.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseEmissionPlanner.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailStyleComputeSwiftBridge.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseWindowRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseWindowRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollFrameComputeBridge.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.State.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Update.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTextEffectFallback.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosOverlayCoordSpaceService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Lifecycle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Display.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.Probes.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlay.ProbeHelpers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Text.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Image.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Pulse.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Polyline.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.PathStroke.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.PathFill.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.GlowBatch.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.GlowEmitter.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Group.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.ParticleEmitter.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.QuadField.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.RibbonTrail.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.SpriteEmitter.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.SpriteBatch.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderResolvers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmPolylineOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmPathStrokeOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmPathFillOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmGlowBatchOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmSpriteBatchOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayRenderMath.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayPolicy.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.Admission.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.Internals.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.Windows.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayState.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayRuntime.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlayFallback.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTransientOverlay.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosDpiAwarenessService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopBridge.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopService.RunLoop.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosShellServicesFactory.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosSettingsLauncher.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosSingleInstanceGuard.cpp"
    "${MFX_MACOS_TRAY_MENU_FACTORY_SOURCE}"
    "${MFX_MACOS_TRAY_LOCALIZATION_SOURCE}"
    "${MFX_MACOS_TRAY_SOURCE}"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogEntryStore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanner.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosCursorPositionService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosForegroundProcessService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosVmForegroundSuppressionService.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Mouse.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.PointRepair.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Key.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTap.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.RunLoop.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.RunLoopHelpers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputEventUtils.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputPermissionState.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputPermissionState.Parse.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjector.EventPost.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjector.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyResolver.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyResolver.NonModifier.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Printable.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Function.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Special.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosKeyboardInjectorKeyTables.Modifier.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosVirtualKeyMapper.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosVirtualKeyMapper.KeyPairs.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosUserNotificationService.AppleScript.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosUserNotificationService.cpp"
    "${MFX_PROJECT_ROOT}/Settings/EmojiUtils.cpp"
)

set(MFX_MACOS_PLATFORM_HEADER_DIRS
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay"
    "${MFX_PROJECT_ROOT}/Platform/macos/Pet"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell"
    "${MFX_PROJECT_ROOT}/Platform/macos/System"
)
target_include_directories(mfx_shell_macos
    PRIVATE
        ${MFX_MACOS_PLATFORM_HEADER_DIRS}
)

if(MFX_MACOS_SWIFT_OBJECTS)
    target_sources(mfx_shell_macos PRIVATE ${MFX_MACOS_SWIFT_OBJECTS})
endif()

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
        "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTraySmokeMain.cpp"
    )
    target_link_libraries(mfx_shell_macos_tray_smoke
        PRIVATE
            mfx_shell_contracts
            mfx_shell_macos
    )
    target_compile_features(mfx_shell_macos_tray_smoke PRIVATE cxx_std_17)
    target_compile_definitions(mfx_shell_macos_tray_smoke PRIVATE MFX_TARGET_PLATFORM_MACOS=1)
    target_include_directories(mfx_shell_macos_tray_smoke
        PRIVATE
            ${MFX_MACOS_PLATFORM_HEADER_DIRS}
    )
endif()

message(STATUS "Configured shell package target: mfx_shell_macos")
