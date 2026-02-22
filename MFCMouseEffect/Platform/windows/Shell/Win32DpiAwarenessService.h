#pragma once

#include "MouseFx/Core/Shell/IDpiAwarenessService.h"

namespace mousefx {

class Win32DpiAwarenessService final : public IDpiAwarenessService {
public:
    void EnableForScreenCoords() override;
};

} // namespace mousefx
