#pragma once

#include "MouseFx/Core/Shell/ISettingsLauncher.h"

namespace mousefx {

class LinuxSettingsLauncher final : public ISettingsLauncher {
public:
    bool OpenUrlUtf8(const std::string& url) override;
};

} // namespace mousefx
