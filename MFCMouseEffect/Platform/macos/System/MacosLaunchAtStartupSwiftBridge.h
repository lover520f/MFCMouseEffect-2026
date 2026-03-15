#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t mfx_macos_set_launch_at_startup_v1(
    int32_t enabled,
    char* outErrorUtf8,
    int32_t outErrorCapacity);

#ifdef __cplusplus
}
#endif

