#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>
#include "MouseFx/Interfaces/IHoldRuntime.h"

namespace mousefx {

/// Registry (Abstract Factory) for hold-runtime implementations.
/// Mirrors the pattern used by RendererRegistry for IRippleRenderer.
class HoldRuntimeRegistry {
public:
    using Factory = std::function<std::unique_ptr<IHoldRuntime>()>;

    static HoldRuntimeRegistry& Instance() {
        static HoldRuntimeRegistry instance;
        return instance;
    }

    void Register(const std::string& name, Factory factory) {
        factories_[name] = factory;
    }

    std::unique_ptr<IHoldRuntime> Create(const std::string& name) {
        auto it = factories_.find(name);
        if (it != factories_.end()) {
            return it->second();
        }
        return nullptr;
    }

private:
    std::map<std::string, Factory> factories_;
};

// Helper class for static registration of IHoldRuntime implementations.
template <typename T>
class HoldRuntimeRegistrar {
public:
    HoldRuntimeRegistrar(const std::string& name) {
        HoldRuntimeRegistry::Instance().Register(name, []() {
            return std::make_unique<T>();
        });
    }
};

#define REGISTER_HOLD_RUNTIME(Name, Class) \
    static mousefx::HoldRuntimeRegistrar<Class> holdReg_##Class(Name);

} // namespace mousefx
