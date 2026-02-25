cmake_minimum_required(VERSION 3.20)

project(mfx_platform_shell_packages LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MFX_PACKAGE_PLATFORM "auto" CACHE STRING "auto|windows|macos|linux|all")
set_property(CACHE MFX_PACKAGE_PLATFORM PROPERTY STRINGS auto windows macos linux all)
option(MFX_ENABLE_CROSS_HOST_PACKAGES "Build non-host packages (requires explicit toolchain setup)." OFF)
option(MFX_ENABLE_ENTRY_RUNTIME_TARGETS "Build shared entry runtime/package targets." ON)
option(MFX_ENABLE_POSIX_CORE_RUNTIME "Enable macOS POSIX core runtime lane instead of scaffold lane." OFF)

get_filename_component(MFX_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

function(mfx_resolve_package_list out_packages)
    if(MFX_PACKAGE_PLATFORM STREQUAL "auto")
        if(WIN32)
            set(packages windows)
        elseif(APPLE)
            set(packages macos)
        elseif(UNIX)
            set(packages linux)
        else()
            set(packages)
        endif()
    elseif(MFX_PACKAGE_PLATFORM STREQUAL "all")
        set(packages windows macos linux)
    elseif(MFX_PACKAGE_PLATFORM STREQUAL "windows" OR
           MFX_PACKAGE_PLATFORM STREQUAL "macos" OR
           MFX_PACKAGE_PLATFORM STREQUAL "linux")
        set(packages "${MFX_PACKAGE_PLATFORM}")
    else()
        message(FATAL_ERROR "Unsupported MFX_PACKAGE_PLATFORM: ${MFX_PACKAGE_PLATFORM}")
    endif()

    set(${out_packages} "${packages}" PARENT_SCOPE)
endfunction()

add_library(mfx_shell_contracts INTERFACE)
target_include_directories(mfx_shell_contracts
    INTERFACE
        "${MFX_PROJECT_ROOT}"
)
target_compile_features(mfx_shell_contracts INTERFACE cxx_std_17)

function(mfx_host_supports_package package_name out_var)
    set(supported FALSE)
    if(package_name STREQUAL "windows" AND WIN32)
        set(supported TRUE)
    elseif(package_name STREQUAL "macos" AND APPLE)
        set(supported TRUE)
    elseif(package_name STREQUAL "linux" AND UNIX AND NOT APPLE)
        set(supported TRUE)
    endif()
    set(${out_var} ${supported} PARENT_SCOPE)
endfunction()

function(mfx_add_platform_package package_name)
    mfx_host_supports_package("${package_name}" package_supported)
    if(NOT package_supported AND NOT MFX_ENABLE_CROSS_HOST_PACKAGES)
        if(MFX_PACKAGE_PLATFORM STREQUAL "all")
            message(STATUS "Skipping non-host shell package: ${package_name}")
            return()
        endif()
        message(FATAL_ERROR
            "Package '${package_name}' is not supported on this host. "
            "Set MFX_ENABLE_CROSS_HOST_PACKAGES=ON with a valid toolchain if you need cross build.")
    endif()

    set(package_cmake "${MFX_PROJECT_ROOT}/Platform/${package_name}/CMakeLists.txt")
    if(NOT EXISTS "${package_cmake}")
        message(FATAL_ERROR "Missing package CMake file: ${package_cmake}")
    endif()

    set(package_binary_dir "${CMAKE_BINARY_DIR}/platform_${package_name}")
    add_subdirectory("${MFX_PROJECT_ROOT}/Platform/${package_name}" "${package_binary_dir}")
endfunction()

mfx_resolve_package_list(MFX_PACKAGE_LIST)
if(NOT MFX_PACKAGE_LIST)
    message(FATAL_ERROR "No platform package resolved for host/system.")
endif()

set(MFX_NEEDS_POSIX OFF)
foreach(package_name IN LISTS MFX_PACKAGE_LIST)
    if(package_name STREQUAL "macos" OR package_name STREQUAL "linux")
        set(MFX_NEEDS_POSIX ON)
    endif()
endforeach()

if(MFX_NEEDS_POSIX)
    add_subdirectory("${MFX_PROJECT_ROOT}/Platform/posix" "${CMAKE_BINARY_DIR}/platform_posix")
endif()

foreach(package_name IN LISTS MFX_PACKAGE_LIST)
    mfx_add_platform_package("${package_name}")
endforeach()

if(MFX_ENABLE_ENTRY_RUNTIME_TARGETS)
    set(MFX_ENTRY_PLATFORM "")
    set(MFX_ENTRY_PLATFORM_TARGET "")

    if(TARGET mfx_shell_windows)
        set(MFX_ENTRY_PLATFORM "windows")
        set(MFX_ENTRY_PLATFORM_TARGET "mfx_shell_windows")
    endif()

    if(TARGET mfx_shell_macos)
        if(MFX_ENTRY_PLATFORM)
            message(FATAL_ERROR
                "Entry runtime target is ambiguous with multiple shell packages. "
                "Set MFX_PACKAGE_PLATFORM to a single platform or disable MFX_ENABLE_ENTRY_RUNTIME_TARGETS.")
        endif()
        set(MFX_ENTRY_PLATFORM "macos")
        set(MFX_ENTRY_PLATFORM_TARGET "mfx_shell_macos")
    endif()

    if(TARGET mfx_shell_linux)
        if(MFX_ENTRY_PLATFORM)
            message(FATAL_ERROR
                "Entry runtime target is ambiguous with multiple shell packages. "
                "Set MFX_PACKAGE_PLATFORM to a single platform or disable MFX_ENABLE_ENTRY_RUNTIME_TARGETS.")
        endif()
        set(MFX_ENTRY_PLATFORM "linux")
        set(MFX_ENTRY_PLATFORM_TARGET "mfx_shell_linux")
    endif()

    if(NOT MFX_ENTRY_PLATFORM)
        message(FATAL_ERROR "Entry runtime target requested but no shell package target is available.")
    endif()

    add_library(mfx_entry_runtime_common STATIC
        "${MFX_PROJECT_ROOT}/Platform/PlatformEntryRunner.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformStartupOptionsFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformAppShellFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformShellServicesFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformControlMessageCodecFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformControlServicesFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformInputServicesFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformOverlayCoordSpaceFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformOverlayServicesFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/PlatformSystemServicesFactory.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixCoreAppShell.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixCoreWebSettingsProbe.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixScaffoldAppShell.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/ScaffoldSettingsApi.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/ScaffoldSettingsRequestHandler.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/ScaffoldSettingsRouteConfig.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/ScaffoldSettingsRuntime.cpp"
        "${MFX_PROJECT_ROOT}/Platform/posix/Shell/ScaffoldSettingsWebUiAssets.cpp"
        "${MFX_PROJECT_ROOT}/MouseFx/Server/HttpServer.cpp"
        "${MFX_PROJECT_ROOT}/MouseFx/Server/HttpServer.Protocol.cpp"
    )
    target_link_libraries(mfx_entry_runtime_common PUBLIC mfx_shell_contracts)
    target_compile_features(mfx_entry_runtime_common PUBLIC cxx_std_17)
    target_link_libraries(mfx_entry_runtime_common PUBLIC "${MFX_ENTRY_PLATFORM_TARGET}")

    if(MFX_ENTRY_PLATFORM STREQUAL "windows")
        target_compile_definitions(mfx_entry_runtime_common PRIVATE MFX_TARGET_PLATFORM_WINDOWS=1)
    elseif(MFX_ENTRY_PLATFORM STREQUAL "macos")
        target_compile_definitions(mfx_entry_runtime_common PRIVATE MFX_TARGET_PLATFORM_MACOS=1)
        if(MFX_ENABLE_POSIX_CORE_RUNTIME)
            target_compile_definitions(mfx_entry_runtime_common PRIVATE MFX_ENTRY_RUNTIME_POSIX_CORE=1)
        else()
            target_compile_definitions(mfx_entry_runtime_common PRIVATE MFX_ENTRY_RUNTIME_SCAFFOLD=1)
        endif()
    elseif(MFX_ENTRY_PLATFORM STREQUAL "linux")
        target_compile_definitions(mfx_entry_runtime_common PRIVATE MFX_TARGET_PLATFORM_LINUX=1)
        target_compile_definitions(mfx_entry_runtime_common PRIVATE MFX_ENTRY_RUNTIME_SCAFFOLD=1)
    endif()

    if(MFX_ENTRY_PLATFORM STREQUAL "macos" AND MFX_ENABLE_POSIX_CORE_RUNTIME)
        set(MFX_WASM3_SOURCE_DIR "${MFX_PROJECT_ROOT}/WasmRuntimeBridge/third_party/wasm3/source")
        set(MFX_WASM3_CORE_SOURCES
            "${MFX_WASM3_SOURCE_DIR}/m3_bind.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_code.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_compile.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_core.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_env.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_exec.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_function.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_info.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_module.c"
            "${MFX_WASM3_SOURCE_DIR}/m3_parse.c"
        )
        add_library(mfx_wasm3_core_runtime STATIC ${MFX_WASM3_CORE_SOURCES})
        target_include_directories(mfx_wasm3_core_runtime
            PUBLIC
                "${MFX_WASM3_SOURCE_DIR}"
        )
        target_compile_features(mfx_wasm3_core_runtime PRIVATE c_std_99)
        if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
            target_compile_options(mfx_wasm3_core_runtime PRIVATE /w)
        else()
            target_compile_options(mfx_wasm3_core_runtime PRIVATE -w)
            set_source_files_properties(
                "${MFX_WASM3_SOURCE_DIR}/m3_env.c"
                PROPERTIES COMPILE_OPTIONS "-Wno-sign-compare")
        endif()

        target_sources(mfx_entry_runtime_common PRIVATE
            "${MFX_PROJECT_ROOT}/Platform/PlatformRuntimeEnvironment.cpp"
            "${MFX_PROJECT_ROOT}/Platform/PlatformTextEncoding.cpp"
            "${MFX_PROJECT_ROOT}/Platform/PlatformApplicationCatalog.cpp"
            "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosApplicationCatalogScanner.mm"
            "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosAppleScriptFolderPicker.mm"
            "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosNativeFolderPicker.mm"
            "${MFX_PROJECT_ROOT}/Platform/macos/System/MacosOpenPanelFolderPicker.mm"
            "${MFX_PROJECT_ROOT}/Platform/PlatformBinaryResourceLoader.cpp"
            "${MFX_PROJECT_ROOT}/Platform/PlatformDisplayTopology.cpp"
            "${MFX_PROJECT_ROOT}/Platform/PlatformNativeFolderPicker.cpp"
            "${MFX_PROJECT_ROOT}/Platform/PlatformWasmRuntimeFactory.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Automation/InputAutomationEngine.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Automation/KeyChord.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Automation/ShortcutCaptureSession.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Input/GestureRecognizer.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/ConfigPathResolver.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfig.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfig.Internal.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfig.Load.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfig.Save.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Automation.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Common.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Effects.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Input.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Trail.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Wasm.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Automation.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Effects.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Input.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Trail.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Wasm.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Config/EffectConfigJsonCodecEffectsColorHelpers.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/AppController.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/AppController.ConfigUpdates.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/AppController.Wasm.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/AutomationDispatchFeature.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/CommandHandler.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/CommandHandler.ApplySettings.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/DispatchRouter.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/EffectFactory.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/InputIndicatorDispatchFeature.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Control/WasmDispatchFeature.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Overlay/OverlayCoordSpace.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Overlay/OverlayHostService.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Protocol/JsonLite.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/System/GdiPlusSession.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/NullWasmRuntime.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmBinaryCodec.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmCommandRenderer.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmCommandBufferParser.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmEffectHost.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmEventInvokeExecutor.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmExecutionBudgetGuard.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmPluginManifest.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmPluginCatalog.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmPluginImageAssetCatalog.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmPluginPaths.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmPluginTransferService.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/Wasm3Runtime.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Core/Wasm/WasmRuntimeFactory.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Effects/HoldRouteCatalog.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterBackendRegistry.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/SettingsSchemaBuilder.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/SettingsStateMapper.BaseSections.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/SettingsStateMapper.Diagnostics.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/SettingsStateMapper.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.AutomationRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.CoreApiRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.RequestGateway.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.TestAutomationApiRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.WebUiPathResolver.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.Routing.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.TestApiRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.TestRouteCommon.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.TestWasmInputApiRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.WasmCatalogRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.WasmRuntimeRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.WasmRouteUtils.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.WasmRoutes.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebSettingsServer.TokenMonitor.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Server/WebUiAssets.cpp"
            "${MFX_PROJECT_ROOT}/MouseFx/Utils/StringUtils.cpp"
        )
        target_link_libraries(mfx_entry_runtime_common PRIVATE mfx_wasm3_core_runtime)
    endif()

    if(MFX_ENTRY_PLATFORM STREQUAL "windows")
        add_library(mfx_entry_windows STATIC
            "${MFX_PROJECT_ROOT}/MFCMouseEffect.cpp"
        )
        target_link_libraries(mfx_entry_windows
            PUBLIC
                mfx_shell_contracts
                mfx_entry_runtime_common
        )
        target_compile_features(mfx_entry_windows PUBLIC cxx_std_17)
        target_compile_definitions(mfx_entry_windows PRIVATE MFX_TARGET_PLATFORM_WINDOWS=1)
        message(STATUS "Configured entry package target: mfx_entry_windows")
    endif()

    if(MFX_ENTRY_PLATFORM STREQUAL "macos" OR MFX_ENTRY_PLATFORM STREQUAL "linux")
        add_library(mfx_entry_posix STATIC
            "${MFX_PROJECT_ROOT}/Platform/posix/Entry/PosixMain.cpp"
        )
        target_link_libraries(mfx_entry_posix
            PUBLIC
                mfx_shell_contracts
                mfx_entry_runtime_common
        )
        target_compile_features(mfx_entry_posix PUBLIC cxx_std_17)
        if(MFX_ENTRY_PLATFORM STREQUAL "macos")
            target_compile_definitions(mfx_entry_posix PRIVATE MFX_TARGET_PLATFORM_MACOS=1)
        else()
            target_compile_definitions(mfx_entry_posix PRIVATE MFX_TARGET_PLATFORM_LINUX=1)
        endif()
        message(STATUS "Configured entry package target: mfx_entry_posix")

        if((MFX_ENTRY_PLATFORM STREQUAL "macos" AND APPLE) OR
           (MFX_ENTRY_PLATFORM STREQUAL "linux" AND UNIX AND NOT APPLE))
            add_executable(mfx_entry_posix_host
                "${MFX_PROJECT_ROOT}/Platform/posix/Entry/PosixMain.cpp"
            )
            target_link_libraries(mfx_entry_posix_host
                PRIVATE
                    mfx_shell_contracts
                    mfx_entry_runtime_common
            )
            target_compile_features(mfx_entry_posix_host PRIVATE cxx_std_17)
            if(MFX_ENTRY_PLATFORM STREQUAL "macos")
                target_compile_definitions(mfx_entry_posix_host PRIVATE MFX_TARGET_PLATFORM_MACOS=1)
            else()
                target_compile_definitions(mfx_entry_posix_host PRIVATE MFX_TARGET_PLATFORM_LINUX=1)
            endif()
            message(STATUS "Configured host entry executable target: mfx_entry_posix_host")
        endif()
    endif()
endif()
