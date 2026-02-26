if(TARGET mfx_shell_posix_common)
    return()
endif()

add_library(mfx_shell_posix_common STATIC
    "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixBlockingEventLoop.cpp"
    "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixKeyValueCaptureFile.cpp"
    "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixSettingsLauncher.Capture.cpp"
    "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixSettingsLauncher.cpp"
    "${MFX_PROJECT_ROOT}/Platform/posix/Shell/PosixSettingsLauncher.Spawn.cpp"
)

target_link_libraries(mfx_shell_posix_common PUBLIC mfx_shell_contracts)
target_compile_features(mfx_shell_posix_common PUBLIC cxx_std_17)

find_package(Threads REQUIRED)
target_link_libraries(mfx_shell_posix_common PUBLIC Threads::Threads)
