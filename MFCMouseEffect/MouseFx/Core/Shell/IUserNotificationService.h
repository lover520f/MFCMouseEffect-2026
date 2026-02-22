#pragma once

#include <string>

namespace mousefx {

// Platform UI notification abstraction.
class IUserNotificationService {
public:
    virtual ~IUserNotificationService() = default;

    virtual void ShowWarning(const std::string& titleUtf8, const std::string& messageUtf8) = 0;
};

} // namespace mousefx
