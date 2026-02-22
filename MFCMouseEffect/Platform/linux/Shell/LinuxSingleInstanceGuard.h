#pragma once

#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"

#include <string>

namespace mousefx {

class LinuxSingleInstanceGuard final : public ISingleInstanceGuard {
public:
    LinuxSingleInstanceGuard() = default;
    ~LinuxSingleInstanceGuard() override;

    bool Acquire(const std::wstring& key) override;
    void Release() override;

private:
    static std::string NormalizeKey(const std::wstring& key);

private:
    int lockFd_ = -1;
};

} // namespace mousefx
