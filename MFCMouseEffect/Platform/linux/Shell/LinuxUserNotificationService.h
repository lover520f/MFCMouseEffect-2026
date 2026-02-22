#pragma once

#include "MouseFx/Core/Shell/IUserNotificationService.h"

namespace mousefx {

class LinuxUserNotificationService final : public IUserNotificationService {
public:
    void ShowWarning(const std::string& titleUtf8, const std::string& messageUtf8) override;
};

} // namespace mousefx
