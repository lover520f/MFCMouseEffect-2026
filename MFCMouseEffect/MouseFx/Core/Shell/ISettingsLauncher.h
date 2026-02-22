#pragma once

#include <string>

namespace mousefx {

// Platform URL launcher abstraction.
class ISettingsLauncher {
public:
    virtual ~ISettingsLauncher() = default;

    virtual bool OpenUrlUtf8(const std::string& url) = 0;
};

} // namespace mousefx
