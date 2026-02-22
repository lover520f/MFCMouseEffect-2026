#pragma once

#include <string>

namespace mousefx {

// Platform single-instance lock abstraction.
class ISingleInstanceGuard {
public:
    virtual ~ISingleInstanceGuard() = default;

    virtual bool Acquire(const std::wstring& key) = 0;
    virtual void Release() = 0;
};

} // namespace mousefx
