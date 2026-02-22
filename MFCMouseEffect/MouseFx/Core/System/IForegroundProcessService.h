#pragma once

#include <string>

namespace mousefx {

// Cross-platform foreground process name resolver.
class IForegroundProcessService {
public:
    virtual ~IForegroundProcessService() = default;

    virtual std::string CurrentProcessBaseName() = 0;
};

} // namespace mousefx
