#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void mfx_macos_event_loop_ensure_application_ready_v1(void);
void mfx_macos_event_loop_run_application_v1(void);
void mfx_macos_event_loop_stop_application_v1(void);

#ifdef __cplusplus
}
#endif
