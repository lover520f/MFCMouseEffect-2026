#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "MouseFx/Interfaces/IOverlayLayer.h"

namespace mousefx {

class OverlayHostWindow final {
public:
    OverlayHostWindow();
    ~OverlayHostWindow();

    bool Create();
    void Shutdown();

    IOverlayLayer* AddLayer(std::unique_ptr<IOverlayLayer> layer);
    void RemoveLayer(IOverlayLayer* layer);
    void ClearLayers();

private:
    static constexpr UINT_PTR kTimerId = 5;
    static constexpr UINT kMsgEnsureTopmost = WM_APP + 0x33;

    static void CALLBACK ForegroundEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD eventTime);

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void Render();
    void UpdateLayered();
    void EnsureSurface(int w, int h);
    void DestroySurface();
    void StartFrameLoop();
    void StopFrameLoop();
    void EnsureTopmostZOrder(bool force = false);
    void RegisterForegroundHook();
    void UnregisterForegroundHook();

    HWND hwnd_ = nullptr;
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    bool ticking_ = false;
    uint64_t lastTopmostEnsureMs_ = 0;
    HWINEVENTHOOK foregroundHook_ = nullptr;
    std::vector<std::unique_ptr<IOverlayLayer>> layers_{};
};

} // namespace mousefx
