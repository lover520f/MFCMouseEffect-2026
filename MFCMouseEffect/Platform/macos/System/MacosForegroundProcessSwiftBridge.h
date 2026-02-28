#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t mfx_macos_resolve_foreground_process_name_v1(
    char* outProcessNameUtf8,
    int32_t outProcessNameCapacity);

#ifdef __cplusplus
}
#endif
