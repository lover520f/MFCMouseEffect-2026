#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mfx_macos_application_catalog_emit_fn)(
    const char* processNameUtf8,
    const char* displayNameUtf8,
    const char* sourceUtf8,
    void* context);

int32_t mfx_macos_scan_application_catalog_v1(
    mfx_macos_application_catalog_emit_fn emitFn,
    void* context,
    char* outErrorUtf8,
    int32_t outErrorCapacity);

#ifdef __cplusplus
}
#endif
