#pragma once

#include <cstdint>

extern "C" {

void* mfx_macos_mouse_companion_panel_create_v1(
    int32_t sizePx,
    int32_t offsetX,
    int32_t offsetY);
void mfx_macos_mouse_companion_panel_release_v1(void* handle);
void mfx_macos_mouse_companion_panel_show_v1(void* handle);
void mfx_macos_mouse_companion_panel_hide_v1(void* handle);
void mfx_macos_mouse_companion_panel_configure_v1(
    void* handle,
    int32_t sizePx,
    int32_t positionModeCode,
    int32_t edgeClampModeCode,
    int32_t offsetX,
    int32_t offsetY,
    int32_t absoluteX,
    int32_t absoluteY,
    const char* targetMonitorUtf8);
void mfx_macos_mouse_companion_panel_update_v1(
    void* handle,
    int32_t actionCode,
    float actionIntensity,
    float headTintAmount);
void mfx_macos_mouse_companion_panel_move_follow_v1(
    void* handle,
    int32_t x,
    int32_t y);
bool mfx_macos_mouse_companion_panel_load_model_v1(
    void* handle,
    const char* modelPathUtf8);
bool mfx_macos_mouse_companion_panel_load_action_library_v1(
    void* handle,
    const char* actionLibraryPathUtf8);
bool mfx_macos_mouse_companion_panel_configure_pose_binding_v1(
    void* handle,
    const char* const* boneNames,
    int32_t boneCount);
void mfx_macos_mouse_companion_panel_apply_pose_v1(
    void* handle,
    const int32_t* boneIndices,
    const float* positions,
    const float* rotations,
    const float* scales,
    int32_t poseCount);

} // extern "C"
