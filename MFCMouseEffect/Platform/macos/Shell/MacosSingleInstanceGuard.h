#pragma once

#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"

#include <string>

namespace mousefx {

class MacosSingleInstanceGuard final : public ISingleInstanceGuard {
public:
    MacosSingleInstanceGuard() = default;
    ~MacosSingleInstanceGuard() override;

    bool Acquire(const std::wstring& key) override;
    void Release() override;

private:
    static std::string NormalizeKey(const std::wstring& key);

private:
    int lockFd_ = -1;
};

} // namespace mousefx
