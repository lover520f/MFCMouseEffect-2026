#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>
#include "../Interfaces/IRippleRenderer.h"

namespace mousefx {

class RendererRegistry {
public:
    using Factory = std::function<std::unique_ptr<IRippleRenderer>()>;

    static RendererRegistry& Instance() {
        static RendererRegistry instance;
        return instance;
    }

    void Register(const std::string& name, Factory factory) {
        factories_[name] = factory;
    }

    std::unique_ptr<IRippleRenderer> Create(const std::string& name) {
        auto it = factories_.find(name);
        if (it != factories_.end()) {
            return it->second();
        }
        return nullptr;
    }

private:
    std::map<std::string, Factory> factories_;
};

// Helper class for static registration
template <typename T>
class RendererRegistrar {
public:
    RendererRegistrar(const std::string& name) {
        RendererRegistry::Instance().Register(name, []() {
            return std::make_unique<T>();
        });
    }
};

#define REGISTER_RENDERER(Name, Class) \
    static mousefx::RendererRegistrar<Class> reg_##Class(Name);

} // namespace mousefx
