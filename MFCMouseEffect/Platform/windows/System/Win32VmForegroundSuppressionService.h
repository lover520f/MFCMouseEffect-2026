#pragma once

#include "MouseFx/Core/System/IForegroundSuppressionService.h"

#include <array>
#include <cstdint>
#include <string>

namespace mousefx {

class Win32VmForegroundSuppressionService final : public IForegroundSuppressionService {
public:
    bool ShouldSuppress(uint64_t nowTickMs) override;

private:
    static bool IsVmForegroundWindow();
    static bool TryGetProcessBaseName(void* hwnd, std::wstring* outBaseName);
    static bool ContainsVmToken(const std::wstring& input);
    static std::wstring ToLower(const std::wstring& input);

    static constexpr std::array<const wchar_t*, 11> kVmTokens = {
        L"vmware",
        L"virtualbox",
        L"virtual box",
        L"virtualboxvm",
        L"vmconnect",
        L"qemu",
        L"virt-viewer",
        L"remote-viewer",
        L"parallels",
        L"prl_vm",
        L"hyper-v"
    };
    static constexpr uint64_t kCheckIntervalMs = 800;

    uint64_t lastCheckTickMs_ = 0;
    bool lastResult_ = false;
};

} // namespace mousefx
