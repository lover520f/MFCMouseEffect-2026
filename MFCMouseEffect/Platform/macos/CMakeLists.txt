if(TARGET mfx_shell_macos)
    return()
endif()

if(NOT TARGET mfx_shell_posix_common)
    message(FATAL_ERROR "mfx_shell_posix_common is required before configuring mfx_shell_macos.")
endif()

set(MFX_MACOS_TRAY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayService.cpp")
set(MFX_MACOS_TRAY_LOCALIZATION_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuLocalization.cpp")
set(MFX_MACOS_TRAY_MENU_FACTORY_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuFactory.cpp")
set(MFX_MACOS_NOTIFICATION_SWIFT_OBJECT "")
set(MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT "")
set(MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT "")
set(MFX_MACOS_APP_CATALOG_SWIFT_OBJECT "")
set(MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT "")
set(MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT "")
set(MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT "")
set(MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT "")
set(MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT "")
set(MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT "")
set(MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT "")
set(MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT "")
set(MFX_MACOS_TRAY_MENU_SWIFT_OBJECT "")
set(MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT "")
set(MFX_SWIFT_LANGUAGE_MODE "auto" CACHE STRING "Swift language mode for macOS bridges: auto|5|6")
set_property(CACHE MFX_SWIFT_LANGUAGE_MODE PROPERTY STRINGS auto 5 6)
if(APPLE)
    set(MFX_MACOS_TRAY_LOCALIZATION_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuLocalization.cpp")

    find_program(MFX_SWIFTC_EXECUTABLE swiftc REQUIRED)
    set(MFX_SWIFTC_COMMON_ARGS -parse-as-library -c)
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

    set(MFX_MACOS_NOTIFICATION_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosUserNotificationBridge.swift")
    set(MFX_MACOS_NOTIFICATION_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosUserNotificationBridge.swift.o")
    set(MFX_MACOS_FOLDER_PICKER_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosNativeFolderPickerBridge.swift")
    set(MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosNativeFolderPickerBridge.swift.o")
    set(MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosSettingsLauncherBridge.swift")
    set(MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosSettingsLauncherBridge.swift.o")
    set(MFX_MACOS_APP_CATALOG_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogBridge.swift")
    set(MFX_MACOS_APP_CATALOG_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosApplicationCatalogBridge.swift.o")
    set(MFX_MACOS_FOREGROUND_PROCESS_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosForegroundProcessBridge.swift")
    set(MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosForegroundProcessBridge.swift.o")
    set(MFX_MACOS_INPUT_PERMISSION_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosInputPermissionBridge.swift")
    set(MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosInputPermissionBridge.swift.o")
    set(MFX_MACOS_INPUT_INDICATOR_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosInputIndicatorBridge.swift")
    set(MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosInputIndicatorBridge.swift.o")
    set(MFX_MACOS_OVERLAY_COORD_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Overlay/MacosOverlayCoordSpaceBridge.swift")
    set(MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosOverlayCoordSpaceBridge.swift.o")
    set(MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosOverlayRenderSupportBridge.swift")
    set(MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosOverlayRenderSupportBridge.swift.o")
    set(MFX_MACOS_LINE_TRAIL_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosLineTrailOverlayBridge.swift")
    set(MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosLineTrailOverlayBridge.swift.o")
    set(MFX_MACOS_TEXT_FALLBACK_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTextEffectFallbackBridge.swift")
    set(MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosTextEffectFallbackBridge.swift.o")
    set(MFX_MACOS_TRAY_LOCALIZATION_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuLocalizationBridge.swift")
    set(MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosTrayMenuLocalizationBridge.swift.o")
    set(MFX_MACOS_TRAY_MENU_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosTrayMenuBridge.swift")
    set(MFX_MACOS_TRAY_MENU_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosTrayMenuBridge.swift.o")
    set(MFX_MACOS_EVENT_LOOP_SWIFT_SOURCE "${MFX_PROJECT_ROOT}/Platform/macos/Shell/MacosEventLoopBridge.swift")
    set(MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/MacosEventLoopBridge.swift.o")
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
    add_custom_command(
        OUTPUT "${MFX_MACOS_NOTIFICATION_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_NOTIFICATION_SWIFT_SOURCE}" -o "${MFX_MACOS_NOTIFICATION_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_NOTIFICATION_SWIFT_SOURCE}"
        COMMENT "Compile Swift notification bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_NOTIFICATION_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_FOLDER_PICKER_SWIFT_SOURCE}" -o "${MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_FOLDER_PICKER_SWIFT_SOURCE}"
        COMMENT "Compile Swift folder picker bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_SOURCE}" -o "${MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_SOURCE}"
        COMMENT "Compile Swift settings launcher bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_APP_CATALOG_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_APP_CATALOG_SWIFT_SOURCE}" -o "${MFX_MACOS_APP_CATALOG_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_APP_CATALOG_SWIFT_SOURCE}"
        COMMENT "Compile Swift application catalog bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_APP_CATALOG_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_FOREGROUND_PROCESS_SWIFT_SOURCE}" -o "${MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_FOREGROUND_PROCESS_SWIFT_SOURCE}"
        COMMENT "Compile Swift foreground process bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_INPUT_PERMISSION_SWIFT_SOURCE}" -o "${MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_INPUT_PERMISSION_SWIFT_SOURCE}"
        COMMENT "Compile Swift input permission bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_INPUT_INDICATOR_SWIFT_SOURCE}" -o "${MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_INPUT_INDICATOR_SWIFT_SOURCE}"
        COMMENT "Compile Swift input indicator bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_OVERLAY_COORD_SWIFT_SOURCE}" -o "${MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_OVERLAY_COORD_SWIFT_SOURCE}"
        COMMENT "Compile Swift overlay coord-space bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_SOURCE}" -o "${MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_SOURCE}"
        COMMENT "Compile Swift overlay render-support bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_LINE_TRAIL_SWIFT_SOURCE}" -o "${MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_LINE_TRAIL_SWIFT_SOURCE}"
        COMMENT "Compile Swift line-trail overlay bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_TEXT_FALLBACK_SWIFT_SOURCE}" -o "${MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_TEXT_FALLBACK_SWIFT_SOURCE}"
        COMMENT "Compile Swift text-effect fallback bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_TRAY_LOCALIZATION_SWIFT_SOURCE}" -o "${MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_TRAY_LOCALIZATION_SWIFT_SOURCE}"
        COMMENT "Compile Swift tray localization bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_TRAY_MENU_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_TRAY_MENU_SWIFT_SOURCE}" -o "${MFX_MACOS_TRAY_MENU_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_TRAY_MENU_SWIFT_SOURCE}"
        COMMENT "Compile Swift tray menu bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_TRAY_MENU_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
    add_custom_command(
        OUTPUT "${MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT}"
        COMMAND "${MFX_SWIFTC_EXECUTABLE}" ${MFX_SWIFTC_COMMON_ARGS} "${MFX_MACOS_EVENT_LOOP_SWIFT_SOURCE}" -o "${MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT}"
        DEPENDS "${MFX_MACOS_EVENT_LOOP_SWIFT_SOURCE}"
        COMMENT "Compile Swift event loop bridge"
        VERBATIM
    )
    set_source_files_properties(
        "${MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT}"
        PROPERTIES
            GENERATED TRUE
            EXTERNAL_OBJECT TRUE
    )
endif()

add_library(mfx_shell_macos STATIC
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/ClickEffectCompute.cpp"
    "${MFX_PROJECT_ROOT}/MouseFx/Core/Effects/TrailEffectCompute.cpp"
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
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseWindowRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseWindowRegistry.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRenderer.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseEffect.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Accent.cpp"
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
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmCommandRenderResolvers.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRenderer.cpp"
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
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.Layout.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.Style.cpp"
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
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.cpp"
    "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Mouse.cpp"
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

if(APPLE)
    # Explicit ObjC++ allowlist. Keep this list tight and prune entries when
    # modules are fully migrated to Swift bridges.
    set(MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosClickPulseOverlayStyle.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosOverlayRenderSupport.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayStyle.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayStyle.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayStyle.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Accent.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Update.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayRenderMath.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmOverlayRuntime.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.Style.cpp"
        "${MFX_PROJECT_ROOT}/Platform/macos/Wasm/MacosWasmTextOverlay.cpp"
    )

    get_target_property(MFX_MACOS_TARGET_SOURCES mfx_shell_macos SOURCES)
    set(MFX_MACOS_OBJCXX_CPP_SOURCES "")
    foreach(MFX_MACOS_SOURCE IN LISTS MFX_MACOS_TARGET_SOURCES)
        if(MFX_MACOS_SOURCE IN_LIST MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST)
            list(APPEND MFX_MACOS_OBJCXX_CPP_SOURCES "${MFX_MACOS_SOURCE}")
        endif()
    endforeach()
    if(MFX_MACOS_OBJCXX_CPP_SOURCES)
        set_source_files_properties(
            ${MFX_MACOS_OBJCXX_CPP_SOURCES}
            PROPERTIES
                COMPILE_FLAGS "-x objective-c++"
        )
    endif()
endif()

set(MFX_MACOS_PLATFORM_HEADER_DIRS
    "${MFX_PROJECT_ROOT}/Platform/macos/Effects"
    "${MFX_PROJECT_ROOT}/Platform/macos/Overlay"
    "${MFX_PROJECT_ROOT}/Platform/macos/Wasm"
    "${MFX_PROJECT_ROOT}/Platform/macos/Shell"
    "${MFX_PROJECT_ROOT}/Platform/macos/System"
)
target_include_directories(mfx_shell_macos
    PRIVATE
        ${MFX_MACOS_PLATFORM_HEADER_DIRS}
)

if(MFX_MACOS_NOTIFICATION_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_NOTIFICATION_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_FOLDER_PICKER_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_SETTINGS_LAUNCHER_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_APP_CATALOG_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_APP_CATALOG_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_FOREGROUND_PROCESS_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_INPUT_PERMISSION_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_INPUT_INDICATOR_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_OVERLAY_COORD_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_OVERLAY_RENDER_SUPPORT_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_LINE_TRAIL_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_TEXT_FALLBACK_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_TRAY_LOCALIZATION_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_TRAY_MENU_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_TRAY_MENU_SWIFT_OBJECT}")
endif()
if(MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT)
    target_sources(mfx_shell_macos PRIVATE "${MFX_MACOS_EVENT_LOOP_SWIFT_OBJECT}")
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
